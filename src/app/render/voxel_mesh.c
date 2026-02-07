#include "app/render/voxel_mesh.h"
#include "app/render/voxel_renderer.h"
#include "core/world/voxel_world.h"
#include "core/world/chunk.h"
#include "core/math/core_math.h"
#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_VERTEX_CAPACITY 1024
#define VERTEX_GROWTH_FACTOR 2

// Verifica se um bloco existe e é sólido (não usado - culling desativado)
__attribute__((unused)) static bool IsBlockSolid(VoxelWorld* world, int32_t x, int32_t y, int32_t z) {
    Voxel v = VoxelWorld_GetBlock(world, x, y, z);
    return v.type != BLOCK_AIR;
}

// Adiciona vértice ao mesh
static void AddVertex(VoxelMesh* mesh, Vector3 pos, Vector3 normal, Color color) {
    if (!mesh || !mesh->initialized) return;
    
    // Expande array se necessário
    if (mesh->vertexCount >= mesh->vertexCapacity) {
        int32_t newCapacity = mesh->vertexCapacity * VERTEX_GROWTH_FACTOR;
        if (newCapacity < INITIAL_VERTEX_CAPACITY) newCapacity = INITIAL_VERTEX_CAPACITY;
        
        VoxelVertex* newVertices = (VoxelVertex*)realloc(mesh->vertices, 
                                                         newCapacity * sizeof(VoxelVertex));
        if (!newVertices) return; // Falha silenciosa
        
        mesh->vertices = newVertices;
        mesh->vertexCapacity = newCapacity;
    }
    
    mesh->vertices[mesh->vertexCount].position = pos;
    mesh->vertices[mesh->vertexCount].normal = normal;
    mesh->vertices[mesh->vertexCount].color = color;
    mesh->vertexCount++;
}

// Adiciona uma face (2 triângulos) ao mesh
static void AddFace(VoxelMesh* mesh, Vector3 blockPos, FaceDirection dir, Color color) {
    float x = blockPos.x;
    float y = blockPos.y;
    float z = blockPos.z;
    
    Vector3 normal;
    Vector3 v[4]; // 4 vértices da face
    
    // Define vértices e normal baseado na direção da face
    switch (dir) {
        case FACE_NEGATIVE_X: // Face esquerda (-X)
            v[0] = (Vector3){x, y, z};
            v[1] = (Vector3){x, y, z + 1.0f};
            v[2] = (Vector3){x, y + 1.0f, z + 1.0f};
            v[3] = (Vector3){x, y + 1.0f, z};
            normal = (Vector3){-1.0f, 0.0f, 0.0f};
            break;
            
        case FACE_POSITIVE_X: // Face direita (+X)
            v[0] = (Vector3){x + 1.0f, y, z};
            v[1] = (Vector3){x + 1.0f, y + 1.0f, z};
            v[2] = (Vector3){x + 1.0f, y + 1.0f, z + 1.0f};
            v[3] = (Vector3){x + 1.0f, y, z + 1.0f};
            normal = (Vector3){1.0f, 0.0f, 0.0f};
            break;
            
        case FACE_NEGATIVE_Y: // Face inferior (-Y) - CHÃO
            v[0] = (Vector3){x, y, z};
            v[1] = (Vector3){x + 1.0f, y, z};
            v[2] = (Vector3){x + 1.0f, y, z + 1.0f};
            v[3] = (Vector3){x, y, z + 1.0f};
            normal = (Vector3){0.0f, -1.0f, 0.0f};
            break;
            
        case FACE_POSITIVE_Y: // Face superior (+Y) - TETO
            v[0] = (Vector3){x, y + 1.0f, z};
            v[1] = (Vector3){x, y + 1.0f, z + 1.0f};
            v[2] = (Vector3){x + 1.0f, y + 1.0f, z + 1.0f};
            v[3] = (Vector3){x + 1.0f, y + 1.0f, z};
            normal = (Vector3){0.0f, 1.0f, 0.0f};
            break;
            
        case FACE_NEGATIVE_Z: // Face traseira (-Z)
            v[0] = (Vector3){x, y, z};
            v[1] = (Vector3){x, y + 1.0f, z};
            v[2] = (Vector3){x + 1.0f, y + 1.0f, z};
            v[3] = (Vector3){x + 1.0f, y, z};
            normal = (Vector3){0.0f, 0.0f, -1.0f};
            break;
            
        case FACE_POSITIVE_Z: // Face frontal (+Z)
            v[0] = (Vector3){x, y, z + 1.0f};
            v[1] = (Vector3){x + 1.0f, y, z + 1.0f};
            v[2] = (Vector3){x + 1.0f, y + 1.0f, z + 1.0f};
            v[3] = (Vector3){x, y + 1.0f, z + 1.0f};
            normal = (Vector3){0.0f, 0.0f, 1.0f};
            break;
    }
    
    // Divide face em 2 triângulos: (0,1,2) e (0,2,3)
    AddVertex(mesh, v[0], normal, color);
    AddVertex(mesh, v[1], normal, color);
    AddVertex(mesh, v[2], normal, color);
    
    AddVertex(mesh, v[0], normal, color);
    AddVertex(mesh, v[2], normal, color);
    AddVertex(mesh, v[3], normal, color);
}

