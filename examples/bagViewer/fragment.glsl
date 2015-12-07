
varying float lighting;
varying float elevation;
varying float isnull;

uniform sampler2D colorMap;

void main() {
    if(isnull > 0.0)
        discard;
    vec2 tc;
    tc.x = elevation;
    tc.y = .5;
    vec4 color = texture2D(colorMap,tc);
    color.rgb *= lighting;
    color.a = 1.0;
    //color.r = elevation;
    gl_FragColor = color;
}
