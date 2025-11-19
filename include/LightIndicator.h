#ifndef LIGHT_INDICATOR_H
#define LIGHT_INDICATOR_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <shader_m.h>

/**
 * @brief Ayuda visual de luces (esferas en las posiciones de las luces)
 */
class LightIndicator {
private:
    GLuint VAO, VBO, EBO;
    Shader* shader;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec4> lightColors;
    bool initialized;

public:
    LightIndicator() : VAO(0), VBO(0), EBO(0), shader(nullptr), initialized(false) {}

    void initialize(const char* vertexPath, const char* fragmentPath) {
        shader = new Shader(vertexPath, fragmentPath);
        createSphere();
        initialized = true;
    }

    /**
     * @brief Agrega una luz al indicador y retorna su índice
     * @return Índice de la luz agregada
     */
    size_t addLight(const glm::vec3& position, const glm::vec4& color) {
        size_t index = lightPositions.size();
        lightPositions.push_back(position);
        lightColors.push_back(color);
        return index;
    }

    void updateLightPosition(size_t index, const glm::vec3& newPosition) {
        if (index < lightPositions.size()) {
            lightPositions[index] = newPosition;
        }
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) {
        if (!initialized || !shader || VAO == 0) return;

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        glBindVertexArray(VAO);

        for (size_t i = 0; i < lightPositions.size(); ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.5f)); // Esfera pequeña

            shader->setMat4("model", model);
            shader->setVec4("lightColor", lightColors[i]);

            glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0); // 180 es por la esfera (10x10)
        }

        glBindVertexArray(0);
    }

    ~LightIndicator() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        delete shader;
    }

private:
    /**
     * @brief Crea una malla de esfera UV de baja resolución (10x10) para el indicador.
     */
    void createSphere() {
        const int sectors = 10;
        const int stacks = 10;
        const float radius = 1.0f;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (int i = 0; i <= stacks; ++i) {
            float stackAngle = glm::pi<float>() / 2.0f - i * glm::pi<float>() / stacks;
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = j * 2.0f * glm::pi<float>() / sectors;
                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }

        for (int i = 0; i < stacks; ++i) {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != (stacks - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
};

#endif // LIGHT_INDICATOR_H
