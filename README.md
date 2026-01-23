# Exploration Co-op Game

Jogo cooperativo em LAN com arquitetura host authoritative, menu inicial, terminal/lobby e gameplay com mapa procedural.

## Arquitetura

O projeto segue uma arquitetura limpa separando **Core** (lógica do jogo) de **App** (renderização/UI):

- **Core**: Lógica pura do jogo, sem dependências do raylib
- **App**: Renderização, UI, input e áudio usando raylib

## Estrutura do Projeto

```
game/
  assets/                # Arquivos de mídia
  external/              # Bibliotecas externas (enet, raylib)
  include/
    core/               # Headers do Core
    app/                # Headers do App
  src/
    main.c              # Loop principal único
    core/               # Lógica do jogo
    app/                # Renderização e UI
  build/                # Arquivos compilados
  Makefile              # Script de build
```

## Compilação

### Pré-requisitos

- MinGW-w64 (gcc)
- Raylib 5.5 instalado em `C:/Users/Higor/Desktop/raylib-5.5_win64_mingw-w64`

### Build

```bash
mingw32-make all
```

O executável será gerado em `build/game.exe`.

### Limpar

```bash
mingw32-make clean
```

## Estados do Jogo

- **MENU**: Menu inicial (Host/Join/Options/Quit)
- **LOBBY**: Terminal da nave (players conectados, configuração de seed/destino)
- **INGAME**: Gameplay cooperativo

## Sistema de Rede

- **Host Authoritative**: O host decide e replica todas as ações
- **Clientes**: Enviam input e recebem snapshots/eventos
- **Procedural**: Mapa gerado localmente por seed (determinístico)

## Desenvolvimento

Este é um projeto em desenvolvimento. A estrutura base está implementada:

- ✅ Loop único com SceneManager
- ✅ Separação Core/App
- ✅ Sistema de estados (Menu, Lobby, Gameplay)
- ✅ Sistema de rede (stub - será implementado com enet)
- ✅ Sistema de geração procedural (básico)

## Próximos Passos

- Implementar rede com enet
- Implementar geração procedural completa
- Adicionar gameplay completo
- Polimento de UI e efeitos
