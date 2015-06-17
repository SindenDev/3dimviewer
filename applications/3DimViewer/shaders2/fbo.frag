varying vec4 fragWorldPosition;

void main()
{
    vec4 wPos = fragWorldPosition;
    vec4 vPos = gl_ModelViewMatrix * wPos;
    vec4 pPos = gl_ProjectionMatrix * vPos;
    pPos /= pPos.w;

    gl_FragData[0] = vec4(pPos.xyz, 0.0);
}
