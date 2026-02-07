#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;

void main() {
    // Coordenadas UV
    vec2 uv = fragTexCoord;
    
    // 1. Curvatura sutil da tela CRT
    vec2 centered = uv - 0.5;
    float dist = dot(centered, centered);
    uv = centered * (1.0 + 0.03 * dist) + 0.5;
    
    // Verifica limites para evitar distorção excessiva
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    
    // 2. Scanlines suaves (mais sutis que CRT tradicional)
    float scanline = sin(uv.y * 1080.0) * 0.02 + 1.0;
    
    // 3. Ruído estático muito leve (atmosfera analógica)
    float noise = fract(sin(dot(uv + time * 0.1, vec2(12.9898, 78.233))) * 43758.5453) * 0.015;
    
    // 4. Vinheta sutil nas bordas
    float vignette = 1.0 - smoothstep(0.3, 1.0, length(centered));
    vignette = mix(0.85, 1.0, vignette);
    
    // 5. Aberração cromática muito leve (efeito VHS sutil)
    float chromaOffset = 0.0008;
    float r = texture(texture0, uv + vec2(chromaOffset, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(chromaOffset, 0.0)).b;
    
    // 6. Composição final
    vec3 color = vec3(r, g, b);
    color *= scanline;
    color += noise;
    color *= vignette;
    
    // 7. Ajuste de brilho e contraste sutil
    color = pow(color, vec3(0.95));
    
    finalColor = vec4(color, 1.0) * fragColor;
}
