#version 330 core
out vec4 output;
in vec3 FragPos;
in vec3 normal;
in vec2 texCoords;
in mat3 TBN;

struct Material {
    sampler2D TEXTURE;
    vec3 specular;
    float shininess;
    sampler2D NORMAL;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 specular;
    vec3 diffuse;
};

uniform Material material;
uniform Light light;
uniform mat4 normalMatrix;
uniform vec3 viewLoc;
uniform int blinnPhong;
void main(){
    vec3 base = vec3(texture(material.TEXTURE, texCoords));
    vec3 normal_n = texture(material.NORMAL, texCoords).rgb;
    normal_n = (normal_n*2.0 - 1.0);
    normal_n = normalize(TBN*normal_n);
    // normal_n = normal_n * vec3(1.0f, -1.0f, 1.0f);
    vec3 ambient = light.ambient * base;

    // vec3 normal_n = mat3(normalMatrix) * normalize(normal);
    // vec3 normal_n = vec3(0.0f, 0.0f, 1.0f);
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal_n);
    vec3 viewDir = normalize(viewLoc - FragPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec;
    if (blinnPhong != 0)
        spec = pow(max(dot(normal_n, halfway), 0.0), material.shininess);
    else
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular =  spec * material.specular * light.specular;
    float diffuseStrength = max(dot(lightDir, normal_n), 0.0);
    vec3 diffuse = diffuseStrength * base * light.diffuse;

    vec3 Color =  ambient + diffuse + specular;
    // vec3 Color = bitangentVec;
    output = vec4(Color, 1.0f);   
}   