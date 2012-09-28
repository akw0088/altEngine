varying vec2  MCposition;
varying float LightIntensity;

void main()
{
    vec3  BrickColor, MortarColor;
    vec2  BrickSize;
    vec2  BrickPct;

    BrickColor = vec3(0.75, 0.0, 0.0);
    MortarColor = vec3(0.5, 0.5, 0.5);
    BrickSize = vec2(1.0, 2.0);
    BrickPct = vec2(0.5, 0.5);

    vec3  color;
    vec2  position, useBrick;
    
    position = MCposition / BrickSize;

    if (fract(position.y * 0.5) > 0.5)
        position.x += 0.5;

    position = fract(position);

    useBrick = step(position, BrickPct);

    color  = mix(MortarColor, BrickColor, useBrick.x * useBrick.y);
    color *= LightIntensity;
    gl_FragColor = vec4(color, 1.0);
}