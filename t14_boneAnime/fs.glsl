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
       sampler2D diffuse0;  //setted
       sampler2D specular0; //setted
       float	 shininess; //setted
};

uniform vec3 viewPos;//setted

uniform sampler2D diffuse;

in vec3 Normal;
in vec2 TexCoords;
in vec3 fragPos;
out vec4 color;


//the fragment shader must follow this protocol, we could optimize it in the
//future by setting the name in Mesh class.


/* 
 * this set of functions can be really helpful to avoid branches
 *
vec4 when_eq(vec4 x, vec4 y) {
  return 1.0 - abs(sign(x - y));
  }

vec4 when_neq(vec4 x, vec4 y) {
  return abs(sign(x - y));
  }

vec4 when_gt(vec4 x, vec4 y) {
  return max(sign(x - y), 0.0);
  }

vec4 when_lt(vec4 x, vec4 y) {
  return max(sign(y - x), 0.0);
  }

vec4 when_ge(vec4 x, vec4 y) {
  return 1.0 - when_lt(x, y);
  }

vec4 when_le(vec4 x, vec4 y) {
  return 1.0 - when_gt(x, y);
  }
*/

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
//	vec3 viewDir = normalize(viewPos - fragPos);
//	vec3 highDir = normalize(viewDir + lightDir);
//	float specl   = pow(max(dot(norm, highDir), 0.0), 32.0);
//	vec3 spec = 0.5 * specl * vec3(texture(specular, TexCoords));

	//so the problem is light struct doesn't work...
//	color = vec4(ambient, 1.0f);
	color = vec4(ambient+diffu, 1.0f);
//	color = vec4(1.0f);
}

)"

