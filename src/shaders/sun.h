// based on https://www.shadertoy.com/view/lsf3RH by
// trisomie21 (THANKS!)
// My apologies for the ugly code.


// Needs to be included after the starburst as it uses its random noise generator

float star_noise(vec3 uv, float res)	// by trisomie21
{
	const vec3 s = vec3(1e0, 1e2, 1e4);
	
	uv *= res;
	
	vec3 uv0 = floor(mod(uv, res))*s;
	vec3 uv1 = floor(mod(uv+vec3(1.), res))*s;
	
	vec3 f = fract(uv); f = f*f*(3.0-2.0*f);
	
	vec4 v = vec4(uv0.x+uv0.y+uv0.z, uv1.x+uv0.y+uv0.z,
		      	  uv0.x+uv1.y+uv0.z, uv1.x+uv1.y+uv0.z);
	
	vec4 r = fract(sin(v*1e-3)*1e5);
	float r0 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	r = fract(sin((v + uv1.z - uv0.z)*1e-3)*1e5);
	float r1 = mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y);
	
	return mix(r0, r1, f.z)*2.-1.;
}

void main_star( out vec4 fragColor, in vec2 uv, in float iTime, in vec3 sunColor )
{
	float radius		= 0.24;
	float invRadius 	= 1.0/radius;
	
	vec3 orange			= sunColor;
	vec3 orangeRed		= clamp(vec3(sunColor.r * 1.25, sunColor.g * 0.5, sunColor.b * 0.5), 0.0, 1.0);
	
	float time		= iTime * 0.1;
	float aspect	= 1.0;
	vec2 p 			= -0.5 + uv;
	p.x *= aspect;

	float fade		= pow( length( 2.0 * p ), 0.5 );
	float fVal1		= 1.0 - fade;
	float fVal2		= 1.0 - fade;
	
	float angle		= atan( p.x, p.y )/6.2832;
	float dist		= length(p);
	vec3 coord		= vec3( angle, dist, time * 0.1 );
	
	float newTime1	= abs( star_noise( coord + vec3( 0.0, -time * 0.35, time * 0.015 ), 15.0 ) );
	float newTime2	= abs( star_noise( coord + vec3( 0.0, -time * 0.15, time * 0.015 ), 45.0 ) );	
	for( int i=1; i<=7; i++ ){
		float power = pow( 2.0, float(i + 1) );
		fVal1 += ( 0.5 / power ) * star_noise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 10.0 ) * ( newTime1 + 1.0 ) ) );
		fVal2 += ( 0.5 / power ) * star_noise( coord + vec3( 0.0, -time, time * 0.2 ), ( power * ( 25.0 ) * ( newTime2 + 1.0 ) ) );
	}
	
	float corona		= pow( fVal1 * max( 1.1 - fade, 0.0 ), 2.0 ) * 50.0;
	corona				+= pow( fVal2 * max( 1.1 - fade, 0.0 ), 2.0 ) * 50.0;
	corona				*= 1.2 - newTime1;
	vec3 sphereNormal 	= vec3( 0.0, 0.0, 1.0 );
	vec3 dir 			= vec3( 0.0 );
	vec3 center			= vec3( 0.5, 0.5, 1.0 );
	vec3 starSphere		= vec3( 0.0 );
	
	vec2 sp = -1.0 + 2.0 * uv;
	sp.x *= aspect;
	sp *= 2.0;
  	float r = dot(sp,sp);
	float f = (1.0-sqrt(abs(1.0-r)))/(r);
	if( dist < radius ){
		corona			*= pow( dist * invRadius, 24.0 );
  		vec2 newUv;
 		newUv.x = sp.x*f;
  		newUv.y = sp.y*f;
		newUv += vec2( time, 0.0 );
		
		vec3 texSample 	= vec3(starburst_noise( newUv ));
		float uOff		= ( texSample.g * 4.5 + time );
		vec2 starUV		= newUv + vec2( uOff, 0.0 );
		starSphere		= vec3(starburst_noise(starUV ) + 0.50);
	}
	
	float starGlow	= min( max( 1.0 - dist, 0.0 ), 1.0 );
	vec3 color = vec3( f * 0.75 * orange ) + starSphere + corona * orange + starGlow * orangeRed;
	color = clamp(color, 0.0, 1.0);
	fragColor.rgb = color;
	fragColor.a		= 1.0;
}

