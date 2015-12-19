#version 400 core

layout (quads, equal_spacing, ccw) in;

uniform sampler2D elevationMap;
uniform sampler2D normalMap;
uniform vec2 spacing;
uniform int tileSize;
uniform vec2 lowerLeft;

uniform sampler2D eastElevationMap;
uniform sampler2D eastNormalMap;
uniform vec2 eastSpacing;
uniform int eastTileSize;
uniform vec2 eastLowerLeft;
uniform bool hasEast;

uniform sampler2D northElevationMap;
uniform sampler2D northNormalMap;
uniform vec2 northSpacing;
uniform int northTileSize;
uniform vec2 northLowerLeft;
uniform bool hasNorth;

uniform sampler2D northEastElevationMap;
uniform sampler2D northEastNormalMap;
uniform vec2 northEastSpacing;
uniform int northEastTileSize;
uniform vec2 northEastLowerLeft;
uniform bool hasNorthEast;

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
    float e = 1e6;
    vec2 posMeters = gl_TessCoord.xy*(tileSize-1)*spacing+lowerLeft;
    vec3 normal = normalize((texture(normalMap,gl_TessCoord.xy).rgb*2.0)-1.0);

    if(gl_TessCoord.x == 1.0 && gl_TessCoord.y == 1.0)
    {
        if(hasNorthEast)
        {
            e = texture(northEastElevationMap,vec2(0.0,0.0)).r;
            posMeters = northEastLowerLeft;
            normal = normalize((texture(northEastNormalMap,vec2(0.0,0.0)).rgb*2.0)-1.0);
        }
    }
    else if(gl_TessCoord.x == 1.0)
    {
        if(hasEast)
        {
            e = texture(eastElevationMap,vec2(0.0,gl_TessCoord.y)).r;
            posMeters.x = eastLowerLeft.x;
            posMeters.y = gl_TessCoord.y*(eastTileSize-1)*eastSpacing+eastLowerLeft.y;
            normal = normalize((texture(eastNormalMap,vec2(0.0,gl_TessCoord.y)).rgb*2.0)-1.0);
        }
    }
    else if(gl_TessCoord.y == 1.0)
    {
        if(hasNorth)
        {
            e = texture(northElevationMap,vec2(gl_TessCoord.x,0.0)).r;
            posMeters.x = gl_TessCoord.x*(northTileSize-1)*northSpacing+northLowerLeft.x;
            posMeters.y = northLowerLeft.y;
            normal = normalize((texture(northNormalMap,vec2(gl_TessCoord.x,0.0)).rgb*2.0)-1.0);
        }
    }
    else
    {
        e = texture(elevationMap,gl_TessCoord.xy).r;
    }

    vec3 vsNormal = normalize(normMatrix*normal);
    lighting = max(dot(vsNormal,lightDirection), 0.0);
    elevation = (e-minElevation)/(maxElevation-minElevation);
    worldPosition = vec3(posMeters,e);
    gl_Position = matrix * vec4(posMeters.x,posMeters.y,e,1.0);
}
