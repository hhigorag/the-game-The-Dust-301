# Checklist ENet - Verificação Rápida

## ✅ Estrutura de Arquivos

- [x] `src/core/net/net.c` - Implementação da rede
- [x] `include/core/net/net.h` - Header da rede
- [x] `include/core/net/protocol.h` - Protocolo de pacotes

## ✅ Inicialização no main.c

- [x] `Net_Init(&net)` chamado (atualmente via `Core_Init()`)
- [x] `Net_Poll(net)` chamado todo frame (atualmente via `Core_Tick()`)

## ✅ Botões Host/Join

- [x] Terminal Lobby → "Host Game" → `Net_StartHost(net, &cfg, 8)`
- [x] Terminal Lobby → "Join Game (localhost)" → `Net_Connect(net, &cfg)`
- [x] Terminal Lobby → "Join Game (IP)" → `Net_Connect(net, &cfg)`

## ⚠️ Compilação com ENet

Para compilar com ENet real, verifique:

1. **Include Path:**
   ```makefile
   INCLUDES += -I$(ENET_INCLUDE)
   ```
   Deve apontar para `external/enet/include/` onde está `enet/enet.h`

2. **Library Path:**
   ```makefile
   LDFLAGS += -L$(ENET_LIB)
   ```
   Deve apontar para `external/enet/lib/` onde está `libenet.a` ou `enet.lib`

3. **Linking:**
   ```makefile
   LIBS += -lenet -lws2_32
   ```
   - `-lenet` → procura `libenet.a` ou `enet.lib`
   - `-lws2_32` → Windows sockets (obrigatório para ENet no Windows)

4. **Define:**
   ```makefile
   CFLAGS += -DUSE_ENET
   ```
   Habilita o código real do ENet (remove stubs)

## 📝 Estrutura Esperada do ENet

```
external/enet/
  include/
    enet/
      enet.h          ← #include <enet/enet.h> procura aqui
  lib/
    libenet.a         ← -lenet procura aqui (MinGW)
    enet.lib          ← ou aqui (MSVC)
```

## 🔍 Como Testar

1. **Sem ENet (modo stub atual):**
   - Compila e executa
   - Botões funcionam mas conexões são simuladas

2. **Com ENet:**
   - Descomente linhas do ENet no Makefile
   - Recompile: `mingw32-make clean && mingw32-make all`
   - Execute duas instâncias
   - Host → "Host Game"
   - Cliente → "Join Game (localhost)"
   - Devem conectar e mostrar no lobby
