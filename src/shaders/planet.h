const float PI = 3.14159265359;

const float SPECULAR_INTENSITY = 1.0;
const float SPECULAR_POWER = 10.0;

vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

mat2 rot(float a) {
	float s = sin(a), c = cos(a);
    return mat2(c, s, -s, c);
}

float kset(int it, vec3 p, vec3 q, float sc, float c, float iTime) {
    p.xz *= rot(iTime * .2);
    p += q;
    p *= sc;
    float l = 0., l2;
    for (int i = 0; i < it; i++) {
    	p = abs(p) / dot(p, p) - c;
		l += exp(-1. * abs(length(p) - l2));
	    l2 = length(p);
    }
    return l * .08;    
}

float clouds(vec3 p2, vec3 dir, float iTime) {
		p2 -= .1 * dir;
    	p2.y *= 3.;
    	float cl1 = 0., cl2 = 0.;
        for (int i = 0; i < 15; i++) {
			p2 -= .05 * dir;
            cl1 += kset(20, p2, vec3(1.7, 3., .54), .3, .95, iTime);
            cl2 += kset(18, p2, vec3(1.2, 1.7, 1.4), .2, .85, iTime);
        }    
        cl1 = pow(cl1 * .045, 10.);
        cl2 = pow(cl2 * .055, 15.);
		return cl1 - cl2;
}

vec4 textureBicubicArray(sampler2DArray tex, vec2 texCoords, uint layer){

   vec2 texSize = vec2(textureSize(tex, 0));
   vec2 invTexSize = 1.0 / texSize;
   
   texCoords = texCoords * texSize - 0.5;
   
    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;
    
    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;
    
    offset *= invTexSize.xxyy;
    
    vec4 sample0 = texture(tex, vec3(offset.xz, layer));
    vec4 sample1 = texture(tex, vec3(offset.yz, layer));
    vec4 sample2 = texture(tex, vec3(offset.xw, layer));
    vec4 sample3 = texture(tex, vec3(offset.yw, layer));

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}

vec3 getCameraRayDir(vec2 uv, vec3 camPos, vec3 camTarget)
{
    // Calculate camera's "orthonormal basis", i.e. its transform matrix components
    vec3 camForward = normalize(camTarget - camPos);
    vec3 camRight = normalize(cross(vec3(0.0, 1.0, 0.0), camForward));
    vec3 camUp = normalize(cross(camForward, camRight));
     
    float fPersp = 2.0;
    vec3 vDir = normalize(uv.x * camRight + uv.y * camUp + camForward * fPersp);
 
    return vDir;
}


void draw_planet(out vec4 fragColor, in vec2 uv, in float iTime, in vec3 sunDir, in uint planetIndex)
{

    // Side perspective
    vec3 camPos = vec3(0, 0, 1);
    vec3 camTarget = vec3(0, 0, 0);
    vec3 rayDir = getCameraRayDir(uv, camPos, camTarget);

    vec3 sphereCenter = vec3(0, 0, 0); // Sphere (planet) center
    float sphereRadius = 0.24; // Sphere (planet) radius

    vec3 oc = camPos - sphereCenter;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4.0 * a * c;

    vec3 color;
    vec3 point;
    vec3 rayNormal;
    float fresnelFactor;
    float diffuse = 0.0;

    vec3 viewDir = normalize(camPos - point);
    float fresnelExponent = 2.0; // Adjust this to change the strength of the Fresnel effect

    if (discriminant < 0.0) {
        color = vec3(0.); // Background color
        point = camPos * rayDir;
        rayNormal = normalize(point - sphereCenter);
    }
    else {
        // Calculate the intersection point
        float t = (-b - sqrt(discriminant)) / (2.0 * a);
        point = camPos + t * rayDir;

        // Calculate the normal at the intersection point
        rayNormal = normalize(point - sphereCenter);

        vec3 normal = rayNormal;

        // Rotate the normal
        float rotationSpeed = 0.3; // Adjust this to change the rotation speed
        float angle = iTime * rotationSpeed;
        mat2 rotationMatrix = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
        normal.xz = rotationMatrix * normal.xz;

        // Calculate the latitude and longitude for the Mercator projection
        float latitude = asin(normal.y) / PI + 0.5;
        float longitude = atan(normal.z, normal.x) / (2.0 * PI) + 0.5;

        // Generate noise based on latitude and longitude
        float noise = noise(vec2(latitude * 256.0, longitude * 256.0));

        // Sample the texture
        vec3 albedo = textureBicubicArray(PlanetTextures, vec2(longitude, latitude), planetIndex).rgb;
        //vec3 albedo = texture(PlanetTextures, vec3(longitude,latitude, planetIndex)).rgb;

        // Adjust the color based on the noise
        albedo = mix(albedo, albedo * noise, 0.1);

        // Calculate the diffuse color
        diffuse = max(dot(rayNormal, sunDir), 0.0);

        color = albedo * diffuse;

        // Generate clouds
        float cloudIntensity = clamp(clouds(point, rayDir, iTime) - 0.8, 0.0, 1.0);
        vec3 cloudColor = vec3(1.3) * cloudIntensity;

        // Add clouds to the color
        //color += cloudColor * diffuse;

        fresnelFactor = pow(1.0 - max(dot(viewDir, rayNormal), 0.0), fresnelExponent);
        color += fresnelFactor * color;

        // Perturb the normal for the specular calculation
        vec3 perturbedNormal = normalize(rayNormal + vec3(noise / 10.0));

        // Add specular lighting for blue parts
        if (albedo.b > albedo.r && albedo.b > albedo.g) { // If blue is the dominant color
            vec3 reflectDir = reflect(-sunDir, perturbedNormal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), SPECULAR_POWER);
            color += vec3(1.0, 1.0, 1.0) * spec * SPECULAR_INTENSITY;
        }
        else { // For land
            vec3 reflectDir = reflect(-sunDir, perturbedNormal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), SPECULAR_POWER);
            color += vec3(1.0, 1.0, 1.0) * spec * (SPECULAR_INTENSITY * 0.25); // 25% of the specular intensity for water
        }
    }

    fresnelFactor = pow(1.0 - max(dot(viewDir, rayNormal), 0.0), fresnelExponent);

    // Add blue-tint atmosphere
    float atmosphereIntensity = 0.5; // Adjust this to change the intensity of the atmosphere
    float atmosphereFade = smoothstep(-0.75, 1.0, discriminant);
    vec3 atmosphereColor = vec3(0.1, 0.2, 0.6) * atmosphereIntensity * atmosphereFade; // Adjust the color to a more realistic sky blue
    color += atmosphereColor * (diffuse + 1.0 * fresnelFactor);

    fragColor = vec4(color, 1.0);
}

