#version 330

// Fog no forward: view-space depth linear, sem depth texture.
// Nomes e atributos iguais ao default raylib para o batch.

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;
out float viewPosZ;
out float viewPosY;

uniform mat4 mvp;
uniform mat4 matView;

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    vec4 viewPos = matView * vec4(vertexPosition, 1.0);
    viewPosZ = viewPos.z;
    viewPosY = viewPos.y;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
