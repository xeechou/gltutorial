#version 330 core
out vec4 color;

in vec3 FragPos;
in vec3 Normal; 
  
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

struct Material {
       float ambient;
       float diffuse;
       float specular;
       float shininess;
};

uniform Material material;


void main()
{
    // Ambient
    vec3 ambient = material.ambient * lightColor;

    //compute the real-light, if it points the back of the surface, set it to
    //zero
    //Diffuse
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    //if the light direction and normal direction is not same, we get negative
    //sign
    float s = sign(dot(norm, lightDir));
    
    if (dot(norm, lightDir) <= 0.0) {
       lightDir = vec3(0.0f);
    } 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * material.diffuse) * lightColor;
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    //if (length(reflectDir) < 0.0) {
    //  color = vec4(1.0f);
    //} else {
    //  color = vec4(0.0f);
    //}
    //color = vec4(vec3(length(dot(reflectDir, viewDir))), 0.0f);

    float spec = pow(dot(viewDir, reflectDir), material.shininess);
    vec3 specular = material.specular * spec * lightColor;
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    //vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, 1.0f);
}