// Verifica se uma face deve ser renderizada (back-face culling) (não usado - culling desativado)
__attribute__((unused)) static bool ShouldRenderFace(Vector3 faceNormal, Vector3 cameraPos, Vector3 blockCenter) {
    // Vetor do centro do bloco para a câmera (direção da câmera em relação ao bloco)
    Vector3 toCamera = (Vector3){
        cameraPos.x - blockCenter.x,
        cameraPos.y - blockCenter.y,
        cameraPos.z - blockCenter.z
    };
    
    // Normaliza
    float len = sqrtf(toCamera.x * toCamera.x + toCamera.y * toCamera.y + toCamera.z * toCamera.z);
    if (len < 0.0001f) return true; // Muito próximo, renderiza
    
    toCamera.x /= len;
    toCamera.y /= len;
    toCamera.z /= len;
    
    // Produto escalar: se positivo, normal da face aponta na direção da câmera (face visível)
    float dot = faceNormal.x * toCamera.x + faceNormal.y * toCamera.y + faceNormal.z * toCamera.z;
    return dot > 0.0f;
}

void VoxelMesh_Init(VoxelMesh* mesh) {
    if (!mesh) return;
    memset(mesh, 0, sizeof(VoxelMesh));
    mesh->vertexCapacity = INITIAL_VERTEX_CAPACITY;
    mesh->vertices = (VoxelVertex*)malloc(mesh->vertexCapacity * sizeof(VoxelVertex));
    mesh->initialized = (mesh->vertices != NULL);
}

void VoxelMesh_Clear(VoxelMesh* mesh) {
    if (!mesh) return;
    mesh->vertexCount = 0;
}

