#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <camera.h>
#include "PhysicsSystem.h"

// Forward declarations de variables globales que se usan
extern bool isPlayerMoving;
extern float movementSpeedFactor;
extern float mouseSensitivityFactor;
extern bool showLightIndicators;
extern PhysicsSystem physicsSystem;
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;

/**
 * @brief Manejo de entrada del jugador
 */
class InputController {
private:
    // Referencias a variables globales de estado
    glm::vec3& position;
    glm::vec3& forwardView;
    float& rotateCharacter;
    bool& activeCamera;
    float& trdpersonOffset;
    Camera& camera;
    Camera& camera3rd;

    // Estado interno del control
    bool cKeyPressed;
    bool spaceKeyPressed;
    bool lKeyPressed;
    float lastX, lastY;
    bool firstMouse;
    float scaleV; // Velocidad de movimiento base
    float runMultiplier;
    float cameraPitch;
    bool wasFirstPerson;

public:
    InputController(glm::vec3& pos, glm::vec3& forward, float& rotate, bool& activeCam,
        float& offset, Camera& cam1st, Camera& cam3rd)
        : position(pos), forwardView(forward), rotateCharacter(rotate),
          activeCamera(activeCam), trdpersonOffset(offset), camera(cam1st), camera3rd(cam3rd),
          cKeyPressed(false), spaceKeyPressed(false), lKeyPressed(false),
          lastX(SCR_WIDTH / 2.0f), lastY(SCR_HEIGHT / 2.0f),
          firstMouse(true), scaleV(0.01f), runMultiplier(2.5f),
          cameraPitch(0.0f), wasFirstPerson(true) {
    }

    void processKeyboard(GLFWwindow* window) {
        isPlayerMoving = false;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Modos de dibujado (Debug)
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

        // Ajuste de sensibilidad
        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            movementSpeedFactor += 0.01f;
            if (movementSpeedFactor > 5.0f) movementSpeedFactor = 5.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            movementSpeedFactor -= 0.01f;
            if (movementSpeedFactor < 0.1f) movementSpeedFactor = 0.1f;
        }

        // Multiplicador de velocidad (correr)
        bool isRunning = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

        float currentSpeed = (isRunning ? scaleV * runMultiplier : scaleV) * movementSpeedFactor;

        // Calcular vector derecho (perpendicular a forwardView)
        glm::vec3 rightVector = glm::normalize(glm::cross(forwardView, glm::vec3(0.0f, 1.0f, 0.0f)));

        // Movimiento (Adelante/Atrás)
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position = position + currentSpeed * forwardView;
            isPlayerMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position = position - currentSpeed * forwardView;
            isPlayerMoving = true;
        }

        // Movimiento Lateral (Strafing)
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position = position - currentSpeed * rightVector;
            isPlayerMoving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position = position + currentSpeed * rightVector;
            isPlayerMoving = true;
        }

        // Rotación con flechas izquierda/derecha
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            rotateCharacter += 0.2f;
            updateForwardView();
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            rotateCharacter -= 0.2f;
            updateForwardView();
        }

        // Salto
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!spaceKeyPressed) {
                physicsSystem.initiateJump();
                spaceKeyPressed = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
            spaceKeyPressed = false;
        }

        // Cambio de cámara (1ra/3ra persona)
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            if (!cKeyPressed) {
                activeCamera = !activeCamera;
                cKeyPressed = true;
                onCameraSwitch();
            }
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
            cKeyPressed = false;
        }

        // Toggle para indicadores de luz
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            if (!lKeyPressed) {
                showLightIndicators = !showLightIndicators;
                lKeyPressed = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
            lKeyPressed = false;
        }
    }

    void processMouse(GLFWwindow* window, double xpos, double ypos) {
        if (firstMouse) {
            lastX = (float)xpos;
            lastY = (float)ypos;
            firstMouse = false;
        }

        float xoffset = (float)xpos - lastX;
        lastX = (float)xpos;
        lastY = (float)ypos;

        // Rotar personaje con el mouse (con sensibilidad ajustable)
        float sensitivity = 0.1f * mouseSensitivityFactor;
        rotateCharacter -= xoffset * sensitivity;
        updateForwardView();
    }

    void processScroll(GLFWwindow* window, double xoffset, double yoffset) {
        if (activeCamera) { // 1ra persona: Pitch de la cámara
            cameraPitch += (float)yoffset * 2.0f;

            // Limitar pitch
            if (cameraPitch > 50.0f)
                cameraPitch = 50.0f;
            if (cameraPitch < -50.0f)
                cameraPitch = -50.0f;

            updateCameraDirection();
        }
        else { // 3ra persona: Zoom (offset)
            trdpersonOffset -= (float)yoffset * 0.5f;
            if (trdpersonOffset < 1.0f)
                trdpersonOffset = 1.0f;
            if (trdpersonOffset > 15.0f)
                trdpersonOffset = 15.0f;
        }
    }

    float getCameraPitch() const { return cameraPitch; }
    void setRunMultiplier(float multiplier) { runMultiplier = multiplier; }
    float getRunMultiplier() const { return runMultiplier; }

private:
    void onCameraSwitch() {
        if (!activeCamera) { // Al cambiar a 3ra persona
            cameraPitch = 0.0f; // Resetear pitch
        }
        updateCameraDirection();
        wasFirstPerson = activeCamera;
    }

    /**
     * @brief Actualiza el vector 'forwardView' basado en la rotación (Yaw) del personaje.
     */
    void updateForwardView() {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotateCharacter), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 viewVector = model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        forwardView = glm::normalize(glm::vec3(viewVector));

        updateCameraDirection(); // Actualizar también la cámara
    }

    /**
     * @brief Sincroniza la cámara (1ra y 3ra persona) con la rotación del personaje.
     */
    void updateCameraDirection() {
        glm::vec3 front;
        float yawRad = glm::radians(rotateCharacter);
        float pitchRad = glm::radians(cameraPitch);

        if (activeCamera) { // 1ra persona
            front.x = cos(pitchRad) * sin(yawRad);
            front.y = sin(pitchRad);
            front.z = cos(pitchRad) * cos(yawRad);
            camera.Front = glm::normalize(front);
        }
        else { // 3ra persona
            front.x = sin(yawRad);
            front.y = 0.0f;
            front.z = cos(yawRad);
            camera3rd.Front = glm::normalize(front);
        }

        // El 'forwardView' del jugador siempre es horizontal
        forwardView.x = sin(yawRad);
        forwardView.y = 0.0f;
        forwardView.z = cos(yawRad);
        forwardView = glm::normalize(forwardView);
    }
};

#endif // INPUT_CONTROLLER_H
