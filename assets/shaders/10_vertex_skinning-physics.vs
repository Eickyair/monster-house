#version 330 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  tangent;
layout (location = 4) in vec3  bitangent;
layout (location = 5) in vec4  bIDs1;
layout (location = 6) in vec4  bIDs2;
layout (location = 7) in vec4  bIDs3;
layout (location = 8) in vec4  bWeights1;
layout (location = 9) in vec4  bWeights2;
layout (location = 10) in vec4 bWeights3;

out vec2 TexCoords;
out vec3 ex_N;
out vec3 vertexPosition_cameraspace;
out vec3 Normal_cameraspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 gBones[100];

// ========== UNIFORMS DE FÍSICA ==========
uniform float physicsTime;
uniform bool isJumping;
uniform float initialVelocity;
uniform float lunarGravity;
uniform float astronautMass;
uniform float groundLevel;

void main()
{
    // 1. Aplicar transformación de huesos (animación esquelética)
    mat4 BoneTransform = gBones[int(bIDs1[0])] * bWeights1[0];
    BoneTransform += gBones[int(bIDs1[1])] * bWeights1[1];
    BoneTransform += gBones[int(bIDs1[2])] * bWeights1[2];  
    BoneTransform += gBones[int(bIDs1[3])] * bWeights1[3];

    BoneTransform += gBones[int(bIDs2[0])] * bWeights2[0];
    BoneTransform += gBones[int(bIDs2[1])] * bWeights2[1];
    BoneTransform += gBones[int(bIDs2[2])] * bWeights2[2]; 
    BoneTransform += gBones[int(bIDs2[3])] * bWeights2[3];

    BoneTransform += gBones[int(bIDs3[0])] * bWeights3[0];
    BoneTransform += gBones[int(bIDs3[1])] * bWeights3[1];
    BoneTransform += gBones[int(bIDs3[2])] * bWeights3[2]; 
    BoneTransform += gBones[int(bIDs3[3])] * bWeights3[3];

    // 2. Aplicar transformación de huesos al vértice
    vec4 PosL = BoneTransform * vec4(aPos, 1.0f);
    
    // 3. Transformar al espacio del mundo
    // IMPORTANTE: La matriz model YA incluye initialTranslation (se aplica en CPU)
    vec4 worldPosition = model * PosL;
    
    // 4. ========== APLICAR FÍSICA DEL SALTO EN ESPACIO WORLD ==========
    if (isJumping) {
        // Ecuaciones cinemáticas: y = v0*t - (1/2)*g*t²
        float t = physicsTime;
        float v0 = initialVelocity;
        float g = lunarGravity;
        
        float verticalDisplacement = (v0 * t) - (0.5 * g * t * t);
        
        // No atravesar el suelo
        verticalDisplacement = max(verticalDisplacement, 0.0);
        
        // Aplicar desplazamiento vertical en ESPACIO WORLD (coordenada Y global)
        worldPosition.y += verticalDisplacement;
    }
    
    // 5. Asegurar nivel del suelo
    worldPosition.y = max(worldPosition.y, groundLevel);
    
    // 6. Transformar a espacio de cámara y clip space
    vec4 viewPosition = view * worldPosition;
    gl_Position = projection * viewPosition;

    // 7. Outputs para iluminación correcta
    TexCoords = aTexCoords;
    
    // Posición en espacio de cámara para iluminación
    vertexPosition_cameraspace = viewPosition.xyz;
    
    // Transformar normal con la matriz de huesos
    vec3 transformedNormal = mat3(BoneTransform) * aNormal;
    Normal_cameraspace = mat3(view) * mat3(model) * transformedNormal;
    
    ex_N = transformedNormal;
}