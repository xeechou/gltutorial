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

//uniform sampler2D diffuse;
//uniform sampler2D specular;

//the fragment shader must follow this protocol, we could optimize it in the
//future by setting the name in Mesh class.

in vec3 Normal;
in vec2 TexCoords;
in vec3 fragPos;
out vec4 color;

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
uniform samplerCube skybox;

void main(void)
{
	vec3 I = normalize(fragPos - viewPos);
	vec3 R = reflect(I, normalize(Normal));
	color = vec4(texture(skybox, R).rgb, 1.0);
}
