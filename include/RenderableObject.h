#ifndef RENDERABLE_OBJECT_H
#define RENDERABLE_OBJECT_H

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <model.h>
#include <shader_m.h>
#include <material.h>
#include "LightManager.h"

// Forward declaration
class LightManager;

/**
 * @brief Objeto base renderizable
 */
class RenderableObject {
protected:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 initialRotation;
    glm::vec3 initialTranslation;
    Model* model;
    Shader* shader;
    bool useBlending;
    glm::vec3* externalPosition;
    float* externalRotation;
    Material material;
    std::vector<size_t> affectedLights;
    
    // NUEVO: Soporte para transformación jerárquica
    bool useHierarchicalTransform;
    glm::mat4 hierarchicalTransform;

public:
    // Constructor para objetos con seguimiento (ej. jugador)
    RenderableObject(Model* mdl, Shader* shdr, glm::vec3* extPos = nullptr,
        float* extRot = nullptr, glm::vec3 scl = glm::vec3(1.0f),
        glm::vec3 initRot = glm::vec3(0.0f),
        glm::vec3 initTrans = glm::vec3(0.0f))
        : model(mdl), shader(shdr), position(extPos ? *extPos : glm::vec3(0.0f)),
          rotation(extRot ? glm::vec3(0.0f, *extRot, 0.0f) : glm::vec3(0.0f)),
          scale(scl), initialRotation(initRot), initialTranslation(initTrans),
          useBlending(false), externalPosition(extPos), externalRotation(extRot),
          useHierarchicalTransform(false), hierarchicalTransform(glm::mat4(1.0f)) {
        setDefaultMaterial();
    }

    // Constructor para objetos estáticos
    RenderableObject(Model* mdl, Shader* shdr, glm::vec3 pos,
        glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scl = glm::vec3(1.0f))
        : model(mdl), shader(shdr), position(pos), rotation(rot), scale(scl),
          initialRotation(glm::vec3(0.0f)), initialTranslation(glm::vec3(0.0f)),
          useBlending(false), externalPosition(nullptr), externalRotation(nullptr),
          useHierarchicalTransform(false), hierarchicalTransform(glm::mat4(1.0f)) {
        setDefaultMaterial();
    }

    virtual ~RenderableObject() = default;

    /**
     * @brief Actualiza el estado del objeto, ej. sincronizando con variables externas.
     */
    virtual void update(float deltaTime) {
        if (externalPosition) {
            position = *externalPosition;
        }
        if (externalRotation) {
            rotation.y = *externalRotation;
        }
    }

    /**
     * @brief Calcula la matriz de modelo final aplicando todas las transformaciones.
     */
    virtual glm::mat4 getModelMatrix() const {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        
        // Si hay transformación jerárquica, aplicarla primero
        if (useHierarchicalTransform) {
            modelMatrix = hierarchicalTransform;
        }
        
        // 1. Aplicar posición final
        modelMatrix = glm::translate(modelMatrix, position);
        // 2. Aplicar rotación del objeto (controlada por jugador o estática)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        // 3. Aplicar traslación inicial (offset del modelo)
        modelMatrix = glm::translate(modelMatrix, initialTranslation);
        // 4. Aplicar rotación inicial (corrección de exportación)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(initialRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(initialRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(initialRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        // 5. Aplicar escala
        modelMatrix = glm::scale(modelMatrix, scale);
        return modelMatrix;
    }

    /**
     * @brief Dibuja el objeto en la pantalla (VERSIÓN CON LUCES LOCALES)
     */
    virtual void render(const glm::mat4& projection, const glm::mat4& view,
        const LightManager& lightManager, const glm::vec3& eyePosition) {
        if (!model || !shader) return;

        shader->use();

        if (useBlending) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", getModelMatrix());

        // Aplicar luces globales + locales
        lightManager.applyLights(shader, affectedLights);

        shader->setVec3("eye", eyePosition);
        shader->setVec4("MaterialAmbientColor", material.ambient);
        shader->setVec4("MaterialDiffuseColor", material.diffuse);
        shader->setVec4("MaterialSpecularColor", material.specular);
        shader->setFloat("transparency", material.transparency);

        if (material.transparency < 1.0f) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        model->Draw(*shader);
        glUseProgram(0);
    }

    // Setters
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(const glm::vec3& rot) { rotation = rot; }
    void setScale(const glm::vec3& scl) { scale = scl; }
    void setUseBlending(bool use) { useBlending = use; }
    void setInitialRotation(const glm::vec3& rot) { initialRotation = rot; }
    void setInitialTranslation(const glm::vec3& trans) { initialTranslation = trans; }
    void setMaterial(const Material& mat) { material = mat; }
    
    /**
     * @brief Establece la transformación jerárquica externa
     */
    void setHierarchicalTransform(const glm::mat4& transform) {
        hierarchicalTransform = transform;
        useHierarchicalTransform = true;
    }
    
    /**
     * @brief Desactiva la transformación jerárquica
     */
    void disableHierarchicalTransform() {
        useHierarchicalTransform = false;
        hierarchicalTransform = glm::mat4(1.0f);
    }
    
    /**
     * @brief Agrega una luz local que afectará a este objeto
     */
    void addAffectedLight(size_t lightIndex) {
        if (std::find(affectedLights.begin(), affectedLights.end(), lightIndex) 
            == affectedLights.end()) {
            affectedLights.push_back(lightIndex);
        }
    }

    /**
     * @brief Remueve una luz local de este objeto
     */
    void removeAffectedLight(size_t lightIndex) {
        auto it = std::find(affectedLights.begin(), affectedLights.end(), lightIndex);
        if (it != affectedLights.end()) {
            affectedLights.erase(it);
        }
    }

    /**
     * @brief Limpia todas las luces locales (mantiene las globales)
     */
    void clearAffectedLights() {
        affectedLights.clear();
    }

    /**
     * @brief Establece todas las luces locales de una vez
     */
    void setAffectedLights(const std::vector<size_t>& lights) {
        affectedLights = lights;
    }

    // Getters
    Material& getMaterial() { return material; }
    const Material& getMaterial() const { return material; }
    const std::vector<size_t>& getAffectedLights() const { return affectedLights; }
    glm::vec3 getPosition() const { return position; }
    bool isUsingHierarchicalTransform() const { return useHierarchicalTransform; }

private:
    void setDefaultMaterial() {
        material.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        material.diffuse = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
        material.specular = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
        material.transparency = 1.0f;
    }
};

#endif // RENDERABLE_OBJECT_H
