#version 150 core

in vec2 inPosition;

uniform sampler2D elevationMap;
uniform sampler2D normalMap;
uniform vec2 spacing;
uniform int tileSize;

uniform vec2 lowerLeft;

out float elevation;
out float lighting;
out float vsUncertainty;
out float isnull;

uniform mat4 matrix;
uniform mat3 normMatrix;
uniform vec3 lightDirection;
uniform float minElevation;
uniform float maxElevation;

void main() {
    vec2 tc = inPosition/float(tileSize);
    float e = texture(elevationMap,tc).r;

    vec3 normal = normalize((texture(normalMap,tc).rgb*2.0)-1.0);
    vec3 vsNormal = normalize(normMatrix*normal);
    lighting = max(dot(vsNormal,lightDirection), 0.0);
    //lighting = 1.0;
    //vsUncertainty = uncertainty;

    if(e > 9999.0)
    {
        isnull = 1.0;
    }
    else
    {
        isnull = 0.0;
    }

    elevation = (e-minElevation)/(maxElevation-minElevation);
    vec2 posMeters = inPosition*spacing+lowerLeft;
    gl_Position = matrix * vec4(posMeters.x,posMeters.y,e,1.0);
}