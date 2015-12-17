#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in float elevation[];
in float lighting[];
//in float isnull[];


out float gelevation;
out float glighting;
//out float gisnull;

void main()
{
    int n;
    // Loop over the input vertices
    float maxElevation = elevation[0];
    for (n = 0; n < gl_in.length(); n++)
        if(elevation[n] > maxElevation)
            maxElevation = elevation[n];

    if(maxElevation <= 1.0)
        for (n = 0; n < gl_in.length(); n++)
        {
            // Copy the input position to the output
            gl_Position = gl_in[n].gl_Position;
            gelevation = elevation[n];
            glighting = lighting[n];
            //gisnull = isnull[n];
            // Emit the vertex
            EmitVertex();
        }
}
