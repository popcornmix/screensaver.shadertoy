void mainImage( out vec4 f, in vec2 w ) 
{
	vec2 r = iResolution.xy, p = (w-r*.5) / r.x;
    
	float d = length(p), c = 4.-d*9., k=1.;
    
	p = vec2(atan(p.x,p.y), d-iDate.w/4.)*.02;       
    
    for(int i=1; i<7; i++)
        c += texture2D(iChannel0, p).r / k, p += p, k += k;
    
	f = vec4(.3,.15,.1,1)*c*c;
}
