
void main()
{
    //normalized coord in window space
    vec2 nCoord = gl_FragCoord.xy / iResolution.xy;
    gl_FragColor = vec4(nCoord.x, nCoord.y, 0., 1.0);
}
