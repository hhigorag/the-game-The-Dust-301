#version 330

// Fog no forward: d = -viewPos.z. Linear ou exponencial (sci-fi): fogT = 1 - exp(-density*d).

in vec2 fragTexCoord;
in vec4 fragColor;
in float viewPosZ;
in float viewPosY;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;
uniform int fogType;     // 0 = linear, 1 = exponential
uniform float fogDensity;
// Fog por altura: mix horizonte (baixo) -> céu (alto) em viewPosY
uniform vec3 fogHorizonColor;
uniform vec3 fogSkyColor;
uniform float fogH0;
uniform float fogHRange;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 color = (texelColor * colDiffuse * fragColor).rgb;

    float d = max(0.0, -viewPosZ);
    float fogT;
    if (fogType == 1) {
        fogT = 1.0 - exp(-fogDensity * d);
        fogT = clamp(fogT, 0.0, 1.0);
    } else {
        float fogRange = fogEnd - fogStart;
        fogT = (fogRange > 0.0001) ? clamp((d - fogStart) / fogRange, 0.0, 1.0) : 0.0;
    }

    // Fog color por altura: horizonte (h=0) -> céu (h=1). Fallback: fogColor se fogHRange <= 0
    vec3 fogCol;
    if (fogHRange > 0.0001) {
        float h = clamp((viewPosY - fogH0) / fogHRange, 0.0, 1.0);
        fogCol = mix(fogHorizonColor, fogSkyColor, h);
    } else {
        fogCol = fogColor;
    }
    vec3 colorOut = mix(color, fogCol, fogT);

    // Dither leve para reduzir banding em 8-bit (barato, sem HDR)
    float n = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
    colorOut += (n - 0.5) / 255.0;

    finalColor = vec4(clamp(colorOut, 0.0, 1.0), texelColor.a * colDiffuse.a * fragColor.a);
}
