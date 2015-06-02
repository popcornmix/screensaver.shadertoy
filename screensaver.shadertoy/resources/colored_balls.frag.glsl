// See: http://glslsandbox.com/e#24875.0

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (fragCoord * 2.0 - iResolution.xy) / min(iResolution.x, iResolution.y);
    vec3 destColor = vec3(0.0);
    
    for(float i = 0.0; i < 10.0; i++){
        float j = i + 1.23;
        vec2 q = p + vec2(sin(iGlobalTime * j), cos(iGlobalTime * j));
        destColor += 0.05 * abs(cos(iGlobalTime)) / length(q);
    }
    
    for(float i = 0.0; i < 10.0; i++){
        float j = i + 6.54;
        vec2 q = p + vec2(-sin(iGlobalTime * j) * abs(sin(iGlobalTime*3.0) + 0.5), cos(iGlobalTime * j) * abs(sin(iGlobalTime*3.0) + 0.5));
        destColor += 0.02 * abs(tan(iGlobalTime/1.23)) / length(q);
    }
    
    float g = destColor.r * abs(sin(iGlobalTime*2.0));
    float b = destColor.r * abs(sin(iGlobalTime*5.0));
    float r = destColor.r * abs(cos(iGlobalTime*0.5));
    
    fragColor = vec4(r, g, b, 1.0);
}
