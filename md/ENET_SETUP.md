# Configuração do ENet

O projeto está configurado para compilar **sem ENet** (modo stub) por padrão. Para habilitar a rede real com ENet:

## Instalação do ENet

### Opção 1: Baixar ENet pré-compilado

1. Baixe ENet de: https://github.com/lsalzman/enet/releases
2. Extraia para `external/enet/`
3. Estrutura esperada:
   ```
   external/enet/
     include/
       enet/
         enet.h
     lib/
       libenet.a (ou enet.lib)
   ```

### Opção 2: Compilar ENet do código-fonte

1. Clone o repositório:
   ```bash
   git clone https://github.com/lsalzman/enet.git external/enet
   ```

2. Compile com MinGW:
   ```bash
   cd external/enet
   ./configure
   make
   ```

3. Copie os arquivos:
   - Headers: `include/enet/enet.h` → `external/enet/include/enet/enet.h`
   - Library: `.libs/libenet.a` → `external/enet/lib/libenet.a`

## Habilitar ENet no Makefile

Quando tiver ENet instalado, edite o `Makefile`:

1. Descomente as linhas de ENet:
   ```makefile
   # CFLAGS += -DUSE_ENET
   # INCLUDES += -I$(ENET_INCLUDE)
   # LDFLAGS += -L$(ENET_LIB)
   # LIBS += -lenet -lws2_32
   ```

2. Torne-as:
   ```makefile
   CFLAGS += -DUSE_ENET
   INCLUDES += -I$(ENET_INCLUDE)
   LDFLAGS += -L$(ENET_LIB)
   LIBS += -lenet -lws2_32
   ```

3. Recompile:
   ```bash
   mingw32-make clean
   mingw32-make all
   ```

## Modo Stub (Atual)

Atualmente o projeto compila em **modo stub**:
- ✅ Compila sem erros
- ✅ API de rede disponível
- ⚠️ Rede não funciona (stubs retornam sucesso mas não fazem nada)
- ✅ Terminal Lobby mostra opções Host/Join
- ⚠️ Conexões são simuladas

## Testando

Com ENet habilitado:
1. Execute duas instâncias do jogo
2. Na primeira: Terminal Lobby → "Host Game"
3. Na segunda: Terminal Lobby → "Join Game (localhost)"
4. As duas devem conectar e mostrar no lobby
