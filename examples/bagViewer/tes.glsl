#version 400 core

layout (quads, equal_spacing, ccw) in;

uniform sampler2D elevationMap;
uniform sampler2D normalMap;
uniform vec2 spacing;
uniform int tileSize;

uniform vec2 lowerLeft;

uniform mat4 matrix;
uniform mat3 normMatrix;
uniform vec3 lightDirection;
uniform float minElevation;
uniform float maxElevation;

out VertexData
{
    float elevation;
    float lighting;
    float uncertainty;
    vec3 worldPosition;
};

void main()
{
    float e = texture(elevationMap,gl_TessCoord.xy).r;

    vec3 normal = normalize((texture(normalMap,gl_TessCoord.xy).rgb*2.0)-1.0);
    vec3 vsNormal = normalize(normMatrix*normal);
    lighting = max(dot(vsNormal,lightDirection), 0.0);
    elevation = (e-minElevation)/(maxElevation-minElevation);
    vec2 posMeters = gl_TessCoord.xy*(tileSize-1)*spacing+lowerLeft;
    worldPosition = vec3(posMeters,e);
    gl_Position = matrix * vec4(posMeters.x,posMeters.y,e,1.0);
}
