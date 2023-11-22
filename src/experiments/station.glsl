// This ShaderToy shader uses a depth map and an albedo map to simulate moving a light source around a scene.
vec4 iEllipsoid( in vec3 ro, in vec3 rd, in vec3 r )
{
    vec3 r2 = r*r;
    float a = dot( rd, rd/r2 );
	float b = dot( ro, rd/r2 );
	float c = dot( ro, ro/r2 );
	float h = b*b - a*(c-1.0);
	if( h<0.0 ) return vec4(-1.0);
	float t = (-b - sqrt( h ))/a;
    vec3 pos = ro + t*rd;
    if(pos.y < 0.0) return vec4(-1.0); // Check if the intersection point is in the upper half of the ellipsoid
    vec3 n = normalize( pos/r2 );
    return vec4(t,n);
}

mat3 rotationMatrix(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat3(
        1.0, 0.0, 0.0,
        0.0, c, -s,
        0.0, s, c
    );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord / iResolution.xy;

    // Get depth
    float depth = texture(iChannel1, uv).r;

    // Calculate surface position
    vec3 pos = vec3(uv, depth);

    // Calculate surface normal
    vec2 eps = vec2(0.01, 0.0);
    vec3 nor = normalize(vec3(
        texture(iChannel1, uv + eps).r - texture(iChannel1, uv - eps).r,
        2.0 * eps.x,
        texture(iChannel1, uv + eps.yx).r - texture(iChannel1, uv - eps.yx).r
    ));

    // Light source
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // Animate light source
    float time = iTime * 1.0;
    vec3 lightPos = vec3(iMouse.x / iResolution.x, iMouse.y / iResolution.y + 0.5 , -1.0);

    // Calculate lighting
    vec3 lightDir = normalize(lightPos - pos);
    float diff = max(dot(nor, lightDir), 0.0);

    // Get albedo color
    vec3 albedo = texture(iChannel0, uv).rgb;

    // Apply lighting to albedo color
    vec3 color = albedo * diff * lightColor;

    // Add specular lighting
    vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0) - pos);
    vec3 reflectDir = reflect(-lightDir, nor);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2.0);

    // Make grey surfaces shiny
    if (albedo.r > 0.5 && albedo.g > 0.5 && albedo.b > 0.5)
    {
        color += spec * lightColor;
    }
    
    {
        vec3 ro = vec3( 0.0, 0.17, 0.75 );
        vec3 ta = vec3( 0.0, -0.13, 0.0 );
        // camera matrix
        vec3 ww = normalize( ta - ro );
        vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
        vec3 vv = normalize( cross(uu,ww));

        vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;

	    // create view ray
        vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

        // radius
        vec3  ra = vec3(0.61,0.1,0.58);
    
        // raytrace
        vec4 tnor = iEllipsoid( ro, rd, ra );

        if( tnor.x>0.0 )
        {
            vec3 pos = ro + tnor.x*rd;
            vec3 nor = tnor.yzw;

            // material
            vec3 col = vec3(0.0, 0.0, 0.8);

            // lighting
            vec3 lig = normalize(lightPos - pos); // Use lightPos here
            vec3 hal = normalize(-rd+lig);
            float dif = clamp( dot(nor,lig), 0.0, 1.0 );
            float amb = clamp( 0.5 + 0.5*dot(nor,vec3(0.0,1.0,0.0)), 0.0, 1.0 );

            // Add specular lighting
            vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0) - pos);
            vec3 reflectDir = reflect(-lig, nor);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 50.0); // Increase shininess factor

            col *= vec3(0.2,0.3,0.4)*amb + vec3(1.0,0.9,0.7)*dif;
            col += vec3(1.0, 1.0, 1.0) * spec; // Add specular color (white for glass)

            color += col;
        }
    }

    fragColor = vec4(color, 1.0);
}