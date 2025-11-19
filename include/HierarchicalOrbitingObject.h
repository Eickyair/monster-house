#ifndef HIERARCHICAL_ORBITING_OBJECT_H
#define HIERARCHICAL_ORBITING_OBJECT_H

#include "HierarchicalObject.h"
#include "OrbitingMoonObject.h"
#include <cmath>

/**
 * @brief Objeto que orbita alrededor de su padre en la jerarquía
 */
class HierarchicalOrbitingObject : public HierarchicalObject {
private:
    OrbitingMoonObject* orbitingObject;
    
    float time;
    float orbitSpeed;
    float orbitRadius;
    float ellipseRatio;
    glm::vec3 orbitAngles;
    glm::vec3 orbitCenter; // NUEVO: Centro de la órbita
    
public:
    // Default constructor (no associated renderable)
    HierarchicalOrbitingObject()
        : HierarchicalObject(nullptr), orbitingObject(nullptr),
          time(0.0f), orbitSpeed(0.1f), orbitRadius(100.0f),
          ellipseRatio(1.0f), orbitAngles(0.0f), orbitCenter(0.0f) {
    }

    // Constructor que acepta un OrbitingMoonObject*
    HierarchicalOrbitingObject(OrbitingMoonObject* obj)
        : HierarchicalObject(obj), orbitingObject(obj),
          time(0.0f), orbitSpeed(0.1f), orbitRadius(100.0f), 
          ellipseRatio(1.0f), orbitAngles(0.0f), orbitCenter(0.0f) {
    }

    // Nuevo constructor que acepta un RenderableObject* (por ejemplo para el Sol)
    HierarchicalOrbitingObject(RenderableObject* obj)
        : HierarchicalObject(obj), orbitingObject(nullptr),
          time(0.0f), orbitSpeed(0.1f), orbitRadius(100.0f),
          ellipseRatio(1.0f), orbitAngles(0.0f), orbitCenter(0.0f) {
        // Intentar castear a OrbitingMoonObject si corresponde
        orbitingObject = dynamic_cast<OrbitingMoonObject*>(obj);
    }

    void setOrbitParameters(float speed, float radius, float ratio) {
        orbitSpeed = speed;
        orbitRadius = radius;
        ellipseRatio = ratio;
    }

    void setOrbitAngles(const glm::vec3& angles) {
        orbitAngles = angles;
    }
    
    void setOrbitCenter(const glm::vec3& center) {
        orbitCenter = center;
    }

    /**
     * @brief Calcula la matriz local incluyendo la órbita
     */
    glm::mat4 getLocalMatrix() const override {
        float t = time * orbitSpeed;
        
        // Calcular posición en la órbita elíptica
        float x = orbitRadius * cos(t);
        float z = orbitRadius * ellipseRatio * sin(t);
        float y = 0.0f;
        
        glm::vec3 orbitPos(x, y, z);
        
        // Crear matriz de transformación
        glm::mat4 matrix = glm::mat4(1.0f);
        
        // 1. Trasladar al centro de la órbita
        matrix = glm::translate(matrix, orbitCenter);
        
        // 2. Aplicar rotación de la órbita
        matrix = glm::rotate(matrix, glm::radians(orbitAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::rotate(matrix, glm::radians(orbitAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(orbitAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        
        // 3. Aplicar posición orbital
        matrix = glm::translate(matrix, orbitPos);
        
        // 4. Aplicar transformaciones locales adicionales
        matrix = glm::translate(matrix, localPosition);
        matrix = glm::rotate(matrix, glm::radians(localRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(localRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(localRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::scale(matrix, localScale);
        
        return matrix;
    }

    void update(float deltaTime) override {
        time += deltaTime;
        
        // Actualizar el objeto orbital si existe
        if (orbitingObject) {
            orbitingObject->update(deltaTime);
        }
        
        // Actualizar hijos
        HierarchicalObject::update(deltaTime);
    }

    void render(const glm::mat4& projection, const glm::mat4& view,
               const LightManager& lightManager, const glm::vec3& eyePosition,
               const glm::mat4& parentTransform = glm::mat4(1.0f)) override {
        
        glm::mat4 globalTransform = parentTransform * getLocalMatrix();
        
        // Renderizar el objeto orbital
        if (orbitingObject) {
            // Aplicar transformación jerárquica
            orbitingObject->setHierarchicalTransform(globalTransform);
            orbitingObject->render(projection, view, lightManager, eyePosition);
        }
        else if (renderableObject) {
            // Si no es un OrbitingMoonObject pero hay un RenderableObject (ej. Sol), aplicarlo
            renderableObject->setHierarchicalTransform(globalTransform);
            renderableObject->render(projection, view, lightManager, eyePosition);
        }
        
        // Renderizar hijos con la transformación acumulada
        for (auto* child : children) {
            if (child) {
                child->render(projection, view, lightManager, eyePosition, globalTransform);
            }
        }
    }

    float getTime() const { return time; }
    glm::vec3 getOrbitCenter() const { return orbitCenter; }
};

#endif // HIERARCHICAL_ORBITING_OBJECT_H