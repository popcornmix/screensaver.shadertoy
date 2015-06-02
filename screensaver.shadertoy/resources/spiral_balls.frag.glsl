// See: http://glslsandbox.com/e#24875.0

#define TWO_PI 6.283185
#define NUMBALLS 50.0
float d = -TWO_PI/36.0;
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 p = (2.0*fragCoord - iResolution.xy)/min(iResolution.x, iResolution.y);
	p *= mat2(cos(iGlobalTime), -sin(iGlobalTime), sin(iGlobalTime), cos(iGlobalTime));
	
	vec3 c = vec3(0); //ftfy
	for(float i = 0.0; i < NUMBALLS; i++) {
		float t = TWO_PI * i/NUMBALLS + iGlobalTime;
		float x = cos(t);
		float y = sin(2.0 * t + d);
		vec2 q = 0.8*vec2(x, y);
		c += 0.01/distance(p, q) * vec3(0.5 * abs(x), 0, abs(y));
	}
	fragColor = vec4(c, 2.0);
}
