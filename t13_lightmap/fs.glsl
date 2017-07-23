#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
	vec4 FragPosLS;
	vec4 FragPosCS;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D cubetex;
uniform sampler2D shadowmap;

//uniform mat4 lightMat;

float ShadowCalculation(vec4 fpls)
{
    // perform perspective divide
    vec3 projCoords = fpls.xyz / fpls.w;
    //all x,y,z are originally in [-1, 1] region
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float cd = texture(shadowmap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > cd  ? 1.0 : 0.0;
    shadow = cd;
    return shadow;
}  

void main(void)
{
	vec3 color = texture(cubetex, fs_in.TexCoord).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(1.0);
	// ambient
	vec3 ambient = 0.15 * color;
	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;
	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;
//	vec4 FragPosLS = lightMat * vec4(fs_in.FragPos, 1.0);
	float shadow = ShadowCalculation(fs_in.FragPosLS);
	vec3 lightling = ambient + (1.0 - shadow) * (diffuse + specular) * color;
//	FragColor = vec4(lightling, 1.0);
	FragColor = vec4(vec3(shadow), 1.0);
//	FragColor = vec4(vec3(texture(shadowmap, fs_in.TexCoord).r), 1.0);
//	FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}
