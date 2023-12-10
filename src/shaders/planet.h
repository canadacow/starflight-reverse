/*--------------------------------------------------------------------------------------
License CC0 - http://creativecommons.org/publicdomain/zero/1.0/
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
----------------------------------------------------------------------------------------
^ This means do ANYTHING YOU WANT with this code. Because we are programmers, not lawyers.
-Otavio Good
*/

float PI=3.14159265;
vec3 sunCol = vec3(258.0, 208.0, 100.0) / 15.0;

float distFromSphere;
vec3 normal;
vec3 texBlurry;

vec3 saturate(vec3 a)
{
	return clamp(a, 0.0, 1.0);
}
vec2 saturate(vec2 a)
{
	return clamp(a, 0.0, 1.0);
}
float saturate(float a)
{
	return clamp(a, 0.0, 1.0);
}

vec3 GetSunColorReflection(vec3 rayDir, vec3 sunDir)
{
	vec3 localRay = normalize(rayDir);
	float sunIntensity = 1.0 - (dot(localRay, sunDir) * 0.5 + 0.5);
	//sunIntensity = (float)Math.Pow(sunIntensity, 14.0);
	sunIntensity = 0.2 / sunIntensity;
	sunIntensity = min(sunIntensity, 40000.0);
	return sunCol * sunIntensity;
}

float IntersectSphereAndRay(vec3 pos, float radius, vec3 posA, vec3 posB, out vec3 intersectA2, out vec3 intersectB2)
{
	vec3 eyeVec2 = normalize(posB-posA);
	float dp = dot(eyeVec2, pos - posA);
	vec3 pointOnLine = eyeVec2 * dp + posA;
	float distance = length(pointOnLine - pos);
	float ac = radius*radius - distance*distance;
	float rightLen = 0.0;
	if (ac >= 0.0) rightLen = sqrt(ac);
	intersectA2 = pointOnLine - eyeVec2 * rightLen;
	intersectB2 = pointOnLine + eyeVec2 * rightLen;
	distFromSphere = distance - radius;
	if (distance <= radius) return 1.0;
	return 0.0;
}

vec2 Spiral(vec2 uv)
{
	float reps = 2.0;
	vec2 uv2 = fract(uv*reps);
	vec2 center = floor(fract(uv*reps)) + 0.5;
	vec2 delta = uv2 - center;
	float dist = length(delta);
	float angle = atan(delta.y, delta.x);
	//if (distance(center, uv2) < 0.02) return vec2(10,10);
	float nudge = dist * 4.0;
	vec2 offset = vec2(delta.y, -delta.x);// * 0.2 / dist ;// vec2(sin(angle+nudge), cos(angle+nudge));
	float blend = max(abs(delta.x), abs(delta.y))* 2.0;
	blend = clamp((0.5 - dist) * 2.0, 0.0, 1.0);
	blend = pow(blend, 1.5);
	//offset *= clamp(1.0 - blend, 0.0, 1.0);
	offset *= clamp(blend, 0.0, 1.0);
	//if (dist > 0.5) offset = vec2(0,0);
	//offset *= dist;
	return uv + offset*vec2(1.0,1.0)*1.1*texBlurry.x ;
}

