void main()
{
    // Transforming The Vertex
    gl_Position = gl_Vertex;
    gl_TexCoord[0] = gl_Vertex * 0.5 + 0.5;
}
