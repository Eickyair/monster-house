#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 ex_N; 

in vec3 vertexPosition_cameraspace;
in vec3 Normal_cameraspace;

uniform mat4 view;

uniform sampler2D texture_diffuse1;

uniform vec4 MaterialAmbientColor;
uniform vec4 MaterialDiffuseColor;
uniform vec4 MaterialSpecularColor;
uniform float transparency;

#define MAX_LIGHTS 10
uniform int numLights;

uniform struct Light {
   vec3  Position;
   vec3  Direction;
   vec4  Color;
   vec4  Power;
   int   alphaIndex;
   float distance;
} allLights[MAX_LIGHTS];

vec4 ApplyLight(Light light, vec3 N, vec3 L, vec3 E) {
    
    // Componente ambiental
    vec4 K_a = MaterialAmbientColor * light.Color;

    // Componente difusa
    float cosTheta = clamp(dot(N, L), 0.0, 1.0);
    vec4 K_d = MaterialDiffuseColor * light.Color * cosTheta;

    // Componente especular
    vec3 R = reflect(-L, N);
    float cosAlpha = clamp(dot(E, R), 0.0, 1.0);
    vec4 K_s = MaterialSpecularColor * light.Color * pow(cosAlpha, light.alphaIndex);

    // Atenuación (1 / d^2)
    float att = 1.0 / (light.distance * light.distance);
    vec4 l_contribution = (K_a + K_d + K_s) * light.Power * att;

    return l_contribution;
}

void main()
{    
    vec3 n = normalize(Normal_cameraspace);
    vec4 ex_color = vec4(0.0);

    for(int i = 0; i < numLights; ++i) {
        vec3 EyeDirection_cameraspace = vec3(0.0) - vertexPosition_cameraspace;
        vec3 LightPosition_cameraspace = (view * vec4(allLights[i].Position, 1.0)).xyz;
        vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

        vec3 e = normalize(EyeDirection_cameraspace);
        vec3 l = normalize(LightDirection_cameraspace);

        ex_color += ApplyLight(allLights[i], n, l, e);
    }
           
    ex_color.a = transparency;

    vec4 texel = texture(texture_diffuse1, TexCoords);

    FragColor = texel * ex_color;
}
