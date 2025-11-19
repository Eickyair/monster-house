#ifndef HIERARCHICAL_OBJECT_H
#define HIERARCHICAL_OBJECT_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "RenderableObject.h"

/**
 * @brief Objeto con transformaciones jerárquicas (padre-hijo)
 */
class HierarchicalObject {
protected:
    glm::vec3 localPosition;
    glm::vec3 localRotation;
    glm::vec3 localScale;
    
    HierarchicalObject* parent;
    std::vector<HierarchicalObject*> children;
    
    RenderableObject* renderableObject;

public:
    HierarchicalObject(RenderableObject* obj = nullptr)
        : localPosition(0.0f), localRotation(0.0f), localScale(1.0f),
          parent(nullptr), renderableObject(obj) {
    }

    virtual ~HierarchicalObject() = default;

    /**
     * @brief Añade un hijo a este nodo
     */
    void addChild(HierarchicalObject* child) {
        if (child && child->parent != this) {
            // Remover del padre anterior si existe
            if (child->parent) {
                child->parent->removeChild(child);
            }
            
            children.push_back(child);
            child->parent = this;
        }
    }

    /**
     * @brief Remueve un hijo de este nodo
     */
    void removeChild(HierarchicalObject* child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            (*it)->parent = nullptr;
            children.erase(it);
        }
    }

    /**
     * @brief Calcula la matriz de transformación local
     */
    virtual glm::mat4 getLocalMatrix() const {
        glm::mat4 matrix = glm::mat4(1.0f);
        
        // Aplicar transformaciones en orden: Traslación -> Rotación -> Escala
        matrix = glm::translate(matrix, localPosition);
        
        matrix = glm::rotate(matrix, glm::radians(localRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(localRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(localRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        
        matrix = glm::scale(matrix, localScale);
        
        return matrix;
    }

    /**
     * @brief Calcula la matriz de transformación global (acumulada desde la raíz)
     */
    glm::mat4 getGlobalMatrix() const {
        if (parent) {
            return parent->getGlobalMatrix() * getLocalMatrix();
        }
        return getLocalMatrix();
    }

    /**
     * @brief Obtiene la posición global en el mundo
     */
    glm::vec3 getGlobalPosition() const {
        glm::mat4 globalMat = getGlobalMatrix();
        return glm::vec3(globalMat[3]);
    }

    /**
     * @brief Actualiza el nodo y todos sus hijos
     */
    virtual void update(float deltaTime) {
        // Actualizar hijos
        for (auto* child : children) {
            if (child) {
                child->update(deltaTime);
            }
        }
    }

    /**
     * @brief Renderiza el nodo y todos sus hijos
     */
    virtual void render(const glm::mat4& projection, const glm::mat4& view,
                       const LightManager& lightManager, const glm::vec3& eyePosition,
                       const glm::mat4& parentTransform = glm::mat4(1.0f)) {
        
        glm::mat4 globalTransform = parentTransform * getLocalMatrix();
        
        // Renderizar este objeto si tiene un RenderableObject
        if (renderableObject) {
            // Aplicar la transformación jerárquica al objeto
            renderableObject->setHierarchicalTransform(globalTransform);
            renderableObject->render(projection, view, lightManager, eyePosition);
        }
        
        // Renderizar hijos
        for (auto* child : children) {
            if (child) {
                child->render(projection, view, lightManager, eyePosition, globalTransform);
            }
        }
    }

    // Setters para transformaciones locales
    void setLocalPosition(const glm::vec3& pos) { localPosition = pos; }
    void setLocalRotation(const glm::vec3& rot) { localRotation = rot; }
    void setLocalScale(const glm::vec3& scl) { localScale = scl; }
    
    // Getters
    glm::vec3 getLocalPosition() const { return localPosition; }
    glm::vec3 getLocalRotation() const { return localRotation; }
    glm::vec3 getLocalScale() const { return localScale; }
    
    RenderableObject* getRenderableObject() { return renderableObject; }
    const std::vector<HierarchicalObject*>& getChildren() const { return children; }
};

#endif // HIERARCHICAL_OBJECT_H