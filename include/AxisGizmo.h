#ifndef AXIS_GIZMO_H
#define AXIS_GIZMO_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader_m.h>

/**
 * @brief Ayuda visual de ejes (X, Y, Z)
 */
class AxisGizmo {
private:
    Shader* shader;
    GLuint VAO, VBO;
    float length;

public:
    AxisGizmo(float axisLength = 5.0f) : shader(nullptr), VAO(0), VBO(0), length(axisLength) {}

    void initialize(const char* vertexPath, const char* fragmentPath) {
        shader = new Shader(vertexPath, fragmentPath);
        createAxes();
    }

    void draw(const glm::mat4& projection, const glm::mat4& view,
        const glm::vec3& origin = glm::vec3(0.0f),
        float uniformScale = 1.0f, bool overlay = false) {
        if (!shader || VAO == 0) return;

        if (overlay) glDisable(GL_DEPTH_TEST); // Dibujar encima de todo
        glLineWidth(2.0f);

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        glm::mat4 model(1.0f);
        model = glm::translate(model, origin);
        model = glm::scale(model, glm::vec3(uniformScale));
        shader->setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);

        glLineWidth(1.0f);
        if (overlay) glEnable(GL_DEPTH_TEST);
    }

    ~AxisGizmo() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        delete shader;
    }

private:
    void createAxes() {
        const float L = length;
        float axisVerts[] = {
            // X axis (Rojo)
            0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            L,    0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            // Y axis (Verde)
            0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
            0.0f,  L,   0.0f,   0.0f, 1.0f, 0.0f,
            // Z axis (Azul)
            0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
            0.0f, 0.0f,  L,    0.0f, 0.0f, 1.0f,
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axisVerts), axisVerts, GL_STATIC_DRAW);

        // Posición
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};

#endif // AXIS_GIZMO_H
