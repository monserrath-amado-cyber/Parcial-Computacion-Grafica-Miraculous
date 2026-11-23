#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

void main() {
    vec3 bottomColor = vec3(0.14f, 0.07f, 0.19f);
    vec3 midColor    = vec3(0.24f, 0.09f, 0.25f);
    vec3 topColor    = vec3(0.06f, 0.08f, 0.18f);
    vec3 col = mix(bottomColor, midColor, smoothstep(0.0f, 0.45f, TexCoord.y));
    col = mix(col, topColor, smoothstep(0.45f, 1.0f, TexCoord.y));
    FragColor = vec4(col, 1.0f);
}
