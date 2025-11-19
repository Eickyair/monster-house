#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <vector>
#include <algorithm>
#include <sstream>
#include <glm/glm.hpp>
#include <shader_m.h>
#include <light.h>

/**
 * @brief Sistema de iluminación basado en referencias directas a luces
 * Cada objeto mantiene una lista de índices de luces que lo afectan
 */
class LightManager {
private:
    std::vector<Light> lights;              // Todas las luces de la escena
    std::vector<size_t> globalLightIndices; // Índices de luces globales

public:
    /**
     * @brief Añade una luz y retorna su índice
     * @param light Luz a agregar
     * @param isGlobal Si es true, esta luz afectará a todos los objetos
     * @return Índice de la luz agregada (usar para referenciarla en objetos)
     */
    size_t addLight(const Light& light, bool isGlobal = false) {
        size_t index = lights.size();
        lights.push_back(light);
        
        if (isGlobal) {
            globalLightIndices.push_back(index);
        }
        
        return index;
    }

    /**
     * @brief Marca una luz existente como global
     */
    void setLightAsGlobal(size_t lightIndex, bool isGlobal = true) {
        if (lightIndex >= lights.size()) return;
        
        // Buscar si ya está en globales
        auto it = std::find(globalLightIndices.begin(), globalLightIndices.end(), lightIndex);
        bool alreadyGlobal = (it != globalLightIndices.end());
        
        if (isGlobal && !alreadyGlobal) {
            globalLightIndices.push_back(lightIndex);
        } else if (!isGlobal && alreadyGlobal) {
            globalLightIndices.erase(it);
        }
    }

    /**
     * @brief Actualiza la posición de una luz específica por su índice
     */
    void updateLightPosition(size_t index, const glm::vec3& newPosition) {
        if (index < lights.size()) {
            lights[index].Position = newPosition;
        }
    }

    /**
     * @brief Obtiene un puntero a una luz por su índice
     */
    Light* getLight(size_t index) {
        if (index < lights.size()) {
            return &lights[index];
        }
        return nullptr;
    }

    /**
     * @brief Aplica luces al shader: primero globales, luego locales del objeto
     * @param shader Shader al que enviar las luces
     * @param localLightIndices Índices de luces locales específicas del objeto
     */
    void applyLights(Shader* shader, const std::vector<size_t>& localLightIndices) const {
        // Combinar luces globales + luces locales (sin duplicados)
        std::vector<size_t> activeLightIndices;
        
        // 1. Agregar luces globales primero
        for (size_t globalIdx : globalLightIndices) {
            if (globalIdx < lights.size()) {
                activeLightIndices.push_back(globalIdx);
            }
        }
        
        // 2. Agregar luces locales (evitando duplicados con globales)
        for (size_t localIdx : localLightIndices) {
            if (localIdx < lights.size()) {
                // Verificar que no esté ya en la lista
                if (std::find(activeLightIndices.begin(), activeLightIndices.end(), localIdx) 
                    == activeLightIndices.end()) {
                    activeLightIndices.push_back(localIdx);
                }
            }
        }

        // 3. Enviar número total de luces activas
        shader->setInt("numLights", static_cast<int>(activeLightIndices.size()));
        
        // 4. Enviar datos de cada luz activa
        for (size_t i = 0; i < activeLightIndices.size(); ++i) {
            const Light& light = lights[activeLightIndices[i]];
            
            setLightUniform(shader, "Position", i, light.Position);
            setLightUniform(shader, "Direction", i, light.Direction);
            setLightUniform(shader, "Color", i, light.Color);
            setLightUniform(shader, "Power", i, light.Power);
            setLightUniform(shader, "alphaIndex", i, light.alphaIndex);
            setLightUniform(shader, "distance", i, light.distance);
        }
    }

    /**
     * @brief Obtiene los índices de todas las luces globales
     */
    const std::vector<size_t>& getGlobalLights() const {
        return globalLightIndices;
    }

    size_t getLightCount() const { return lights.size(); }
    size_t getGlobalLightCount() const { return globalLightIndices.size(); }

private:
    /**
     * @brief Helpers para enviar uniformes de luz al shader (C++14 compatible)
     * Sobrecarga para int
     */
    void setLightUniform(Shader* shader, const char* propertyName, size_t lightIndex, int value) const {
        std::ostringstream ss;
        ss << "allLights[" << lightIndex << "]." << propertyName;
        shader->setInt(ss.str().c_str(), value);
    }

    /**
     * @brief Sobrecarga para float
     */
    void setLightUniform(Shader* shader, const char* propertyName, size_t lightIndex, float value) const {
        std::ostringstream ss;
        ss << "allLights[" << lightIndex << "]." << propertyName;
        shader->setFloat(ss.str().c_str(), value);
    }

    /**
     * @brief Sobrecarga para glm::vec3
     */
    void setLightUniform(Shader* shader, const char* propertyName, size_t lightIndex, const glm::vec3& value) const {
        std::ostringstream ss;
        ss << "allLights[" << lightIndex << "]." << propertyName;
        shader->setVec3(ss.str().c_str(), value);
    }

    /**
     * @brief Sobrecarga para glm::vec4
     */
    void setLightUniform(Shader* shader, const char* propertyName, size_t lightIndex, const glm::vec4& value) const {
        std::ostringstream ss;
        ss << "allLights[" << lightIndex << "]." << propertyName;
        shader->setVec4(ss.str().c_str(), value);
    }
};

#endif // LIGHT_MANAGER_H
