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

    //vec2 uv = (fragCoord - 0.5 * iResolution.xy) / min(iResolution.y, iResolution.x);
    vec2 uv = (fragCoord-.5*iResolution.xy)/iResolution.y;
    vec3 camPos = vec3(0.0, -30.0, 0.0);
    float fov = PI / 4.00; // 45 degrees
    vec3 rayDir = normalize(vec3(uv * tan(fov / 2.0), -1.0));
    
    rayDir *= rotateX(PI / 2.0);

    vec3 sphereCenter = vec3(7.0 * cos(iTime), 0.0, 7.0 * sin(iTime)); // Simulate orbit
    float sphereRadius = 2.0;

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
        
        vec3 colorMap = texture(iChannel1, vec2(u, v)).rgb;
        
        v *= 0.5;
        v -= 0.5;
        
        // Sample the texture
        vec3 heightMap = texture(iChannel0, vec2(u, v)).rgb;
        
        vec3 albedoMap = texture(iChannel0, vec2(u, v + 0.5)).rgb;

        // Compute normals from the texture
        vec4 bumpNormal = bumpFromDepth(vec2(u, v), vec2(48.0, 24.0), 0.1); // Adjust the second parameter to accentuate the gradient

        // Define light properties
        vec3 lightColor = vec3(1.0, 1.0, 1.0); // white light
        float ambientStrength = 0.1;
        float specularStrength = 0.5;
        float shininess = 32.0;

        // Calculate ambient light
        vec3 ambient = ambientStrength * lightColor;

        // Calculate diffuse light for bump map
        float diffBump = max(dot(bumpNormal.xyz, lightDir), 0.0);

        // Combine diffuse light of the sphere surface and the bump map
        vec3 diffuse = diff /* * diffBump  */ * lightColor;

        // Calculate specular light
        vec3 viewDir = normalize(-rayDir);
        vec3 reflectDir = reflect(-lightDir, bumpNormal.xyz);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = specularStrength * spec * lightColor;

        // Combine results
        vec3 result = (ambient + diffuse + specular) * albedoMap;

        // Use the blended diffuse lighting to set fragColor
        fragColor = vec4(result, 1.0);
    }
}