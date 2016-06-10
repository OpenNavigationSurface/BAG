#version 150 core

in VertexData
{
    float elevation;
    float lighting;
    float uncertainty;
    vec3 worldPosition;
};

uniform sampler2D colorMap;

out vec4 fragColor;

void main() {
    vec2 tc;
    tc.x = elevation;
    tc.y = .5;
    vec4 color = texture(colorMap,tc);
    color.rgb *= lighting;
    color.a = 1.0;
    fragColor = color;
}
