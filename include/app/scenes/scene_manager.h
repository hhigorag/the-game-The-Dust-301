#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

typedef enum SceneId {
    SCENE_MENU_MAIN = 0,
    SCENE_GAMEPLAY   = 1,
    SCENE_COUNT
} SceneId;

typedef struct Scene {
    void (*Init)(void);
    void (*Shutdown)(void);
    void (*Update)(float dt);
    void (*Draw)(void);
} Scene;

void SceneManager_Init(void);
void SceneManager_Shutdown(void);
void SceneManager_Change(SceneId next);
void SceneManager_Update(float dt);
void SceneManager_Draw(void);

SceneId SceneManager_GetCurrent(void);

#endif // SCENE_MANAGER_H
