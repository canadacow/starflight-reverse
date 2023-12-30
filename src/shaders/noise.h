// random/hash function              
float hash( float n )
{
  return fract(cos(n)*41415.92653);
}

// 2d noise function
float noise( in vec2 x )
{
  vec2 p  = floor(x);
  vec2 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0;

  return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
}

float noise( in vec3 x )
{
  vec3 p  = floor(x);
  vec3 f  = smoothstep(0.0, 1.0, fract(x));
  float n = p.x + p.y*57.0 + 113.0*p.z;

  return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}

mat3 m = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );

// Fractional Brownian motion
float fbmslow( vec3 p )
{
  float f = 0.5000*noise( p ); p = m*p*1.2;
  f += 0.2500*noise( p ); p = m*p*1.3;
  f += 0.1666*noise( p ); p = m*p*1.4;
  f += 0.0834*noise( p ); p = m*p*1.84;
  return f;
}

float fbm( vec3 p )
{
  float f = 0., a = 1., s=0.;
  f += a*noise( p ); p = m*p*1.149; s += a; a *= .75;
  f += a*noise( p ); p = m*p*1.41; s += a; a *= .75;
  f += a*noise( p ); p = m*p*1.51; s += a; a *= .65;
  f += a*noise( p ); p = m*p*1.21; s += a; a *= .35;
  f += a*noise( p ); p = m*p*1.41; s += a; a *= .75;
  f += a*noise( p ); 
  return f/s;
}