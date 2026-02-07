# Como Compilar ENet do Zero (Método Recomendado)

## Passo 1: Baixar o ENet

Abra o terminal (PowerShell ou Git Bash) e vá para a pasta do projeto:

```bash
cd c:\Users\Higor\Desktop\game\external
git clone https://github.com/lsalzman/enet.git
```

Isso vai criar: `external/enet/`

## Passo 2: Compilar ENet com MinGW (CMake)

**PRECISA de:**
- MinGW (gcc) ✅ (já tem)
- CMake instalado

**Dentro da pasta do ENet:**

```bash
cd enet
mkdir build
cd build
```

**Agora gere com MinGW:**

```bash
cmake .. -G "MinGW Makefiles"
```

**Se isso funcionar, compile:**

```bash
mingw32-make
```

## Passo 3: Verificação Crítica

**Depois do build, você PRECISA TER:**

```
external/enet/build/
  libenet.a        ✅ (essa é a lib REAL)
```

**E os headers:**

```
external/enet/include/enet/
  enet.h
  types.h
  time.h
  protocol.h
```

**Se `libenet.a` não existir, ainda está em stub.**

## Passo 4: Ajustar o Makefile

O Makefile já foi atualizado com:

```makefile
ENET_PATH = $(EXTERNAL_DIR)/enet
ENET_BUILD = $(ENET_PATH)/build
ENET_INCLUDE = $(ENET_PATH)/include

CFLAGS += -DUSE_ENET
INCLUDES += -I$(ENET_INCLUDE)
LDFLAGS += -L$(ENET_BUILD)
LIBS += -lenet -lws2_32
```

## Passo 5: Teste Definitivo (Anti-Stub)

Crie um arquivo temporário `enet_test.c`:

```c
#include <enet/enet.h>
#include <stdio.h>

int main(void) {
    if (enet_initialize() != 0) {
        printf("ENet init FAILED\n");
        return 1;
    }
    printf("ENet OK (real lib)\n");
    enet_deinitialize();
    return 0;
}
```

**Compile manualmente:**

```bash
gcc enet_test.c -Iexternal/enet/include -Lexternal/enet/build -lenet -lws2_32 -o enet_test.exe
```

**Se rodar e imprimir:**

```
ENet OK (real lib)
```

**✅ Acabou o stub, ENet tá 100% funcional!**

## Passo 6: Compilar o Projeto

Depois que o teste funcionar:

```bash
mingw32-make clean
mingw32-make all
```

O projeto vai compilar com ENet real (sem stubs).

## Troubleshooting

### CMake não encontrou CMakeLists.txt
- Verifique se o clone do git funcionou
- O diretório `external/enet/` deve ter arquivos como `CMakeLists.txt`, `include/`, `src/`, etc.

### libenet.a não foi gerado
- Verifique se o `mingw32-make` completou sem erros
- Procure por `libenet.a` em `external/enet/build/`

### Headers não encontrados
- Verifique se `external/enet/include/enet/enet.h` existe
- Se não existir, o clone pode ter falhado
