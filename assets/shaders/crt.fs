#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time; // Adicione isso no seu main.c (GetTime)

void main() {
    // 1. Curvatura sutil (Mantida a sua original)
    vec2 uv = fragTexCoord - 0.5;
    float rSq = dot(uv, uv);
    uv *= 1.0 + 0.08 * rSq; 
    uv += 0.5;

    // 2. Efeito de instabilidade de fita VHS (Jitter horizontal)
    // Cria pequenos "pulos" horizontais aleatórios em linhas específicas
    float jitter = sin(time * 100.0 + uv.y * 10.0) * 0.0012 * step(0.98, sin(time * 5.0 + uv.y * 2.0));
    uv.x += jitter;

    // 3. Limites da tela
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // 4. Aberração Cromática (Efeito VHS de cores vazando)
        float chroOffset = 0.0015;
        float rCol = texture(texture0, uv + vec2(chroOffset, 0.0)).r;
        float gCol = texture(texture0, uv).g;
        float bCol = texture(texture0, uv - vec2(chroOffset, 0.0)).b;
        vec4 texel = vec4(rCol, gCol, bCol, 1.0);
        
        // 5. Glow (Seu brilho original ajustado para as novas cores)
        float dist = 0.0012;
        vec4 glow = texture(texture0, uv + vec2(dist, dist)) * 0.25;
        glow += texture(texture0, uv - vec2(dist, dist)) * 0.25;
        
        // 6. Scanlines (Sua versão original)
        float scanline = sin(uv.y * 720.0 * 1.2) * 0.04;
        
        // 7. Ruído Estático (Chuva analógica)
        float noise = (fract(sin(dot(uv + time, vec2(12.9898, 78.233))) * 43758.5453)) * 0.03;

        // 8. Composição Final
        vec3 colorRGB = texel.rgb + (glow.rgb * 1.5);
        colorRGB -= vec3(scanline);
        colorRGB += vec3(noise); // Adiciona o granulado VHS
        
        finalColor = vec4(colorRGB, 1.0) * fragColor;
    }
}