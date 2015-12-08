#version 150 core

in float lighting;
in float elevation;
in float isnull;

uniform sampler2D colorMap;

out vec4 fragColor;

void main() {
    if(isnull > 0.0)
        discard;
    vec2 tc;
    tc.x = elevation;
    tc.y = .5;
    vec4 color = texture(colorMap,tc);
    color.rgb *= lighting;
    color.a = 1.0;
    fragColor = color;
}
