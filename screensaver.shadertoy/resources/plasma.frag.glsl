float     u_time=iGlobalTime*0.2;
vec2      u_k = vec2(32.0, 32.0);
precision mediump float;
const float PI=3.1415926535897932384626433832795;
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
   vec2 p = fragCoord.xy / iResolution.xy;
   float v = 0.0;
   vec2 c = p * u_k - u_k/2.0;
   v += sin((c.x+u_time));
   v += sin((c.y+u_time)/2.0);
   v += sin((c.x+c.y+u_time)/2.0);
   c += u_k/2.0 * vec2(sin(u_time/3.0), cos(u_time/2.0));
   v += sin(sqrt(c.x*c.x+c.y*c.y+1.0)+u_time);
   v = v/2.0;
   vec3 col = vec3(1.0, sin(PI*v), cos(PI*v));
   fragColor = vec4(col*0.5 + 0.5, 1.0);
}

