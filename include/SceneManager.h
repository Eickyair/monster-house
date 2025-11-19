#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <vector>
#include <memory>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>
#include <cubemap.h>
#include <shader_m.h>
#include "RenderableObject.h"
#include "OrbitingMoonObject.h"
#include "LightManager.h"
#include "AxisGizmo.h"
#include "LightIndicator.h"
#include "HierarchicalObject.h"
#include "OrbitVisualizer.h"
#include <unordered_set>
#include <functional>

// Forward declaration de variable global
extern bool showLightIndicators;
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

/**
 * @brief Controlador principal de la escena
 */
class SceneManager {
private:
    std::vector<std::unique_ptr<RenderableObject>> objects;
    LightManager lightManager;
    Material defaultMaterial;
    CubeMap* cubemap;
    Shader* cubemapShader;
    AxisGizmo* axisGizmo;
    LightIndicator* lightIndicator;
    OrbitVisualizer* orbitVisualizer;

    // Referencias a cámaras
    Camera& camera;
    Camera& camera3rd;
    bool& activeCamera;

    // Soporte para múltiples satélites con índices separados
    struct SatelliteLightPair {
        OrbitingMoonObject* satellite;
        size_t lightManagerIndex;    // Índice en LightManager
        size_t lightIndicatorIndex;  // Índice en LightIndicator
    };
    std::vector<SatelliteLightPair> satelliteLights;

    // Sistema jerárquico
    std::vector<std::unique_ptr<HierarchicalObject>> hierarchicalObjects;
    HierarchicalObject* worldRoot;

public:
    SceneManager(Camera& cam1st, Camera& cam3rd, bool& activeCam)
        : cubemap(nullptr), cubemapShader(nullptr), axisGizmo(nullptr), 
          lightIndicator(nullptr), orbitVisualizer(nullptr), worldRoot(nullptr),
          camera(cam1st), camera3rd(cam3rd), activeCamera(activeCam) {
    }

    ~SceneManager() {
        delete cubemap;
        delete axisGizmo;
        delete lightIndicator;
        delete orbitVisualizer;
    }

    void addObject(std::unique_ptr<RenderableObject> obj) {
        objects.push_back(std::move(obj));
    }

    LightManager& getLightManager() { return lightManager; }
    Material& getMaterial() { return defaultMaterial; }

    void setCubemap(CubeMap* cm, Shader* shader) {
        cubemap = cm;
        cubemapShader = shader;
    }

    void setAxisGizmo(AxisGizmo* gizmo) {
        axisGizmo = gizmo;
    }

    void setLightIndicator(LightIndicator* indicator) {
        lightIndicator = indicator;
    }

    void setOrbitVisualizer(OrbitVisualizer* visualizer) {
        orbitVisualizer = visualizer;
    }

    void addHierarchicalObject(std::unique_ptr<HierarchicalObject> obj) {
        hierarchicalObjects.push_back(std::move(obj));
    }

    void setWorldRoot(HierarchicalObject* root) {
        worldRoot = root;
    }

    HierarchicalObject* getWorldRoot() { return worldRoot; }

    /**
     * @brief Vincula una luz a un satélite orbital para que lo siga
     */
    void addSatelliteLight(OrbitingMoonObject* satellite, size_t lightManagerIndex, size_t lightIndicatorIndex) {
        satelliteLights.push_back({satellite, lightManagerIndex, lightIndicatorIndex});
    }

    void update(float deltaTime) {
        // Actualizar todas las luces dinámicas de satélites
        for (auto& pair : satelliteLights) {
            if (pair.satellite != nullptr) {
                glm::vec3 leadingPos = pair.satellite->getLeadingOrbitPosition(0.5f);
                
                // Actualizar posición en LightManager
                lightManager.updateLightPosition(pair.lightManagerIndex, leadingPos);

                // Actualizar posición en LightIndicator
                if (lightIndicator) {
                    lightIndicator->updateLightPosition(pair.lightIndicatorIndex, leadingPos);
                }
            }
        }

        // Actualizar todos los objetos de la escena
        for (auto& obj : objects) {
            obj->update(deltaTime);
        }
        
        // Actualizar jerarquía si existe
        if (worldRoot) {
            worldRoot->update(deltaTime);
        }
    }

    void render() {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec3 eyePosition;

        if (activeCamera) {
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
            view = camera.GetViewMatrix();
            eyePosition = camera.Position;
        }
        else {
            projection = glm::perspective(glm::radians(camera3rd.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
            view = camera3rd.GetViewMatrix();
            eyePosition = camera3rd.Position;
        }

        // Dibujar cubemap si está disponible
        if (cubemap && cubemapShader) {
            cubemap->drawCubeMap(*cubemapShader, projection, view);
        }

        // Recolectar punteros a RenderableObject que están en la jerarquía
        std::unordered_set<RenderableObject*> hierarchicalSet;
        if (worldRoot) {
            std::function<void(HierarchicalObject*)> collect = [&](HierarchicalObject* node) {
                if (!node) return;
                RenderableObject* ro = node->getRenderableObject();
                if (ro) hierarchicalSet.insert(ro);
                for (auto* c : node->getChildren()) {
                    if (c) collect(c);
                }
            };
            collect(worldRoot);
        }

        // 1) Renderizar jerarquía primero (si existe) para aplicar transformaciones jerárquicas
        if (worldRoot) {
            worldRoot->render(projection, view, lightManager, eyePosition);
        }

        // 2) Renderizar objetos normales, omitiendo los que forman parte de la jerarquía
        for (auto& obj : objects) {
            if (hierarchicalSet.find(obj.get()) != hierarchicalSet.end()) {
                continue; // este objeto ya fue renderizado por la jerarquía
            }
            obj->render(projection, view, lightManager, eyePosition);
        }

        // 3. Dibujar el gizmo de ejes
        if (axisGizmo) {
            axisGizmo->draw(projection, view, glm::vec3(0.1f), 1.0f, false);
        }

        // 4. Dibujar los indicadores de luz
        if (lightIndicator && showLightIndicators) {
            lightIndicator->draw(projection, view);
        }
    }
};

#endif // SCENE_MANAGER_H
