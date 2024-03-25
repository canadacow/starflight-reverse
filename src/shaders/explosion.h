
float noise3d(in vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    vec2 rg = texture(noiseTexture, (uv + 0.5)/256.0).xy;
    return mix(rg.y, rg.x, f.z);
}

#define FXD(uv) texture(noiseTexture, uv + seed).x
#define FXE(uv) 0.0
#define FXF(uv) 0.0
#define FXG(uv) noise3d(vec3(uv - seed, 0.0))

// Function to rotate a 2D vector by a given angle
vec2 rotate(vec2 v, float a) {
    float s = sin(a);
    float c = cos(a);
    mat2 m = mat2(c, -s, s, c);
    return m * v;
}

// Explosion effect function
void explosion(vec2 uv, inout vec3 col, float t, float size, float seed) {
    uv /= size;
    float x = t - 0.1;
    float iCore = exp(t * -20.0); //exp(x * x * -200.0);
    float szEx = smoothstep(0.2, 0.4, t) - smoothstep(0.4, 1.0, t) * 0.1;
    float szEx2 = smoothstep(0.2, 0.4, t);
    float flwEx = 1.0 - t * 0.5;
    float flwCore = 1.0 - t * 0.2;
    float iTotal = smoothstep(1.2, 0.5, t) * smoothstep(0.0, 0.01, t);
    float iPrt = smoothstep(0.2, 0.5, t) * smoothstep(1.0, 0.5, t);
    float szCorona = t;
    float flwPrt = 1.0 - t * 0.4;
    
    float iPrt2 = smoothstep(0.2, 0.5, t) * smoothstep(0.6, 0.5, t);
    float szCorona2 = t * 2.0;
    float flwPrt2 = 1.0 - t * 0.8;
    
    t += seed;
    
    uv += (fract(sin(vec2(t, t - 2.0) * 42.412) * 28972.0) - 0.5) * 0.1 * szEx * iTotal;
    
    float v = length(uv), sum = 0.0;
    sum = smoothstep(szEx * 0.6 + 0.1, 0.0,
   		v - szEx * 0.6
        - FXF(rotate(uv * flwEx, t * 0.2) * 0.04) * 0.9 * szEx
        - FXD(rotate(uv * flwEx, t * 0.4) * 0.2) * 0.2 * szEx                               
        );
    sum *= FXD(uv * flwEx * 0.02);
    
   	sum -= smoothstep(1.0, 0.2, v + FXE(uv * 0.1 * flwCore) * 0.5) * FXE((uv - vec2(0.1, t)) * 0.02) * 3.0 * FXD(uv * 0.7 * flwCore);
  	sum = max(sum, 0.0);

    sum += smoothstep(0.9, 0.0, v + 0.3 - iCore * 0.6);    
    sum *= iTotal;
    
    sum += pow(FXG(uv * 0.5 * flwPrt), 5.0) * iPrt * smoothstep(0.04, 0.0, v - szCorona - 0.7);
    sum += pow(FXG(-uv * 0.5 * flwPrt2 - 0.3333), 5.0) * iPrt2 * smoothstep(0.04, 0.0, v - szCorona2 - 0.7);

    // map intensity to color
    vec3 c = pow(vec3(sum), vec3(0.44, 0.6, 1.0) * 2.5) * 1.5;
    col += c;
}