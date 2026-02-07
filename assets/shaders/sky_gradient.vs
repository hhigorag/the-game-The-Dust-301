#version 330

// Sky gradient por direção do raio: não usa depth. Esfera centrada na câmera.
// rayDir = normal do vértice (direção do centro para o fragmento).
// vertexTexCoord/vertexColor existem para compatibilidade com o batch raylib.

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

out vec3 rayDir;

uniform mat4 mvp;

void main() {
    rayDir = normalize(vertexPosition);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
