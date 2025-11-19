#version 330 core

layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  tangent;
layout (location = 4) in vec3  bitangent;

out vec2 TexCoords;
out vec3 ex_N;

out vec3 vertexPosition_cameraspace;
out vec3 Normal_cameraspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// ==== Parámetros de la órbita ====
// time: tiempo global en segundos
uniform float time;
// radius: semieje mayor
uniform float radius;
// height: desplazamiento local sobre el eje Z de la órbita antes de rotarla
uniform float height;
// ratio entre semieje menor y mayor (ry = radius * ellipseRatio)
uniform float ellipseRatio;
// centro de la órbita en espacio mundo
uniform vec3  orbitCenter;

// Ángulos de orientación de la órbita (en radianes)
uniform float orbitAngleX;  // rotación alrededor de eje X
uniform float orbitAngleY;  // rotación alrededor de eje Y
uniform float orbitAngleZ;  // rotación alrededor de eje Z

// Funciones de rotación 3D
mat3 rotX(float a) {
    float c = cos(a);
    float s = sin(a);
    return mat3(
        1.0, 0.0, 0.0,
        0.0,   c,  -s,
        0.0,   s,   c
    );
}

mat3 rotY(float a) {
    float c = cos(a);
    float s = sin(a);
    return mat3(
          c, 0.0,  s,
        0.0, 1.0, 0.0,
         -s, 0.0,  c
    );
}

mat3 rotZ(float a) {
    float c = cos(a);
    float s = sin(a);
    return mat3(
          c,  -s, 0.0,
          s,   c, 0.0,
        0.0, 0.0, 1.0
    );
}

void main()
{
    // ===============================
    // 1) Posición base en espacio mundo
    // ===============================
    vec4 worldPos = model * vec4(aPos, 1.0);

    // ===============================
    // 2) Órbita elíptica en espacio local de órbita
    //    (elipse en el plano XY, con Z = height)
    // ===============================
    float angle = time;             // si quieres velocidad: time * speed desde C++
    float rx = radius;              // semieje mayor
    float ry = radius * ellipseRatio; // semieje menor

    // Posición en la elipse antes de rotar
    vec3 localPos = vec3(
        rx * cos(angle),
        ry * sin(angle),
        height
    );

    // ===============================
    // 3) Rotar la órbita según tres ángulos (X, Y, Z)
    //    Orden elegido: R = Rz * Ry * Rx
    // ===============================
    mat3 Rx = rotX(orbitAngleX);
    mat3 Ry = rotY(orbitAngleY);
    mat3 Rz = rotZ(orbitAngleZ);

    mat3 R = Rz * Ry * Rx;      // primero X, luego Y, luego Z

    vec3 rotatedPos = R * localPos;

    // Offset final de la órbita en mundo
    vec3 orbitOffsetWorld = orbitCenter + rotatedPos;

    // Sumamos la órbita a la posición del vértice
    worldPos.xyz += orbitOffsetWorld;

    // ===============================
    // 4) Transformaciones a espacio de cámara y clip space
    // ===============================
    vec4 posCamera = view * worldPos;
    gl_Position = projection * posCamera;

    vertexPosition_cameraspace = posCamera.xyz;
    Normal_cameraspace = (view * model * vec4(aNormal, 0.0)).xyz;

    TexCoords = aTexCoords;
    ex_N = aNormal;
}
