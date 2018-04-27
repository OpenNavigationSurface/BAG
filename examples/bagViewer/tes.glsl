#version 400 core

layout (quads, equal_spacing, ccw) in;

uniform sampler2D elevationMap;
uniform sampler2D normalMap;
uniform vec2 spacing;
uniform vec2 tileSize;
uniform vec2 lowerLeft;

uniform sampler2D eastElevationMap;
uniform sampler2D eastNormalMap;
uniform vec2 eastSpacing;
uniform vec2 eastTileSize;
uniform vec2 eastLowerLeft;
uniform bool hasEast;

uniform sampler2D northElevationMap;
uniform sampler2D northNormalMap;
uniform vec2 northSpacing;
uniform vec2 northTileSize;
uniform vec2 northLowerLeft;
uniform bool hasNorth;

uniform sampler2D northEastElevationMap;
uniform sampler2D northEastNormalMap;
uniform vec2 northEastSpacing;
uniform vec2 northEastLowerLeft;
uniform bool hasNorthEast;

uniform mat4 matrix;
uniform mat3 normMatrix;
uniform vec3 lightDirection;
uniform float minElevation;
uniform float maxElevation;

out VertexData
{
    float elevation; // normalized elevation (0-1)
    float lighting;
    float uncertainty;
    vec3 worldPosition; // position in meters
};

void main()
{
    float e = 1e6;
    vec2 posMeters = gl_TessCoord.xy*tileSize*spacing+lowerLeft;
    vec2 texCoordFactor = (tileSize+1)/tileSize;
    vec2 eastTexCoordFactor = (eastTileSize+1)/eastTileSize;
    vec2 northTexCoordFactor = (northTileSize+1)/northTileSize;
    vec3 normal = normalize((texture(normalMap,gl_TessCoord.xy*texCoordFactor).rgb*2.0)-1.0);

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
            posMeters.y = gl_TessCoord.y*eastTileSize.y*eastSpacing.y+eastLowerLeft.y;
            normal = normalize((texture(eastNormalMap,vec2(0.0,gl_TessCoord.y*eastTexCoordFactor.y)).rgb*2.0)-1.0);
        }
    }
    else if(gl_TessCoord.y == 1.0)
    {
        if(hasNorth)
        {
            e = texture(northElevationMap,vec2(gl_TessCoord.x,0.0)).r;
            posMeters.x = gl_TessCoord.x*northTileSize.x*northSpacing.x+northLowerLeft.x;
            posMeters.y = northLowerLeft.y;
            normal = normalize((texture(northNormalMap,vec2(gl_TessCoord.x*northTexCoordFactor.x,0.0)).rgb*2.0)-1.0);
        }
    }
    else
    {
        e = texture(elevationMap,gl_TessCoord.xy*texCoordFactor).r;
    }

    vec3 vsNormal = normalize(normMatrix*normal);
    lighting = max(dot(vsNormal,lightDirection), 0.0);
    elevation = (e-minElevation)/(maxElevation-minElevation);
    worldPosition = vec3(posMeters,e);
    gl_Position = matrix * vec4(posMeters.x,posMeters.y,e,1.0);
}
