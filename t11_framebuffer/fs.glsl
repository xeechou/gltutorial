R"(
#version 330 core

struct Light {
       vec3 position;//setted
       float ambient;//setted
       float diffuse;//setted
       float specular;//setted
};
uniform Light light; //static, setup in main.cc

struct Material {
       sampler2D diffuse0;//setted
       sampler2D specular0;//setted
       float	 shininess;//setted
};
//uniform Material material; //setup in the Mesh class
uniform vec3 viewPos;//setted
uniform vec3 lightColor; //setted


uniform sampler2D diffuse;
uniform sampler2D specular;

//the fragment shader must follow this protocol, we could optimize it in the
//future by setting the name in Mesh class.

in vec3 Normal;
in vec2 TexCoords;
in vec3 fragPos;
out vec4 color;

/* here we have a blin-phone model */
void main(void)
{
	vec3 norm = normalize(Normal);
	vec3 lightDir =  normalize(light.position-fragPos);
	//mute light on the back of object
	lightDir = max(sign(dot(lightDir, norm)), 0.0f) * lightDir;

	//ambient
//	vec3 ambient = vec3(light.ambient, 1.0, 1.0);
	vec3 ambient = 0.3 * vec3(texture(diffuse, TexCoords));

	//diffuse
	float diff   = dot(lightDir, norm);
	vec3 diffu = diff *
	     	       vec3(texture(diffuse, TexCoords));

	//specular
//	vec3 refDir  = reflect(-lightDir, norm);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 highDir = normalize(viewDir + lightDir);
	float specl   = pow(max(dot(norm, highDir), 0.0), 32.0);
	vec3 spec = 0.5 * specl * vec3(texture(specular, TexCoords));

	//so the problem is light struct doesn't work...
//	color = vec4(ambient, 1.0f);
	color = vec4(ambient+diffu+spec, 1.0f);
}

)"
