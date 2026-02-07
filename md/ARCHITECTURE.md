# Arquitetura — Responsabilidades por Camada

Referência de **quem manda em quê**: do mais baixo (engine) ao gameplay.  
Use ao tocar em cenas, render, atmosphere, world, entity ou ao avaliar includes/dependências.

---

## 1. CORE / ENGINE (`core/`)

**Responsabilidade:** loop principal, tempo, estado global mínimo, bootstrap da engine.

- Inicializar janela / contexto GL  
- Gerenciar delta time  
- Controlar shutdown  
- Chamar `Scene_Update` / `Scene_Draw`

**Não sabe:** player, mapa, fog, céu, etc.

---

## 2. PLATFORM (`platform/`)

**Responsabilidade:** dependências do SO/backend, abstração de input, janela, tempo se necessário.

- Tudo que não deve se misturar com lógica de jogo

---

## 3. SCENE / GAME FLOW (`scene/`)

**Responsabilidade:** fluxo de jogo, decidir o que é desenhado e quando, ligar sistemas (render, gameplay, atmosfera).

**Exemplo: `scene_gameplay.c`**

| Fase   | Faz                                                                 |
|--------|---------------------------------------------------------------------|
| **Init**  | Carrega shaders, inicializa atmosfera, inicializa player           |
| **Update**| Input → player, atualiza câmera                                     |
| **Draw**  | BeginMode3D → céu → mundo → player → EndMode3D                      |

- **Scene manda.**
- **Scene não calcula** iluminação, fog, física nem math de shader.

---

## 4. RENDER PIPELINE (`render/`)

**Responsabilidade:** *como* desenhar, não *o que* desenhar.

### 4.1 `render/lighting.c`

- Iluminação forward  
- **Não sabe** de fog nem sky  

**Hoje:** directional fake (wrap diffuse), hemispherical ambient.  
**Entrada:** normal, lightDir. **Saída:** fator de luz.

### 4.2 `fog_forward.fs` / `fog_forward.vs`

- Fog por fragment: linear / exponencial, height fog, dither  
- **Entrada:** cor base + distância + altura. **Saída:** cor final fogada

### 4.3 `atmosphere.c` / `atmosphere.h`

- Céu, cores atmosféricas, fallback CPU  
- `Atmosphere_Init`, `Atmosphere_DrawSky`, `Atmosphere_SetFog`, etc.

**Não desenha** mundo. **Não aplica** fog na geometria (shader faz).  
Só define como o ar e o céu se comportam.

### 4.4 `sky_gradient.vs` / `sky_gradient.fs`

- Shader do céu: gradient por direção do raio, dither  
- **Não depende** de depth. **Não sabe** que existe chão.

---

## 5. WORLD / MAP (`world/`)

**Responsabilidade:** geometria do mundo — grid, chunks, debug map.

- Desenha blocos, chão, paredes  
- **Não calcula** iluminação nem fog (isso é shader)

---

## 6. ENTITY / PLAYER (`entity/` ou `gameplay/`)

**Responsabilidade:** entidades — movimentação, pulo, colisão básica, estados (idle, move, jump).

- **Não desenha** céu  
- **Não sabe** o que é fog  
- **Só** se move e reage a input

---

## 7. CAMERA (`camera/`)

**Responsabilidade:** câmera FPS — view/projection, smoothing, clamp.

- **Entrada:** player  
- **Saída:** view matrix

---

## 8. ATMOSPHERE STATE (struct de dados)

**Responsabilidade:** dados configuráveis. **Sem** lógica pesada.

**Contém:**

- **fog:** type, fogStart / fogEnd, fogDensity, fogColor  
- **sky:** bottomColor, horizonColor, topColor  

A **scene** lê isso e repassa aos shaders.

---

## 9. DEBUG / DEV TOOLS (`debug/`)

**Responsabilidade:** HUD, overlays, FPS, toggles.

- Não influencia gameplay  
- Pode ficar fora do build final

---

## Relação entre as camadas

```
CORE
 └── Scene
      ├── Player / World
      ├── Camera
      ├── Atmosphere (dados)
      └── Render
           ├── Sky
           ├── Lighting
           └── Fog
```

- **Scene** manda  
- **Render** obedece  
- **Atmosphere** fornece parâmetros  
- **Shaders** fazem o trabalho pesado  
- Nenhum sistema se cruza sem permissão  

---

## Regra de ouro

> **Se um arquivo precisa incluir outro e isso “parece errado”, provavelmente é.**

Evitar includes que quebram as camadas (ex.: `lighting` incluindo `atmosphere`, `world` incluindo `render/lighting` para aplicar fog, etc.).
