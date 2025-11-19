#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H

#include <string>
#include <GLFW/glfw3.h>
#include <glad/glad.h>

/**
 * @brief Pantalla de carga con barra de progreso
 */
class LoadingScreen {
private:
    GLFWwindow* window;
    int totalSteps;
    int currentStep;
    std::string currentMessage;

public:
    LoadingScreen(GLFWwindow* win, int steps) 
        : window(win), totalSteps(steps), currentStep(0), currentMessage("Inicializando...") {}

    void updateProgress(const std::string& message) {
        currentStep++;
        currentMessage = message;
        render();
    }

    void render() {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Aquí podrías dibujar una barra de progreso simple
        // Por ahora solo actualizamos la ventana
        float progress = (float)currentStep / (float)totalSteps * 100.0f;
        
        std::string title = "Cargando... " + std::to_string((int)progress) + "% - " + currentMessage;
        glfwSetWindowTitle(window, title.c_str());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
};

#endif // LOADING_SCREEN_H
