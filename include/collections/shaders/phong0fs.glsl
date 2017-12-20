R"(

#version 330 core

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
	vec4 FragPosCS;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;


void main(void)
{
	vec3 scolor = texture(specular_tex, fs_in.TexCoord).rgb;
	vec3 dcolor = texture(diffuse_tex, fs_in.TexCoord).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(1.0);
	// ambient
	vec3 ambient = 0.15 * dcolor;
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

//	float shadow = ShadowCalculation(fs_in.FragPosLS);
	vec3 lightling = ambient + (diffuse * dcolor) + (specular * scolor);
	FragColor = vec4(lightling, 1.0);
}

)"
