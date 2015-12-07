
in vec2 inPosition;

uniform sampler2D elevationMap;
uniform vec2 spacing;
uniform int tileSize;

uniform vec2 lowerLeft;

varying float elevation;
varying float lighting;
varying float vsUncertainty;
varying float isnull;

uniform highp mat4 matrix;
uniform highp mat3 normMatrix;
uniform vec3 lightDirection;
uniform float minElevation;
uniform float maxElevation;

void main() {
   //vec3 vsNormal = normalize(normMatrix*normal);
   //lighting = max(dot(vsNormal,lightDirection), 0.0);
    lighting = 1.0;
    //vsUncertainty = uncertainty;
    vec2 tc = inPosition/float(tileSize);
    float e = texture2D(elevationMap,tc);
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