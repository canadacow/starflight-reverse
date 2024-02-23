
float engine_mask(vec2 uv, float r)
{
    uv.x = mod(uv.x, 0.5) * 2.0; // Scale uv.x to [0, 2] range, where [1, 2] becomes the mirrored half
    if (uv.x > 1.0) {
        uv.x = 2.0 - uv.x; // Mirror the second half
    }
    
    vec2 p = vec2(0.5, 0.0);
    float dx = uv.x - p.x;
    float dy = uv.y - p.y;
    //dx *= .75;
    dy *= .75;
    dy += .1;
    
    return (r * r) / (dx * dx + dy * dy);
}

float engine_random(vec2 uv)
{
 	return fract(sin(dot(vec2(100., 213.), uv)) * 3141.);   
}

float value_noise(vec2 uv)
{
 	vec2 i = floor(uv);
    vec2 f = fract(uv);
    
    f = f * f * (3. - 2. * f);
    
    float b = mix(engine_random(i), engine_random(i + vec2(1., 0.)), f.x);
    float t = mix(engine_random(i + vec2(0., 1.)), engine_random(i + vec2(1.)), f.x);
    
    return mix(b,t,f.y);
}

float engine_noise(vec2 uv)
{
    float n = value_noise(uv);
    n += value_noise(uv * 2.) * 0.5;
    n += value_noise(uv * 4.) * 0.25;
    n += value_noise(uv * 8.) * 0.125;
    n += value_noise(uv * 16.) * 0.0625;
    
    return n / 1.9375;
}

void main_engines( out vec4 fragColor, in vec2 uv, float iTime, float radius )
{
    uv.xy = uv.yx;

    uv.y = 1.0 - uv.y;
    
    vec2 nuv1 = vec2(uv.x * 5., uv.y - iTime * 0.6);
    vec2 nuv2 = vec2(uv.x * 6., uv.y - iTime * 0.4);
    vec2 nuv3 = vec2(uv.x * 8., uv.y - iTime * 0.7);
    
    vec4 n1 = vec4(1., 0.,0.,1.) * engine_noise(nuv1);
    vec4 n2 = vec4(0., 0., 1., 1.) * engine_noise(nuv2);
    vec4 n3 = vec4(1.,1., 0.,1.) * engine_noise(nuv3);
    

    // Output to screen
    vec4 outColor = (n1 + n2 + n3 - vec4(.1)) * (engine_mask(uv, radius)- 0.5);

    outColor = max(outColor, vec4(0.0));

    float luminance = 0.299 * outColor.r + 0.587 * outColor.g + 0.114 * outColor.b;
    outColor.a = luminance * .75;

    fragColor = outColor;
}