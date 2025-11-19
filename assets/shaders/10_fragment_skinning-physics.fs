#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 ex_N;
in vec3 vertexPosition_cameraspace;
in vec3 Normal_cameraspace;

uniform mat4 view;
uniform sampler2D texture_diffuse1;

// Propiedades del material
uniform vec4 MaterialAmbientColor;
uniform vec4 MaterialDiffuseColor;
uniform vec4 MaterialSpecularColor;
uniform float transparency;

// Sistema de múltiples luces (igual que 11_PhongShaderMultLights.fs)
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

// Función para aplicar una luz (igual que en 11_PhongShaderMultLights.fs)
vec4 ApplyLight(Light light, vec3 N, vec3 L, vec3 E) {
    
    // Componente ambiental (reducida para evitar oversaturation)
    vec4 K_a = MaterialAmbientColor * light.Color * 0.3; // ← Reducido
    
    // Componente difusa
    float cosTheta = clamp(dot(N, L), 0.0, 1.0);
    vec4 K_d = MaterialDiffuseColor * light.Color * cosTheta;
    
    // Componente especular
    vec3 R = reflect(-L, N);
    float cosAlpha = clamp(dot(E, R), 0.0, 1.0);
    vec4 K_s = MaterialSpecularColor * light.Color * pow(cosAlpha, float(light.alphaIndex));
    
    // Atenuación por distancia (importante para evitar exceso de luz)
    float distanceSq = max(light.distance * light.distance, 1.0); // Evitar división por 0
    
    vec4 l_contribution = 
        K_a * light.Power / distanceSq +
        K_d * light.Power / distanceSq +
        K_s * light.Power / distanceSq;
    
    return l_contribution;
}

void main()
{    
    // Normalizar la normal interpolada
    vec3 n = normalize(Normal_cameraspace);
    
    // Acumular contribución de todas las luces
    vec4 ex_color = vec4(0.0);
    
    for(int i = 0; i < numLights; ++i) {
        
        vec3 EyeDirection_cameraspace = vec3(0.0, 0.0, 0.0) - vertexPosition_cameraspace;
        vec3 LightPosition_cameraspace = (view * vec4(allLights[i].Position, 1.0)).xyz;
        vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
        
        vec3 e = normalize(EyeDirection_cameraspace);
        vec3 l = normalize(LightDirection_cameraspace);
        
        ex_color += ApplyLight(allLights[i], n, l, e);
    }
    
    // Limitar el color para evitar oversaturation (clamping)
    ex_color = clamp(ex_color, 0.0, 1.0);
    ex_color.a = transparency;
    
    // Aplicar textura
    vec4 texel = texture(texture_diffuse1, TexCoords);
    
    // Color final
    FragColor = texel * ex_color;
    
    // Asegurar que el alpha sea correcto
    FragColor.a = texel.a * transparency;
}