#ifndef ORBIT_VISUALIZER_H
#define ORBIT_VISUALIZER_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <shader_m.h>

/**
 * @brief Visualizador de órbitas y jerarquías
 */
class OrbitVisualizer {
private:
    Shader* shader;
    GLuint orbitVAO, orbitVBO;
    GLuint pointVAO, pointVBO;
    std::vector<glm::vec3> orbitPoints;
    int numSegments;
    bool initialized;

public:
    OrbitVisualizer() 
        : shader(nullptr), orbitVAO(0), orbitVBO(0), 
          pointVAO(0), pointVBO(0), numSegments(100), initialized(false) {
    }

    void initialize(const char* vertexPath, const char* fragmentPath) {
        shader = new Shader(vertexPath, fragmentPath);
        createOrbitCircle();
        createReferencePoint();
        initialized = true;
    }

    /**
     * @brief Dibuja una órbita elíptica
     */
    void drawOrbit(const glm::mat4& projection, const glm::mat4& view,
                   const glm::vec3& center, float radius, float ellipseRatio,
                   const glm::vec3& rotationAngles, const glm::vec4& color,
                   const glm::mat4& parentTransform = glm::mat4(1.0f)) {
        
        if (!initialized || !shader) return;

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setVec4("color", color);

        // Crear matriz de transformación de la órbita
        glm::mat4 model = parentTransform;
        model = glm::translate(model, center);
        
        // Aplicar rotaciones de la órbita
        model = glm::rotate(model, glm::radians(rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        
        // Aplicar escala para crear elipse
        model = glm::scale(model, glm::vec3(radius, 1.0f, radius * ellipseRatio));
        
        shader->setMat4("model", model);

        glLineWidth(2.0f);
        glBindVertexArray(orbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, numSegments);
        glBindVertexArray(0);
        glLineWidth(1.0f);
    }

    /**
     * @brief Dibuja un punto de referencia en una posición
     */
    void drawReferencePoint(const glm::mat4& projection, const glm::mat4& view,
                           const glm::vec3& position, const glm::vec4& color,
                           float size = 1.0f) {
        
        if (!initialized || !shader) return;

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setVec4("color", color);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(size));
        
        shader->setMat4("model", model);

        glPointSize(10.0f * size);
        glBindVertexArray(pointVAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
        glPointSize(1.0f);
    }

    /**
     * @brief Dibuja una línea entre dos puntos (para mostrar jerarquía)
     */
    void drawConnectionLine(const glm::mat4& projection, const glm::mat4& view,
                           const glm::vec3& start, const glm::vec3& end,
                           const glm::vec4& color) {
        
        if (!initialized || !shader) return;

        // Crear línea temporal
        float lineVertices[] = {
            start.x, start.y, start.z,
            end.x, end.y, end.z
        };

        GLuint tempVAO, tempVBO;
        glGenVertexArrays(1, &tempVAO);
        glGenBuffers(1, &tempVBO);
        
        glBindVertexArray(tempVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", glm::mat4(1.0f));
        shader->setVec4("color", color);

        glLineWidth(2.0f);
        glBindVertexArray(tempVAO);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);
        glLineWidth(1.0f);

        // Limpiar recursos temporales
        glDeleteVertexArrays(1, &tempVAO);
        glDeleteBuffers(1, &tempVBO);
    }

    ~OrbitVisualizer() {
        if (orbitVAO) glDeleteVertexArrays(1, &orbitVAO);
        if (orbitVBO) glDeleteBuffers(1, &orbitVBO);
        if (pointVAO) glDeleteVertexArrays(1, &pointVAO);
        if (pointVBO) glDeleteBuffers(1, &pointVBO);
        delete shader;
    }

private:
    /**
     * @brief Crea un círculo para representar órbitas
     */
    void createOrbitCircle() {
        orbitPoints.clear();
        
        for (int i = 0; i < numSegments; ++i) {
            float angle = 2.0f * glm::pi<float>() * float(i) / float(numSegments);
            float x = cos(angle);
            float y = 0.0f;
            float z = sin(angle);
            orbitPoints.push_back(glm::vec3(x, y, z));
        }

        glGenVertexArrays(1, &orbitVAO);
        glGenBuffers(1, &orbitVBO);
        
        glBindVertexArray(orbitVAO);
        glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
        glBufferData(GL_ARRAY_BUFFER, orbitPoints.size() * sizeof(glm::vec3), 
                     &orbitPoints[0], GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }

    /**
     * @brief Crea un punto de referencia
     */
    void createReferencePoint() {
        float point[] = { 0.0f, 0.0f, 0.0f };
        
        glGenVertexArrays(1, &pointVAO);
        glGenBuffers(1, &pointVBO);
        
        glBindVertexArray(pointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }
};

#endif // ORBIT_VISUALIZER_H
