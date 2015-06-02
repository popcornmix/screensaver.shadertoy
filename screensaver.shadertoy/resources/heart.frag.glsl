//http://glslsandbox.com/e#24426

vec3 heart( float x, float y )
{
    float s = (mod(iGlobalTime/1.35, 1.0 )+fract(iGlobalTime/1.35)) / 12.0;
    s = 0.9 + 0.1*(1.0-exp(-5.0*s)*sin(50.0*s));
    x *= s;
    y *= s;
    float a = atan(x,y)/3.14159265359;
    float r = sqrt(x*x*1.5+y*y);

    float h = abs(a);
    float d = (13.0*h - 22.0*h*h + 10.1*h*h*h)/(6.0-5.0*h);
    float g = pow(1.0-clamp(r/d,0.0,1.0),0.25);

    return vec3(0.5+0.5*g,0.2,0.4);
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (-1.0+2.0*fragCoord/iResolution.xy);
    vec3 col = heart(p.x, p.y-0.35 );
    fragColor = vec4(col,1.0);
}
