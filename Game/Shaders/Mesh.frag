// Fragment shader

#version 430 

// Struct Definitions

struct Material {
    sampler2D texture;
	bool hasSpecMap;
	sampler2D specMap;

	bool hasNormMap;
	sampler2D normMap;

    float shininess;
}; 

struct SpotLight {
	vec3 position;
	vec3  direction;
    float cutOff;
	float outerCutOff;

	float constant;
    float linear;
    float quadratic;

	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
    float linear;
    float quadratic;

	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Variables 

in vec3 FragPosition;
in vec3 Normal;
in vec2 TexCoords;
in vec3 ModelView;
in mat3 TBN;

uniform bool spotLightOn;
uniform SpotLight spotlight;

#define MaxNumLights 15
uniform PointLight pointlights[MaxNumLights];

uniform int numLights;

uniform vec3 viewPos;
uniform Material material;

uniform bool enableFog = true;
uniform vec4 fog_color = vec4(0.7, 0.8, 0.9, 0.0); // 0.1 0.2 0.3 works pretty good too O.o

out vec4 color;

uniform samplerCube depthMaps[MaxNumLights];
uniform bool shadows;
uniform float far_plane;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);


// Function Declarations

vec3 CalcSpotLight(SpotLight light, vec3 textureColor, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// Spotlight Intensity (soft edges)
	float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// Attenuation
	float distance    = length(light.position - FragPosition);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	vec3 ambient = light.ambient; // * textureColor;
    vec3 diffuse = light.diffuse * diff;// * textureColor;
	vec3 specular;
	if(material.hasSpecMap){
		specular = light.specular * spec * vec3(texture(material.specMap, TexCoords));// * textureColor;
	} else {
		specular = light.specular * spec;// * textureColor;
	}

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular) * textureColor;
}

vec3 CalcPointLight(PointLight light, vec3 textureColor, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow){
	vec3 lightDir = normalize(light.position - fragPos);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient;//  * textureColor;
    vec3 diffuse  = light.diffuse  * diff;// * textureColor;
	vec3 specular;
	if(material.hasSpecMap){
		specular = light.specular * spec * vec3(texture(material.specMap, TexCoords));// * textureColor;
	} else {
		specular = light.specular * spec;// * textureColor;
	}

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * textureColor;
}

float ShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube depthMap)
{
    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);
    // Test for shadows with PCF
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
	float closestDepth;
    for(int i = 0; i < samples; ++i)
    {
        closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // Undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // Display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    // return shadow;
    return shadow;
}

vec4 fog(vec4 c){
	float z = length(ModelView);
	float de = 0.025 * smoothstep(0.0, 10.0, 10.0 - FragPosition.y);
	float di = 0.045 * smoothstep(0.0, 100.0, 20.0 - FragPosition.y);

	float extinction = exp(-z * de);
	float inscattering = exp(-z * di);

	return c * extinction + fog_color * (1.0 - inscattering);
}

void main()
{   
	vec3 norm;
	if(material.hasNormMap){
		norm = vec3(texture(material.normMap, TexCoords));
		norm = normalize(norm * 2.0 - 1.0);  
		norm = normalize(TBN * norm);
	} else {
		norm = normalize(Normal);
	}
    vec3 viewDir = normalize(viewPos - FragPosition);
	vec3 textureColor = vec3(texture(material.texture, TexCoords));
	vec3 result = vec3(0.0);

	for(int i = 0; i < numLights; i++){
		float shadow = shadows ? ShadowCalculation(FragPosition, pointlights[i].position, depthMaps[i]) : 0.0;
		result += CalcPointLight(pointlights[i], textureColor, norm, FragPosition, viewDir, shadow);
	}

	if(spotLightOn){
		result += CalcSpotLight(spotlight, textureColor, norm, FragPosition, viewDir);
	}

	if(enableFog){
		color = fog(vec4(result, 1.0f));
	} else {
		color = vec4(result, 1.0f);
	}
}
