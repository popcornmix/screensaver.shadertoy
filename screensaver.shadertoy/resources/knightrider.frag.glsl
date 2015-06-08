// See: http://glslsandbox.com/e#24875.0

#define TWO_PI 3.283185
#define NUMBALLS 30.0
float d = -TWO_PI/36.0;
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 p = (2.*fragCoord - iResolution.xy)/min(iResolution.x, iResolution.y);
	p *= mat2(cos(iGlobalTime), -sin(iGlobalTime), sin(iGlobalTime), cos(iGlobalTime));
	
	float p1=iMouse.y*10.0;
	
	vec3 c = vec3(0); //ftfy
	for(float i = 1.0; i < NUMBALLS; i++) {
		float t = TWO_PI * i/NUMBALLS + iGlobalTime;
		float x = cos(t);
		float y = sin(.0 * t + d);
		vec2 q = 0.8*vec2(x, y);
		c += 0.009/distance(p, q) * vec3(1.0 * abs(x), -7, abs(y));
	}
	fragColor = vec4(c, 7.0);
}
