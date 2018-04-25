#version 150 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VertexData
{
    float elevation;
    float lighting;
    float uncertainty;
    vec3 worldPosition;
} inData[];


out VertexData
{
    float elevation;
    float lighting;
    float uncertainty;
    vec3 worldPosition;
} outData;

void main()
{
    int n;
    float maxElevation = inData[0].elevation;
    for (n = 0; n < gl_in.length(); n++)
        if(inData[n].elevation > maxElevation)
            maxElevation = inData[n].elevation;

    // only generate a triangle if all verticies have a valid elevation.
    if(maxElevation <= 1.0)
    {
        for (n = 0; n < gl_in.length(); n++)
        {
            gl_Position = gl_in[n].gl_Position;
            outData.elevation = inData[n].elevation;
            outData.lighting = inData[n].lighting;
            outData.worldPosition = inData[n].worldPosition;
            EmitVertex();
        }
    }
}
