#version 330 core

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Phong {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};

out vec4 outFragColor;

uniform vec3 cameraPosition;
uniform vec3 color;

in vec3 normal;
in vec3 fragPosition;

vec3 DirectionalLightAddition(DirectionalLight light, Phong phong, vec3 normal, vec3 viewDir);

void main() {
    DirectionalLight dirLight;
    dirLight.direction = vec3(1.0, 1.0, 1.0);
    dirLight.direction = normalize(dirLight.direction);
    dirLight.ambient = vec3(0.4);
    dirLight.diffuse = vec3(0.8);
    dirLight.specular = vec3(0.1);

    Phong phong;
    phong.ambient = vec3(0.4) * color;
    phong.diffuse = vec3(0.8) * color;
    phong.specular = vec3(0.1) * color;
    phong.shininess = 32.0;

    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(cameraPosition - fragPosition);

    outFragColor = vec4(DirectionalLightAddition(dirLight, phong, norm, viewDir), 1.0);
}

vec3 DirectionalLightAddition(DirectionalLight light, Phong phong, vec3 normal, vec3 viewDir) {
    vec3 lightDirection = normalize(light.direction);

    float diff = max(dot(normal, lightDirection), 0.0);

    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDirection), 0.0), phong.shininess);

    vec3 ambient = phong.ambient;

    vec3 diffuse = light.diffuse * diff * phong.diffuse;
    vec3 specular = light.specular * spec * phong.specular;

    return ambient + diffuse + specular;
}
