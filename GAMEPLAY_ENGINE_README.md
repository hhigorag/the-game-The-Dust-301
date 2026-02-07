# Gameplay Engine - Implementação Completa

## 📋 Estrutura de Arquivos

```
include/
├── core/
│   ├── math/
│   │   └── core_math.h          # Matemática básica (Vec3, operações)
│   ├── input/
│   │   └── core_input.h         # Sistema de input agnóstico
│   ├── gameplay/
│   │   └── player.h             # Controller do player
│   └── world/
│       └── chunk_system.h       # Stubs de integração com chunks
└── app/
    ├── camera/
    │   └── camera_fp.h          # Câmera FPS estilo Minecraft
    └── render/
        └── frustum.h            # Frustum culling

src/
├── core/
│   ├── math/
│   │   └── core_math.c
│   ├── input/
│   │   └── core_input.c
│   ├── gameplay/
│   │   └── player.c
│   └── world/
│       └── chunk_system.c
├── app/
│   ├── camera/
│   │   └── camera_fp.cpp        # C++ para câmera
│   └── render/
│       └── frustum.c
└── main.cpp                     # Testes e validação
```

## 🎯 Convenções Obrigatórias

### Sistema de Coordenadas
- **X** = direita
- **Y** = cima
- **Z** = frente (positivo = frente)

### Ângulos
- **Yaw** = rotação horizontal (0° = olhando +Z)
- **Pitch** = rotação vertical (limitado -89° a +89°)

### Movimento
- **W/S**: move para frente/trás (relativo ao yaw, ignorando pitch)
- **A/D**: move para esquerda/direita (relativo ao yaw)
- **Movimento usa `forward_flat`**: projetado no plano XZ (pitch não afeta)

## 🔧 Compilação

### Windows (MinGW)
```bash
g++ -std=c++11 -DUSE_RAYLIB -I./include \
    src/main.cpp \
    src/core/math/core_math.c \
    src/core/input/core_input.c \
    src/core/gameplay/player.c \
    src/app/camera/camera_fp.cpp \
    src/app/render/frustum.c \
    src/core/world/chunk_system.c \
    -lraylib -o test_gameplay.exe
```

### Linux
```bash
g++ -std=c++11 -DUSE_RAYLIB -I./include \
    src/main.cpp \
    src/core/math/core_math.c \
    src/core/input/core_input.c \
    src/core/gameplay/player.c \
    src/app/camera/camera_fp.cpp \
    src/app/render/frustum.c \
    src/core/world/chunk_system.c \
    -lraylib -o test_gameplay
```

### Sem Raylib (apenas testes)
```bash
g++ -std=c++11 -I./include \
    src/main.cpp \
    src/core/math/core_math.c \
    src/core/input/core_input.c \
    src/core/gameplay/player.c \
    src/app/camera/camera_fp.cpp \
    src/app/render/frustum.c \
    src/core/world/chunk_system.c \
    -o test_gameplay
```

## 📐 Fórmulas Principais

### Forward Vector (direção da câmera)
```c
forward = {
    sin(yaw) * cos(pitch),  // X
    sin(pitch),             // Y
    cos(yaw) * cos(pitch)   // Z
}
```

### Forward Flat (para movimento)
```c
forward_flat = normalize({forward.x, 0, forward.z})
```

### Right Vector
```c
right = normalize(cross(forward, up))  // right-handed
```

### Wish Direction (movimento desejado)
```c
wishdir = normalize(forward_flat * (W-S) + right_flat * (D-A))
```

## ✅ Testes Incluídos

O `main.cpp` contém 5 testes:

1. **Teste 1**: Direção de movimento (W/S em diferentes yaws)
2. **Teste 2**: Pitch não afeta movimento horizontal
3. **Teste 3**: A/D não invertidos
4. **Teste 4**: Frustum culling
5. **Teste 5**: Chunk culling

Execute para validar:
```bash
./test_gameplay
```

## 🐛 Debug

Consulte `CHECKLIST_DEBUG.md` para problemas comuns:
- W anda para trás
- A/D invertidos
- Yaw=0 não olha +Z
- Pitch afeta movimento
- Frustum culling não funciona

## 🔗 Integração com Projeto Existente

### 1. Substituir câmera atual
```c
#include "app/camera/camera_fp.h"

CameraFP camera;
CameraFP_Init(&camera, startPos);

// No loop:
Vec3 mouseDelta = GetMouseDelta();
CameraFP_UpdateMouse(&camera, mouseDelta.x, mouseDelta.y);

Vec3 wishdir = CameraFP_CalculateWishDir(&camera, forwardAmount, rightAmount);
Player_ApplyMovement(&player, wishdir, dt);
```

### 2. Integrar frustum culling
```c
Frustum frustum;
Vec3 camForward = CameraFP_GetForward(&camera);
Vec3 camUp = CameraFP_GetUp(&camera);

Frustum_Calculate(&frustum, camera.position, camForward, camUp,
                 DegToRad(75.0f), aspect, 0.1f, 15.0f);

// Para cada chunk:
if (Frustum_ShouldRenderChunk(&frustum, chunkCenter, chunkMin, chunkMax)) {
    // Renderiza chunk
}
```

## 📝 Notas Importantes

1. **Movimento sempre no plano XZ**: Pitch não afeta movimento horizontal
2. **Far clampado a 15m**: Frustum usa 15m + margem anti-pop
3. **Distância ao quadrado**: Culling usa dist² para evitar sqrt
4. **Chunk-level culling**: Testa chunks primeiro, depois blocos individuais
5. **Normalização**: Wishdir sempre normalizado para movimento diagonal correto

## 🎮 Uso Básico

```c
// Inicialização
Player player;
CameraFP camera;
Player_Init(&player, Vec3_Make(0, 64, 0));
CameraFP_Init(&camera, Vec3_Make(0, 64, 0));

// Loop de jogo
float dt = GetFrameTime();

// Input
float forward = 0.0f, right = 0.0f;
if (IsKeyDown(KEY_W)) forward += 1.0f;
if (IsKeyDown(KEY_S)) forward -= 1.0f;
if (IsKeyDown(KEY_A)) right -= 1.0f;
if (IsKeyDown(KEY_D)) right += 1.0f;

// Atualiza câmera
Vector2 mouseDelta = GetMouseDelta();
CameraFP_UpdateMouse(&camera, mouseDelta.x, mouseDelta.y);

// Calcula movimento
Vec3 wishdir = CameraFP_CalculateWishDir(&camera, forward, right);
Player_ApplyMovement(&player, wishdir, dt);

// Atualiza posição da câmera
camera.position = player.position;
camera.position.y += 1.6f; // Altura dos olhos
```

## 📚 Documentação Adicional

- `CHECKLIST_DEBUG.md`: Guia de debug
- Comentários no código: Explicações detalhadas em cada função
- Testes em `main.cpp`: Exemplos práticos de uso