void VoxelMesh_Destroy(VoxelMesh* mesh) {
    if (!mesh) return;
    if (mesh->vertices) {
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    mesh->vertexCount = 0;
    mesh->vertexCapacity = 0;
    mesh->initialized = false;
}

void VoxelMesh_GenerateChunk(VoxelMesh* mesh, VoxelWorld* world, 
                             int32_t chunkX, int32_t chunkZ,
                             float playerX, float playerY, float playerZ) {
    (void)playerX; (void)playerY; (void)playerZ; // Parâmetros não usados (culling desativado)
    if (!mesh || !mesh->initialized || !world) return;
    
    VoxelMesh_Clear(mesh);
    
    Chunk* chunk = VoxelWorld_GetChunk(world, chunkX, chunkZ);
    if (!chunk || chunk->state != CHUNK_STATE_READY) return;
    
    // DISTANCE CULLING DESATIVADO
    // const float MAX_RENDER_DISTANCE = 30.0f;
    // const float MAX_DISTANCE_SQ = MAX_RENDER_DISTANCE * MAX_RENDER_DISTANCE;
    
    // BACK-FACE CULLING DESATIVADO
    // Vector3 cameraPos = {playerX, playerY + 1.6f, playerZ};
    
    // Itera sobre todos os blocos do chunk
    for (int32_t localY = 0; localY < CHUNK_SIZE_Y; localY++) {
        for (int32_t localZ = 0; localZ < CHUNK_SIZE_Z; localZ++) {
            for (int32_t localX = 0; localX < CHUNK_SIZE_X; localX++) {
                Voxel voxel = Chunk_GetBlock(chunk, localX, localY, localZ);
                if (voxel.type == BLOCK_AIR) continue;
                
                // Converte para coordenadas globais
                int32_t globalX, globalY, globalZ;
                Chunk_LocalToGlobal(chunkX, chunkZ, localX, localY, localZ,
                                   &globalX, &globalY, &globalZ);
                
                // DISTANCE CULLING DESATIVADO - renderiza todos os blocos
                // float dx = (float)globalX + 0.5f - playerX;
                // float dy = (float)globalY + 0.5f - playerY;
                // float dz = (float)globalZ + 0.5f - playerZ;
                // float distSq = dx * dx + dy * dy + dz * dz;
                // if (distSq > MAX_DISTANCE_SQ) continue;
                
                Vector3 blockPos = {(float)globalX, (float)globalY, (float)globalZ};
                // BACK-FACE CULLING DESATIVADO
                // Vector3 blockCenter = {(float)globalX + 0.5f, (float)globalY + 0.5f, (float)globalZ + 0.5f};
                
                BlockColor blockColor = VoxelRenderer_GetBlockColor(voxel.type);
                Color color = {blockColor.r, blockColor.g, blockColor.b, blockColor.a};
                
                // OCCLUSION CULLING E BACK-FACE CULLING DESATIVADOS
                // Renderiza todas as faces de todos os blocos
                AddFace(mesh, blockPos, FACE_NEGATIVE_X, color);
                AddFace(mesh, blockPos, FACE_POSITIVE_X, color);
                AddFace(mesh, blockPos, FACE_NEGATIVE_Y, color);
                AddFace(mesh, blockPos, FACE_POSITIVE_Y, color);
                AddFace(mesh, blockPos, FACE_NEGATIVE_Z, color);
                AddFace(mesh, blockPos, FACE_POSITIVE_Z, color);
                
                /* CÓDIGO ORIGINAL COM CULLING (DESATIVADO)
                // Verifica faces adjacentes (occlusion culling)
                // Só renderiza faces que não têm bloco sólido adjacente
                
                // Face -X (esquerda)
                if (!IsBlockSolid(world, globalX - 1, globalY, globalZ)) {
                    Vector3 normal = {-1.0f, 0.0f, 0.0f};
                    if (ShouldRenderFace(normal, cameraPos, blockCenter)) {
                        AddFace(mesh, blockPos, FACE_NEGATIVE_X, color);
                    }
                }
                
                // Face +X (direita)
                if (!IsBlockSolid(world, globalX + 1, globalY, globalZ)) {
                    Vector3 normal = {1.0f, 0.0f, 0.0f};
                    if (ShouldRenderFace(normal, cameraPos, blockCenter)) {
                        AddFace(mesh, blockPos, FACE_POSITIVE_X, color);
                    }
                }
                
                // Face -Y (chão) - sempre renderiza se não tiver bloco abaixo
                if (!IsBlockSolid(world, globalX, globalY - 1, globalZ)) {
                    Vector3 normal = {0.0f, -1.0f, 0.0f};
                    if (ShouldRenderFace(normal, cameraPos, blockCenter)) {
                        AddFace(mesh, blockPos, FACE_NEGATIVE_Y, color);
                    }
                }
                
                // Face +Y (teto) - só renderiza se não tiver bloco acima E se a câmera estiver abaixo
                if (!IsBlockSolid(world, globalX, globalY + 1, globalZ)) {
                    // Só renderiza teto se a câmera estiver abaixo do bloco
                    if (cameraPos.y < blockCenter.y) {
                        Vector3 normal = {0.0f, 1.0f, 0.0f};
                        if (ShouldRenderFace(normal, cameraPos, blockCenter)) {
                            AddFace(mesh, blockPos, FACE_POSITIVE_Y, color);
                        }
                    }
                }
                
                // Face -Z (traseira)
                if (!IsBlockSolid(world, globalX, globalY, globalZ - 1)) {
                    Vector3 normal = {0.0f, 0.0f, -1.0f};
                    if (ShouldRenderFace(normal, cameraPos, blockCenter)) {
                        AddFace(mesh, blockPos, FACE_NEGATIVE_Z, color);
                    }
                }
                
                // Face +Z (frontal)
                if (!IsBlockSolid(world, globalX, globalY, globalZ + 1)) {
                    Vector3 normal = {0.0f, 0.0f, 1.0f};
                    if (ShouldRenderFace(normal, cameraPos, blockCenter)) {
                        AddFace(mesh, blockPos, FACE_POSITIVE_Z, color);
                    }
                }
                */
            }
        }
    }
}

void VoxelMesh_Render(VoxelMesh* mesh, Camera3D* camera) {
    if (!mesh || !mesh->initialized || !camera || mesh->vertexCount == 0) return;
    
    // Renderiza usando DrawTriangle3D para cada triângulo
    // DrawTriangle3D renderiza faces sólidas (não wireframe)
    for (int32_t i = 0; i < mesh->vertexCount; i += 3) {
        if (i + 2 >= mesh->vertexCount) break;
        
        Vector3 v1 = mesh->vertices[i].position;
        Vector3 v2 = mesh->vertices[i + 1].position;
        Vector3 v3 = mesh->vertices[i + 2].position;
        Color color = mesh->vertices[i].color;
        
        // DrawTriangle3D renderiza triângulos sólidos com cor
        DrawTriangle3D(v1, v2, v3, color);
    }
}
