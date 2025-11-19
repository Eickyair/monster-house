#ifndef OBJECT_GENERATOR_H
#define OBJECT_GENERATOR_H

#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include "RenderableObject.h"
#include "SceneManager.h"

/**
 * @brief Generador de objetos 3D con distribución uniforme y control de distancias
 */
class ObjectGenerator {
private:
    std::mt19937 randomEngine;
    
    /**
     * @brief Calcula la distancia 2D entre dos posiciones (solo X-Z)
     */
    float distance2D(const glm::vec3& a, const glm::vec3& b) const {
        float dx = a.x - b.x;
        float dz = a.z - b.z;
        return std::sqrt(dx * dx + dz * dz);
    }
    
    /**
     * @brief Verifica si una posición está suficientemente lejos de todas las existentes
     */
    bool isValidPosition(const glm::vec3& newPos, 
                        const std::vector<glm::vec3>& existingPositions,
                        float minDistance) const {
        for (const auto& pos : existingPositions) {
            if (distance2D(newPos, pos) < minDistance) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * @brief Genera una posición aleatoria dentro de los límites especificados
     */
    glm::vec3 generateRandomPosition(float x_min, float x_max, 
                                     float z_min, float z_max,
                                     float y_fixed = 0.0f) {
        std::uniform_real_distribution<float> distX(x_min, x_max);
        std::uniform_real_distribution<float> distZ(z_min, z_max);
        
        return glm::vec3(distX(randomEngine), y_fixed, distZ(randomEngine));
    }
    
    /**
     * @brief Crea una variación aleatoria del material base
     */
    Material createMaterialVariation(const Material& baseMaterial) {
        Material variant = baseMaterial;
        
        // Variación de ±10% en componentes ambient/diffuse
        std::uniform_real_distribution<float> variation(0.7f, 1.3f);
        
        variant.ambient *= variation(randomEngine);
        variant.diffuse *= variation(randomEngine);
        variant.specular *= variation(randomEngine);
        
        // Asegurar que los valores estén en rango válido [0, 1]
        variant.ambient = glm::clamp(variant.ambient, glm::vec4(0.0f), glm::vec4(1.0f));
        variant.diffuse = glm::clamp(variant.diffuse, glm::vec4(0.0f), glm::vec4(1.0f));
        variant.specular = glm::clamp(variant.specular, glm::vec4(0.0f), glm::vec4(1.0f));
        
        return variant;
    }
    
public:
    /**
     * @brief Constructor con semilla opcional para reproducibilidad
     */
    ObjectGenerator(unsigned int seed = std::random_device{}()) 
        : randomEngine(seed) {
    }
    
    /**
     * @brief Establece una nueva semilla para el generador aleatorio
     */
    void setSeed(unsigned int seed) {
        randomEngine.seed(seed);
    }
    
    /**
     * @brief Genera y agrega múltiples objetos a la escena con control de distribución
     * 
     * @param sceneManager Gestor de escena donde se agregarán los objetos
     * @param model Modelo 3D a instanciar
     * @param baseMaterial Material base (se aplicarán variaciones)
     * @param shader Shader a utilizar
     * @param x_min Límite mínimo en X
     * @param x_max Límite máximo en X
     * @param z_min Límite mínimo en Z
     * @param z_max Límite máximo en Z
     * @param y_fixed Altura fija para todos los objetos
     * @param n_objects Número de objetos a generar
     * @param min_distance_between Distancia mínima entre objetos
     * @param rotations Vector de rotaciones disponibles (en grados)
     * @param scales Vector de escalas disponibles
     * @param initialRotation Rotación base para orientar el modelo correctamente (default: -90°, 0°, 0°)
     * @param maxAttempts Número máximo de intentos por objeto (default: 10)
     * @return Número de objetos colocados exitosamente
     */
    int generateObjects(SceneManager& sceneManager,
                       Model* model,
                       const Material& baseMaterial,
                       Shader* shader,
                       float x_min, float x_max,
                       float z_min, float z_max,
                       float y_fixed,
                       int n_objects,
                       float min_distance_between,
                       const std::vector<float>& rotations,
                       const std::vector<float>& scales,
                       const glm::vec3& initialRotation = glm::vec3(-90.0f, 0.0f, 0.0f),
                       int maxAttempts = 10) {
        
        if (rotations.empty() || scales.empty()) {
            std::cerr << "[ObjectGenerator] ERROR: rotations o scales vacíos" << std::endl;
            return 0;
        }
        
        std::vector<glm::vec3> placedPositions;
        int objectsPlaced = 0;
        
        std::uniform_int_distribution<size_t> rotDist(0, rotations.size() - 1);
        std::uniform_int_distribution<size_t> scaleDist(0, scales.size() - 1);
        
        std::cout << "[ObjectGenerator] Generando " << n_objects << " objetos..." << std::endl;
        std::cout << "[ObjectGenerator] Área: X[" << x_min << ", " << x_max 
                  << "] Z[" << z_min << ", " << z_max << "]" << std::endl;
        std::cout << "[ObjectGenerator] Distancia mínima: " << min_distance_between << std::endl;
        std::cout << "[ObjectGenerator] Rotación inicial: (" << initialRotation.x << "°, " 
                  << initialRotation.y << "°, " << initialRotation.z << "°)" << std::endl;
        
        for (int i = 0; i < n_objects; ++i) {
            bool placed = false;
            glm::vec3 position;
            
            // Intentar hasta maxAttempts veces encontrar una posición válida
            for (int attempt = 0; attempt < maxAttempts; ++attempt) {
                position = generateRandomPosition(x_min, x_max, z_min, z_max, y_fixed);
                
                if (isValidPosition(position, placedPositions, min_distance_between)) {
                    placed = true;
                    break;
                }
                
                // Incrementar semilla para próximo intento
                randomEngine.seed(randomEngine() + 1);
            }
            
            if (!placed) {
                std::cout << "[ObjectGenerator] ⚠️  Objeto " << i + 1 
                          << " omitido: no se encontró posición válida después de " 
                          << maxAttempts << " intentos" << std::endl;
                continue;
            }
            
            // Seleccionar rotación y escala aleatoria
            float variationRotation = rotations[rotDist(randomEngine)];
            float scale = scales[scaleDist(randomEngine)];
            
            // Crear rotación combinada: inicial + variación en Y
            glm::vec3 finalRotation = initialRotation;
            finalRotation.y += variationRotation;  // Agregar variación solo en eje Y
            
            glm::vec3 scaleVec(scale);
            
            // Crear material con variación
            Material objectMaterial = createMaterialVariation(baseMaterial);
            
            // Crear objeto renderizable con rotación inicial
            auto obj = std::make_unique<RenderableObject>(
                model, shader, position, glm::vec3(0.0f), scaleVec);
            
            // Establecer rotación inicial (orientación del modelo)
            obj->setInitialRotation(finalRotation);
            obj->setMaterial(objectMaterial);
            
            // Agregar a la escena
            sceneManager.addObject(std::move(obj));
            placedPositions.push_back(position);
            objectsPlaced++;
            
            if ((i + 1) % 10 == 0) {
                std::cout << "[ObjectGenerator] Progreso: " << objectsPlaced 
                          << "/" << n_objects << " objetos colocados" << std::endl;
            }
        }
        
        std::cout << "[ObjectGenerator] ✅ Generación completada: " 
                  << objectsPlaced << "/" << n_objects << " objetos colocados" << std::endl;
        
        return objectsPlaced;
    }
    
    /**
     * @brief Versión simplificada con rotaciones y escalas por defecto
     */
    int generateObjectsSimple(SceneManager& sceneManager,
                              Model* model,
                              const Material& baseMaterial,
                              Shader* shader,
                              float x_min, float x_max,
                              float z_min, float z_max,
                              float y_fixed,
                              int n_objects,
                              float min_distance_between,
                              const glm::vec3& initialRotation = glm::vec3(-90.0f, 0.0f, 0.0f)) {
        
        // Rotaciones por defecto: 0°, 90°, 180°, 270°
        std::vector<float> defaultRotations = {0.0f, 90.0f, 180.0f, 270.0f};
        
        // Escalas por defecto: 80% a 120%
        std::vector<float> defaultScales = {0.8f, 0.9f, 1.0f, 1.1f, 1.2f};
        
        return generateObjects(sceneManager, model, baseMaterial, shader,
                              x_min, x_max, z_min, z_max, y_fixed,
                              n_objects, min_distance_between,
                              defaultRotations, defaultScales, initialRotation);
    }
};

#endif // OBJECT_GENERATOR_H
