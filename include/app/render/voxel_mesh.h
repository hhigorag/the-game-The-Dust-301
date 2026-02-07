#ifndef VOXEL_MESH_H
#define VOXEL_MESH_H

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
typedef struct Camera3D Camera3D;

// Vértice de um triângulo
typedef struct {
    Vector3 position;
    Vector3 normal;
    Color color;
} VoxelVertex;

// Face de um bloco (2 triângulos)
typedef struct {
    VoxelVertex vertices[6]; // 2 triângulos = 6 vértices
    int32_t vertexCount;     // Pode ser 6 ou menos se otimizado
} VoxelFace;

// Direções das faces (6 faces de um cubo)
typedef enum {
    FACE_NEGATIVE_X = 0,  // Face esquerda (-X)
    FACE_POSITIVE_X = 1,  // Face direita (+X)
    FACE_NEGATIVE_Y = 2,  // Face inferior (-Y) - CHÃO
    FACE_POSITIVE_Y = 3,  // Face superior (+Y) - TETO (não renderiza se não visível)
    FACE_NEGATIVE_Z = 4,  // Face traseira (-Z)
    FACE_POSITIVE_Z = 5   // Face frontal (+Z)
} FaceDirection;

// Sistema de mesh para voxels
typedef struct {
    VoxelVertex* vertices;
    int32_t vertexCount;
    int32_t vertexCapacity;
    bool initialized;
} VoxelMesh;

// Inicializa o sistema de mesh
void VoxelMesh_Init(VoxelMesh* mesh);

// Limpa o mesh
void VoxelMesh_Clear(VoxelMesh* mesh);

// Destrói o mesh
void VoxelMesh_Destroy(VoxelMesh* mesh);

// Gera mesh de um chunk (apenas faces visíveis)
void VoxelMesh_GenerateChunk(VoxelMesh* mesh, VoxelWorld* world, 
                             int32_t chunkX, int32_t chunkZ,
                             float playerX, float playerY, float playerZ);

// Renderiza o mesh
void VoxelMesh_Render(VoxelMesh* mesh, Camera3D* camera);

#endif // VOXEL_MESH_H
