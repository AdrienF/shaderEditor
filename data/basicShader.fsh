
void main()
{
    vec2 pos = gl_FragCoord.xy;
    gl_FragColor = vec4(pos.x, 1.0, pos.y, 1.0);
}
