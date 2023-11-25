void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / min(iResolution.y, iResolution.x);
    vec3 camPos = vec3(0.0, 0.0, -1.0);
    vec3 rayDir = normalize(vec3(uv, 1.0));

    vec3 sphereCenter = vec3(0.5 * cos(iTime), 0.0, 0.5 * sin(iTime)); // Simulate orbit
    float sphereRadius = 0.25;

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
        fragColor = vec4(diff * vec3(1.0, 0.0, 0.0), 1.0);
    }
}