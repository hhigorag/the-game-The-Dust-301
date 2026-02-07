# Checklist de Debug - Gameplay Engine

## Problemas Comuns e Soluções

### 1. W anda para trás
**Sintoma:** Ao pressionar W com yaw=0, player move em -Z em vez de +Z.

**Causa:** Sinal errado na fórmula de forward.

**Solução:**
- Verifique `CameraFP_GetForward()`: deve retornar `{sin(yaw)*cos(pitch), sin(pitch), cos(yaw)*cos(pitch)}`
- Com yaw=0, pitch=0: forward deve ser `{0, 0, 1}` (Z positivo)
- Se estiver invertido, troque o sinal de `cos(yaw)` ou `sin(yaw)`

### 2. A/D invertidos
**Sintoma:** A move para direita, D move para esquerda (ou vice-versa).

**Causa:** Ordem errada no cross product.

**Solução:**
- Verifique `CameraFP_GetRight()`: deve usar `cross(forward, up)` (right-handed)
- Se estiver invertido, troque para `cross(up, forward)`
- Teste: com yaw=0, A deve mover X negativo, D deve mover X positivo

### 3. Yaw=0 não olha +Z
**Sintoma:** Com yaw=0, câmera não olha para frente (+Z).

**Causa:** Fórmula de forward incorreta.

**Solução:**
- Verifique `Vec3_FromYawPitch()` ou `CameraFP_GetForward()`
- Com yaw=0 (radianos), forward deve ser `{0, 0, 1}`
- Se estiver errado, ajuste a fórmula:
  - `forwardX = sin(yaw) * cos(pitch)`
  - `forwardY = sin(pitch)`
  - `forwardZ = cos(yaw) * cos(pitch)`

### 4. Pitch afeta movimento horizontal
**Sintoma:** Ao olhar para cima/baixo, W move o player para cima/baixo.

**Causa:** Movimento não está usando `forward_flat`.

**Solução:**
- Verifique `CameraFP_CalculateWishDir()`: deve usar `GetForwardFlat()` e `GetRightFlat()`
- `forward_flat` projeta forward no plano XZ (remove componente Y)
- Movimento deve ser apenas no plano XZ

### 5. Movimento diagonal muito rápido
**Sintoma:** W+D move mais rápido que W ou D sozinhos.

**Causa:** Wishdir não está sendo normalizado.

**Solução:**
- Verifique `CameraFP_CalculateWishDir()`: deve normalizar o resultado
- `Vec3_Normalize()` garante que movimento diagonal tem mesma velocidade

### 6. Frustum culling não funciona
**Sintoma:** Chunks/blocos aparecem mesmo fora da tela.

**Causa:** Frustum calculado incorretamente ou teste de culling errado.

**Solução:**
- Verifique `Frustum_Calculate()`: planos devem estar corretos
- Teste com `Frustum_IsPointInside()`: ponto na frente deve retornar true
- Verifique distância: `Frustum_DistSqToCamera()` deve usar distância² (sem sqrt)

### 7. Pop-in de chunks
**Sintoma:** Chunks aparecem/desaparecem abruptamente.

**Causa:** Margem anti-pop muito pequena ou distância de culling muito exata.

**Solução:**
- Aumente `ANTI_POP_MARGIN` em `frustum.c`
- Use `EFFECTIVE_FAR = MAX_FAR_DIST + ANTI_POP_MARGIN` para culling
- Teste com chunks em diferentes distâncias

## Tabela de Validação

| Yaw (graus) | Pitch (graus) | W move | A move | D move |
|-------------|---------------|--------|--------|--------|
| 0           | 0             | +Z     | -X     | +X     |
| 90          | 0             | +X     | -Z     | +Z     |
| -90         | 0             | -X     | +Z     | -Z     |
| 180         | 0             | -Z     | +X     | -X     |
| 0           | +60           | +Z     | -X     | +X     |
| 0           | -60           | +Z     | -X     | +X     |

**Nota:** Pitch NÃO deve afetar movimento horizontal (W/S/A/D sempre no plano XZ).

## Testes Automáticos

Execute `main.cpp` para testes automáticos:
- Teste 1: Direção de movimento (W/S em diferentes yaws)
- Teste 2: Pitch não afeta movimento
- Teste 3: A/D não invertidos
- Teste 4: Frustum culling
- Teste 5: Chunk culling
