#version 330

// Fog em post-process usando DEPTH LINEARIZADO.
// O depth do hardware (gl_FragCoord.z / depth buffer) é NÃO-LINEAR;
// usar direto no mix causa faixas e degraus. Este shader lineariza antes.

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;     // Cor da cena
uniform sampler2D textureDepth; // Depth buffer (valor 0..1, não-linear)
uniform float fogStart;
uniform float fogEnd;
uniform float cameraNear;
uniform float cameraFar;
uniform vec3 fogColor;

// Lineariza depth. Depth buffer OpenGL (NDC 0..1) é não-linear.
// Fórmula DirectX/OpenGL window-space [0,1]:
//   d_linear = (near * far) / (far - z01 * (far - near))
// Alternativa OpenGL NDC [-1,1]: zNdc = z01*2 - 1 e
//   d_linear = (2*near*far) / (far + near - zNdc*(far-near))
float LinearizeDepth01(float z01, float near, float far) {
    return (near * far) / (far - z01 * (far - near));
}

void main() {
    vec4 color = texture(texture0, fragTexCoord);
    float z01 = texture(textureDepth, fragTexCoord).r;
    float d = LinearizeDepth01(z01, cameraNear, cameraFar);

    float fogRange = fogEnd - fogStart;
    float fogT = (fogRange > 0.0001) ? clamp((d - fogStart) / fogRange, 0.0, 1.0) : 1.0;

    finalColor = vec4(mix(color.rgb, fogColor, fogT), color.a) * fragColor;
}
