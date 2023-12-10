const float cloudscale = 1.1;
const float clouddark = 0.5;
const float cloudlight = 0.3;
const float cloudcover = 0.5;
const float cloudalpha = 8.0;
const float skytint = 0.5;

const mat2 nebula_m = mat2( 1.6,  1.2, -1.2,  1.6 );

vec2 hash( vec2 p ) {
	p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float nebula_noise( in vec2 p ) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;
	vec2 i = floor(p + (p.x+p.y)*K1);	
    vec2 a = p - i + (i.x+i.y)*K2;
    vec2 o = (a.x>a.y) ? vec2(1.0,0.0) : vec2(0.0,1.0); //vec2 of = 0.5 + 0.5*vec2(sign(a.x-a.y), sign(a.y-a.x));
    vec2 b = a - o + K2;
	vec2 c = a - 1.0 + 2.0*K2;
    vec3 h = max(0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	vec3 n = h*h*h*h*vec3( dot(a,hash(i)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));
    return dot(n, vec3(70.0));	
}

float nebula_fbm(vec2 n) {
	float total = 0.0, amplitude = 0.1;
	for (int i = 0; i < 7; i++) {
		total += noise(n) * amplitude;
		n = nebula_m * n;
		amplitude *= 0.4;
	}
	return total;
}

vec2 nebula_processUV(vec2 srcUv, float scale, float q) {
    vec2 uv = srcUv * scale - q;
    return uv;
}

float nebula_processNoise(vec2 uv, float initialWeight, float weightDecay, int iterations, bool absValue) {
    float result = 0.0;
    float weight = initialWeight;
    for (int i = 0; i < iterations; i++) {
        result += (absValue ? abs(weight * nebula_noise(uv)) : weight * nebula_noise(uv));
        uv = nebula_m * uv;
        weight *= weightDecay;
    }
    return result;
}

void nebula(out vec4 fragColor, in vec2 uv, in float iTime) {
    float q = nebula_fbm(uv * cloudscale * 0.5);

    q += iTime / 250.0;

    float r = nebula_processNoise(nebula_processUV(uv, cloudscale, q), 0.8, 0.7, 8, true);
    float f = nebula_processNoise(nebula_processUV(uv, cloudscale, q), 0.7, 0.6, 8, false);
    f *= r + f;

    float c = nebula_processNoise(nebula_processUV(uv, cloudscale * 2.0, q), 0.4, 0.6, 7, false);
    float c1 = nebula_processNoise(nebula_processUV(uv, cloudscale * 3.0, q), 0.4, 0.6, 7, true);
    c += c1;

    vec3 cloudcolour = vec3(1.0, 0.1, 0.8) * clamp((clouddark + cloudlight * c), 0.0, 1.0);
    f = cloudcover + cloudalpha * f * r;

    float normalizedF = clamp(f, 0.0, 1.0);
    vec3 result = mix(cloudcolour, vec3(.0, .0, c), normalizedF);

    fragColor = vec4(result, 1.0);
}