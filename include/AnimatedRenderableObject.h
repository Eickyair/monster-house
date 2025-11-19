#ifndef ANIMATED_RENDERABLE_OBJECT_H
#define ANIMATED_RENDERABLE_OBJECT_H

#include <glm/glm.hpp>
#include <animatedmodel.h>
#include <shader_m.h>
#include "RenderableObject.h"
#include "PhysicsSystem.h"

/**
 * @brief Objeto renderizable con animación
 */
class AnimatedRenderableObject : public RenderableObject {
private:
    AnimatedModel* animatedModel;
    glm::vec3* externalPosition;
    float* externalRotation;
    bool isMoving;
    glm::vec3 lastPosition;
    PhysicsSystem* physicsSystem;

public:
    AnimatedRenderableObject(AnimatedModel* mdl, Shader* shdr, PhysicsSystem* physics,
        glm::vec3* extPos = nullptr, float* extRot = nullptr,
        glm::vec3 scl = glm::vec3(1.0f))
        : RenderableObject(nullptr, shdr, glm::vec3(0.0f), glm::vec3(0.0f), scl),
          animatedModel(mdl), physicsSystem(physics),
          externalPosition(extPos), externalRotation(extRot),
          isMoving(false), lastPosition(0.0f) {
        if (externalPosition) {
            lastPosition = *externalPosition;
        }
    }

    void update(float deltaTime) override {
        // Sincronizar con la posición externa (jugador)
        if (externalPosition) {
            position = *externalPosition;

            // Detectar si el jugador se está moviendo
            const float movementThreshold = 0.0001f;
            glm::vec3 positionDelta = position - lastPosition;
            float movementDistance = glm::length(positionDelta);

            isMoving = (movementDistance > movementThreshold);
            lastPosition = position;
        }

        if (externalRotation) {
            rotation.y = *externalRotation;
        }

        // Actualizar la animación solo si el modelo se está moviendo
        if (animatedModel && isMoving) {
            animatedModel->UpdateAnimation(deltaTime);
        }

        // Actualizar el sistema de físicas (salto)
        if (physicsSystem) {
            physicsSystem->update(deltaTime);
        }
    }

    void render(const glm::mat4& projection, const glm::mat4& view,
        const LightManager& lightManager, const glm::vec3& eyePosition) override {
        if (!animatedModel || !shader) return;

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", getModelMatrix());

        // Enviar datos de los huesos (skinning)
        shader->setMat4("gBones", MAX_RIGGING_BONES, animatedModel->gBones);

        // Enviar datos de físicas
        if (physicsSystem) {
            shader->setFloat("physicsTime", physicsSystem->getJumpTime());
            shader->setBool("isJumping", physicsSystem->getIsJumping());
            shader->setFloat("initialVelocity", physicsSystem->getInitialVelocity());
            shader->setFloat("lunarGravity", physicsSystem->getLunarGravity());
            shader->setFloat("astronautMass", physicsSystem->getAstronautMass());
            shader->setFloat("groundLevel", physicsSystem->getGroundLevel());
        }
        else {
            shader->setFloat("physicsTime", 0.0f);
            shader->setBool("isJumping", false);
            shader->setFloat("initialVelocity", 0.0f);
            shader->setFloat("lunarGravity", 1.62f);
            shader->setFloat("astronautMass", 180.0f);
            shader->setFloat("groundLevel", 0.0f);
        }

        // Aplicar luces globales + locales
        lightManager.applyLights(shader, affectedLights);
        
        shader->setVec3("eye", eyePosition);
        shader->setVec4("MaterialAmbientColor", material.ambient);
        shader->setVec4("MaterialDiffuseColor", material.diffuse);
        shader->setVec4("MaterialSpecularColor", material.specular);
        shader->setFloat("transparency", material.transparency);

        animatedModel->Draw(*shader);
        glUseProgram(0);
    }

    bool getIsMoving() const { return isMoving; }
};

#endif // ANIMATED_RENDERABLE_OBJECT_H
