#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform vec3 objectColor;

float reflectRefractRatio = 0.8;
float lightingResistance = 0.6;
float opacity = 0.7;

void main()
{             
	//ambient
	vec3 ambient = light.ambient * material.ambient;

	//diffuse
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - Position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * material.diffuse);

	//specular
	vec3 viewDir = normalize(cameraPos - Position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * (spec * material.specular);

	vec3 result = ambient + diffuse + specular;


    // Glass
    //float ratio = 1.00 / 1.52;
    // Diamond
    //float ratio = 1.00 / 2.42;
    // Emerald
    float ratio = 1.00 / 1.58;

    vec3 I = normalize(Position - cameraPos);
    vec3 Rr = refract(I, normalize(Normal), ratio);
    vec3 Rl = reflect(I, normalize(Normal));


	// mix refraction and reflection 
	vec3 processResult = mix(
							texture(skybox, Rr).rgb, 
							texture(skybox, Rl).rgb, 
							reflectRefractRatio
						);

	// experimental: reduce the contrast seen in the emerald
	//processResult = processResult - (processResult - 0.5) * 0.1;
	//processResult = processResult + (processResult - 0.5) * 0.1; // increase contrast
	
	// apply color
	processResult = objectColor * processResult;

	// apply lighting
	processResult = mix(result, processResult, lightingResistance);

    FragColor = vec4(processResult, opacity);
}  
