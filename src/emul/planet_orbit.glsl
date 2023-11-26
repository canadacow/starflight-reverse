#define PI 3.1415926535897932384626433832795

float intensity = 3.0; 

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

float getHeight(vec2 uv) {
  return texture(iChannel0, uv).r;
}

vec4 bumpFromDepth(vec2 uv, vec2 resolution, float scale) {
  vec2 step = 1. / resolution;
    
  float height = getHeight(uv);
    
  vec2 dxy = height - vec2(
      getHeight(uv + vec2(step.x, 0.)), 
      getHeight(uv + vec2(0., step.y))
  );
    
  return vec4(normalize(vec3(dxy * scale / step, 1.)), height);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 mouse = iMouse.xy / iResolution.xy - 0.5;

    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / min(iResolution.y, iResolution.x);
    vec3 camPos = vec3(0.0, -8.0, 0.0);
    float fov = PI / 2.0; // 45 degrees
    vec3 rayDir = normalize(vec3(uv * tan(fov / 2.0), -1.0));
    
    rayDir *= rotateX(PI / 2.0);

    vec3 sphereCenter = vec3(4.0 * cos(iTime), 0.0, 4.0 * sin(iTime)); // Simulate orbit
    float sphereRadius = 1.0;

    vec3 oc = camPos - sphereCenter;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - sphereRadius*sphereRadius;
    float discriminant = b*b - 4.0*a*c;

    if(discriminant < 0.0)
    {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        float t = (-b - sqrt(discriminant)) / (2.0*a);
        vec3 point = camPos + t*rayDir;
        vec3 normal = normalize(point - sphereCenter);
        vec3 lightDir = normalize(vec3(0.0, 0.0, 0.0) - point); // Light at the center
        float diff = max(0.0, dot(normal, lightDir));
        
        diff *= intensity;
        
        // Calculate texture coordinates
        float u = 0.5 + atan(normal.z, normal.x) / (2.0 * PI);
        float v = 0.5 - asin(normal.y) / PI;
        
        v *= 0.5;
        v -= 0.5;
        
        // Sample the texture
        vec3 textureColor = texture(iChannel0, vec2(u, v)).rgb;

        // Compute normals from the texture
        vec3 bumpNormal = bumpFromDepth(vec2(u, v), vec2(48.0, 24.0), 0.1).xyz; // Adjust the second parameter to accentuate the gradient

        // Use the blended diffuse lighting to set fragColor
        fragColor = vec4(diff * vec3(1.0), 1.0);
    }
}