void draw_planet( out vec4 fragColor, in vec2 uv, in float iTime )
{
	// Camera setup
	vec3 camUp = vec3(0, 1, 0);
	vec3 camLookat = vec3(0, 0.0, 0);
	float mx = -PI / 2.0;
	float my = 0.0;
	vec3 camPos = vec3(cos(my) * cos(mx), sin(my), cos(my) * sin(mx)) * 2.5;
	vec3 camVec = normalize(camLookat - camPos);
	vec3 sideNorm = normalize(cross(camUp, camVec));
	vec3 upNorm = cross(camVec, sideNorm);
	vec3 worldFacing = camPos + camVec;
	vec3 worldPix = worldFacing + uv.x * sideNorm + uv.y * upNorm;
	vec3 relVec = normalize(worldPix - camPos);

	// Planet setup
	vec3 planetPos = vec3(0.0, 0.0, 0.0);
	vec3 iA, iB, iA2, iB2;
	float t = iTime * 0.1 + 0.7;
	float cloudT = iTime * 0.1;
	float distFromSphere2;
	vec3 normal2;
	float hit2 = IntersectSphereAndRay(planetPos, 1.18, camPos, worldPix, iA2, iB2);
	normal2 = normal;
	distFromSphere2 = distFromSphere;
	float hit = IntersectSphereAndRay(planetPos, 1.10, camPos, worldPix, iA, iB);
	normal = normalize(iA - planetPos);

	// Texture setup
	vec2 polar = vec2(atan(normal.x, normal.z), acos(normal.y));
	polar.x = (polar.x + PI) / (PI * 2.0);
	polar.y = polar.y / PI;
	polar.x = polar.x + 2.03;
	polar.xy = iA.xy;
	// vec4 texNoise = texture(iChannel2, (polar.xy + vec2(t, 0)) * 2.0);
	// texNoise.y = texture(iChannel2, (polar.xy + vec2(t, 0)) * 1.0).y;
	// texNoise.z = texture(iChannel2, (polar.xy + vec2(t, 0)) * 4.0).z;
	// texBlurry = texture(iChannel0, (polar.xy + vec2(t, 0)) * 0.03125 * 0.25).rgb;
    vec4 texNoise = vec4(0.0);
    texBlurry = vec3(0.0);
	vec3 tex = vec3(0.0);
    vec3 texS = vec3(0.0);
    vec3 texFlip = vec3(0.0);
    vec3 texFlipS = vec3(0.0);
	// tex *= tex;
	// vec3 texFlip = texture(iChannel0, (1.0 - (polar.xy + vec2(t, 0)) * 0.5)).rgb;
	// texFlip *= texFlip;
	// vec3 texS = texture(iChannel0, (Spiral(polar.xy + vec2(t, 0)) + vec2(cloudT * 0.25, 0)) * 1.0).rgb;
	// texS *= texS;
	// vec3 texFlipS = texture(iChannel0, (1.0 - (Spiral(polar.xy + vec2(t, 0)) + vec2(cloudT * 0.25, 0)) * 0.5)).rgb;
	// texFlipS *= texFlipS;

	// Atmosphere setup
	float atmosphereDensity = 1.45 + normal.z;
	vec3 atmosphereColor = vec3(0.075, 0.35, 0.99) * 0.45;
	float cloudDensity = max(0.0, pow(texFlipS.x * texS.x, 0.7) * 3.0);
	vec3 finalAtmosphere = atmosphereColor * atmosphereDensity;
	vec3 finalColor = finalAtmosphere;

	// Land setup
	// vec3 detailMap = min(texture(iChannel3, (polar.xy + vec2(t, 0)) * 2.0).xyz, 0.25) * 4.0;
    vec3 detailMap = vec3(0.0);
	// float land = pow(max(0.0, texture(iChannel1, (polar.xy + vec2(t, 0)) * 0.25).z - 0.25), 0.4) * 0.75;
    float land = 0.0;
	float land2 = 0.0;
	land *= detailMap.x;
	land2 = max(0.0, land2);
	land -= tex.x * 0.65;
	land = max(0.0, land);
	float iceFactor = abs(pow(normal.y, 2.0));
	vec3 landColor = max(vec3(0.0, 0.0, 0.0), vec3(0.13, 0.65, 0.01) * land);
	vec3 landColor2 = max(vec3(0.0, 0.0, 0.0), vec3(0.8, 0.4, 0.01) * land2);
	vec3 mixedLand = (landColor + landColor2) * 0.5;
	mixedLand *= (detailMap.zyx + 2.0) * 0.333;
	vec3 finalLand = mix(mixedLand, vec3(7.0, 7.0, 7.0) * land, iceFactor);
	finalLand = mix(atmosphereColor * 0.05, finalLand, pow(min(1.0, max(0.0, -distFromSphere * 1.0)), 0.2));
	finalColor += finalLand;
	finalColor *= hit;

	// Reflection setup
	float refNoise = (texNoise.x + texNoise.y + texNoise.z) * 0.3333;
	vec3 noiseNormal = normal;
	noiseNormal.x += refNoise * 0.05 * hit;
	noiseNormal.y += tex.x * hit * 0.1;
	noiseNormal.z += texFlip.x * hit * 0.1;
	noiseNormal = normalize(noiseNormal);
	vec3 ref = reflect(normalize(worldPix - camPos), noiseNormal);
	refNoise = refNoise * 0.25 + 0.75;
	float orbitSpeed = 0.125;
	vec3 sunDir = normalize(vec3(1.0, 0.0, 0.0));
	vec3 refNorm = normalize(ref);
	float glance = saturate(dot(refNorm, sunDir) * saturate(sunDir.z - 0.65));
	float landMask = finalLand.x + finalLand.y * 1.5;
	vec3 sunRef = GetSunColorReflection(refNorm, sunDir) * 0.005 * hit * (1.0 - saturate(landMask * 3.5)) * (1.0 - texS.x) * refNoise;
	sunRef = mix(sunRef, vec3(3.75, 0.8, 0.02) * hit, glance);
	finalColor += sunRef;

	// Final color setup
	vec3 sunsColor = vec3(0.);
	float outerGlow = 1.0 - clamp(distFromSphere * 20.0, 0.0, 1.0);
	outerGlow = pow(outerGlow, 5.2);
	finalColor += (atmosphereColor + vec3(0.2, 0.2, 0.2)) * outerGlow * (1.0 - hit);
	float light = saturate(dot(sunDir, noiseNormal));
	finalColor *= light * 0.75 + 0.001;
	finalColor += sunsColor;
	float scattering;
	if (hit2 == 1.0) scattering = distance(iA2, iB2);
	scattering *= pow(saturate(dot(relVec, sunDir) - 0.96), 2.0);
	scattering *= hit2 * (1.0 - hit);
	scattering *= outerGlow;
	finalColor += vec3(1.0, 0.25, 0.05) * scattering * 3060.0;
	fragColor = clamp(vec4(sqrt(finalColor), 1.0), 0.0, 1.0);
}
