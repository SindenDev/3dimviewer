varying vec4 fragWorldPosition;

void main()
{
    // Transforming The Vertex
    gl_Position = ftransform();
    fragWorldPosition = vec4(gl_Vertex.xyz, 1.0);
}
