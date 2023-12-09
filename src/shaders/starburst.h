// Pseudo-random number generator
vec2 starburst_random(vec2 st) {
    st = vec2( dot(st,vec2(127.1,311.7)), dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Perlin noise function
float starburst_noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    vec2 u = f*f*(3.0-2.0*f);
    return mix( mix( dot(starburst_random(i + vec2(0.0,0.0)), f - vec2(0.0,0.0)), 
                     dot(starburst_random(i + vec2(1.0,0.0)), f - vec2(1.0,0.0)), u.x),
                mix( dot(starburst_random(i + vec2(0.0,1.0)), f - vec2(0.0,1.0)), 
                     dot(starburst_random(i + vec2(1.0,1.0)), f - vec2(1.0,1.0)), u.x), u.y);
}

void starburst( out vec4 fragColor, in vec2 fragCoord, in float iTime, in float sizeAdjust )
{
    fragCoord -= 0.5;

    // Add time-dependent, noise-based warp to the uv coordinates
    vec2 warp = 0.001 * vec2(starburst_noise(fragCoord + vec2(iTime)), starburst_noise(fragCoord + vec2(0.4, iTime)));
    vec2 uv = (fragCoord + (warp * 10.));
   
    float angle = atan(uv.y, uv.x) + 0.785398; // Add 45 degrees in radians
    float radius = length(uv) * sizeAdjust;
    
    // Create star shape function with 4 rays
    float starShape = smoothstep(0.0, 0.1, 0.1 - abs(mod(angle, 1.57) - 0.785));

    // Add optional smaller set of diagonal rays
    float diagonalStarShape = smoothstep(0.0, 0.05, 0.05 - abs(mod(angle + 0.785, 1.57) - 0.785)); // Add 45 degrees offset

    // Make diagonal rays shorter
    diagonalStarShape *= smoothstep(0.2, 0.01, radius);

    // Combine the two star shapes
    starShape = max(starShape, diagonalStarShape);

    // Modulate brightness with Gaussian
    float brightness = exp(-15.0 * radius * radius);
    
    // Create color using star shape and brightness
    vec3 color = mix(vec3(0.1, 0.2, 0.7), vec3(1.0), starShape) * brightness;
    
    // Add core star from the new shader
    vec2 uvCore = 2. * uv;
    float t = .001 + sin(iTime * 2.) * .0002;
    float d = t / abs(uvCore.x * uvCore.y) * (1. - length(uvCore));
    color += vec3(d) * vec3(0.8, 0.8, 1.0); // Multiply by a color vector to make the core star blueish on the ends
    
    // Add randomness to the blueish ring
    float rotationSpeed = 0.1; // Adjust this to change the speed of rotation
    float rotationAngle = iTime * rotationSpeed;
    mat2 rotationMatrix = mat2(cos(rotationAngle), -sin(rotationAngle), sin(rotationAngle), cos(rotationAngle));
    float ring = smoothstep(0.2, 0.4, radius + 0.1 * noise(rotationMatrix * uv * 10.0));
    color = mix(color, vec3(0.05, 0.05, 0.35) * noise(rotationMatrix * uv * 3.0), ring);
   
    // Fade rays to black
    color *= 1.0 - smoothstep(0.4, 0.6, radius);
    
    fragColor = vec4(color, 1.0);
}