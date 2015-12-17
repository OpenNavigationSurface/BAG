#version 150 core

in float glighting;
in float gelevation;
//in float gisnull;

uniform sampler2D colorMap;

out vec4 fragColor;

void main() {
//    if(gisnull > 0.0)
//        discard;
    vec2 tc;
    tc.x = gelevation;
    tc.y = .5;
    vec4 color = texture(colorMap,tc);
    color.rgb *= glighting;
    color.a = 1.0;
    fragColor = color;
}
