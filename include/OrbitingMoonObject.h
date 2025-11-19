#ifndef ORBITING_MOON_OBJECT_H
#define ORBITING_MOON_OBJECT_H

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <model.h>
#include <shader_m.h>
#include "RenderableObject.h"

/**
 * @brief Calcula un ángulo de rotación en radianes basado en el tiempo y RPM.
 */
inline float rotationAngleRad(float timeSeconds, float rpm, float phaseRad = 0.0f)
{
    const float minutes = timeSeconds * (1.0f / 60.0f);
    const float turns = rpm * minutes;
    const float frac = turns - std::floor(turns);
    float angle = frac * glm::two_pi<float>() + phaseRad;
    angle = glm::mod(angle, glm::two_pi<float>());
    return angle;
}

/**
 * @brief Objeto con shader de órbita
 */
class OrbitingMoonObject : public RenderableObject {
private:
    float time;
    float orbitSpeed;
    float orbitRadius;
    float ellipseRatio;
    float height;
    glm::vec3 orbitCenter;
    glm::vec3 orbitAngles;
    float selfRotationRPM;

public:
    OrbitingMoonObject(Model* mdl, Shader* shdr, glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 scl = glm::vec3(1.0f))
        : RenderableObject(mdl, shdr, pos, glm::vec3(0.0f), scl),
          time(0.0f), orbitSpeed(0.2f), orbitRadius(30.0f), ellipseRatio(0.6f),
          height(0.0f), orbitCenter(0.0f), orbitAngles(90.0f, 60.0f, 0.0f),
          selfRotationRPM(10.0f) {
    }

    void update(float deltaTime) override {
        time += deltaTime;
    }

    /**
     * @brief Calcula la posición actual del objeto en su órbita.
     */
    glm::vec3 getCurrentOrbitPosition() const {
        float t = time * orbitSpeed;

        // Calcular posición base en la elipse
        float x = orbitRadius * cos(t);
        float z = orbitRadius * ellipseRatio * sin(t);
        float y = height;

        glm::vec3 orbitPos(x, y, z);

        // Aplicar rotaciones de la órbita
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(orbitAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(orbitAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(orbitAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec4 rotatedPos = rotationMatrix * glm::vec4(orbitPos, 1.0f);

        // Añadir el centro de la órbita
        return glm::vec3(rotatedPos) + orbitCenter + position;
    }

    /**
     * @brief Calcula una posición adelantada en la órbita (para la luz).
     */
    glm::vec3 getLeadingOrbitPosition(float leadAngle = 0.05f) const {
        float t = time * orbitSpeed + leadAngle;

        // Calcular posición base en la elipse
        float x = orbitRadius * cos(-t);
        float z = orbitRadius * ellipseRatio * sin(-t);
        float y = height;

        glm::vec3 orbitPos(x, y, z);

        // Aplicar rotación inicial del satélite
        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(initialRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(initialRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(initialRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));

        // Aplicar rotaciones de la órbita
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(orbitAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(orbitAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(orbitAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec4 rotatedPos = rotationMatrix * glm::vec4(orbitPos, 1.0f);

        // Añadir el centro de la órbita
        return glm::vec3(rotatedPos) + orbitCenter + position;
    }

    void render(const glm::mat4& projection, const glm::mat4& view,
        const LightManager& lightManager, const glm::vec3& eyePosition) override {
        if (!model || !shader) return;

        shader->use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", getModelMatrix());

        // Enviar uniformes específicos del shader de órbita
        shader->setFloat("time", time * orbitSpeed);
        shader->setFloat("radius", orbitRadius);
        shader->setFloat("ellipseRatio", ellipseRatio);
        shader->setFloat("height", height);
        shader->setVec3("orbitCenter", orbitCenter);
        shader->setFloat("orbitAngleX", glm::radians(orbitAngles.x));
        shader->setFloat("orbitAngleY", glm::radians(orbitAngles.y));
        shader->setFloat("orbitAngleZ", glm::radians(orbitAngles.z));

        float angle = rotationAngleRad(time, selfRotationRPM);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, scale);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(initialRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(initialRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(initialRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        shader->setMat4("model", modelMatrix);

        // Aplicar luces globales + locales
        lightManager.applyLights(shader, affectedLights);
        
        shader->setVec3("eye", eyePosition);
        shader->setVec4("MaterialAmbientColor", material.ambient);
        shader->setVec4("MaterialDiffuseColor", material.diffuse);
        shader->setVec4("MaterialSpecularColor", material.specular);
        shader->setFloat("transparency", material.transparency);

        model->Draw(*shader);
        glUseProgram(0);
    }

    // Setters para parámetros orbitales
    void setOrbitParameters(float speed, float radius, float ratio) {
        orbitSpeed = speed;
        orbitRadius = radius;
        ellipseRatio = ratio;
    }
    void setOrbitAngles(const glm::vec3& angles) { orbitAngles = angles; }
    void setSelfRotationRPM(float rpm) { selfRotationRPM = rpm; }
    void setOrbitCenter(const glm::vec3& center) { orbitCenter = center; }
    void setHeight(float h) { height = h; }
};

#endif // ORBITING_MOON_OBJECT_H
