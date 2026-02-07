#version 330

// Sky gradient: t = dir.y*0.5+0.5, mix(bottom, top, t). 3 paradas: bottom, horizon, top.

in vec3 rayDir;

out vec4 finalColor;

uniform vec3 skyBottom;
uniform vec3 skyHorizon;
uniform vec3 skyTop;

void main() {
    float t = clamp(rayDir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 sky;
    if (t < 0.5) {
        sky = mix(skyBottom, skyHorizon, t * 2.0);
    } else {
        sky = mix(skyHorizon, skyTop, (t - 0.5) * 2.0);
    }

    // Dither leve (8-bit banding)
    float n = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
    sky += (n - 0.5) / 255.0;

    finalColor = vec4(clamp(sky, 0.0, 1.0), 1.0);
    // Sky atrás: rlDisableDepthMask na CPU evita write; 3D desenha por cima.
}
