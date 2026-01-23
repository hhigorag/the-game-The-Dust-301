#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

// Tipos de cenas
typedef enum {
    SCENE_MENU,
    SCENE_TERMINAL_LOBBY,
    SCENE_GAMEPLAY,
    SCENE_COUNT
} SceneType;

// Inicializa o SceneManager
void SceneManager_Init(void);

// Define a cena atual
void SceneManager_SetScene(SceneType scene);

// Retorna a cena atual
SceneType SceneManager_GetCurrentScene(void);

// Atualiza a cena atual
void SceneManager_Update(float dt);

// Desenha a cena atual
void SceneManager_Draw(void);

#endif // SCENE_MANAGER_H
