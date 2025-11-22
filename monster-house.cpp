/*
*
* 08 - Modelos de Iluminación
*/

#include <iostream>
#include <stdlib.h>
#include <sstream>   // NUEVO
#include <vector>    // NUEVO
#include <algorithm> // opcional

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <material.h>
#include <light.h>
#include <cubemap.h>

#include <irrKlang.h>
using namespace irrklang;

// Functions
bool Start();
bool Update();

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Helpers de luces
void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value);
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value);
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value);
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara (posición en XYZ)
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float elapsedTime = 0.0f;

// Shaders
Shader* cubemapShader;
Shader* phonIlumShader;

// Carga la información del modelo
Model* lightDummy;
Model* monsterHouse;
// Cubemap
CubeMap* mainCubeMap;

// Materiales
Material material;

// Luces base y subconjuntos por objeto
std::vector<Light> globalLights;

// Audio
ISoundEngine* SoundEngine = createIrrKlangDevice();

// Entrada a función principal
int main()
{
	if (!Start())
		return -1;

	while (!glfwWindowShouldClose(window))
	{
		if (!Update())
			break;
	}

	glfwTerminate();
	return 0;
}

bool Start() {
	// Inicialización de GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creación de la ventana con GLFW
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Illumination Models", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// glad: Cargar todos los apuntadores
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// DEPTH
	glEnable(GL_DEPTH_TEST);

	// Compilación y enlace de shaders
	// (mismo VS/FS para los 3, instancias separadas)
	const char* VS = "shaders/11_PhongShaderMultLights.vs";
	const char* FS = "shaders/11_PhongShaderMultLights.fs";
	phonIlumShader = new Shader(VS, FS);

	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");

	// Modelos
	lightDummy = new Model("models/lightDummy.fbx");
	monsterHouse = new Model("models/monster_house.fbx");

	// Cubemap
	std::vector<std::string> faces{
		"textures/cubemap/01/px.jpg",
		"textures/cubemap/01/nx.jpg",
		"textures/cubemap/01/py.jpg",
		"textures/cubemap/01/ny.jpg",
		"textures/cubemap/01/pz.jpg",
		"textures/cubemap/01/nz.jpg"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);


	Light l1;
	l1.Position = glm::vec3(0.0f, 10.0f, 4.0f);
	l1.Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	l1.Power = 50.0f * glm::vec4(2.0f);
	l1.alphaIndex = 32;
	l1.distance = 15.0f; // Distancia de 1 para evitar división muy grande
	globalLights.push_back(l1);

	return true;
}

void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	shader->setInt(ss.str().c_str(), value);
}
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	shader->setFloat(ss.str().c_str(), value);
}
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	shader->setVec4(ss.str().c_str(), value);
}
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
	std::ostringstream ss;
	ss << "allLights[" << lightIndex << "]." << propertyName;
	shader->setVec3(ss.str().c_str(), value);
}

static inline void UploadLightsMax4(Shader* shader, const std::vector<Light>& L) {
	int n = (int)std::min<size_t>(L.size(), 4); // máx 4 luces
	shader->setInt("numLights", n);
	for (int i = 0; i < n; ++i) {
		SetLightUniformVec3(shader, "Position", i, L[i].Position);
		SetLightUniformVec3(shader, "Direction", i, L[i].Direction);
		SetLightUniformVec4(shader, "Color", i, L[i].Color);
		SetLightUniformVec4(shader, "Power", i, L[i].Power);
		SetLightUniformInt(shader, "alphaIndex", i, L[i].alphaIndex);
		SetLightUniformFloat(shader, "distance", i, L[i].distance);
	}
}

bool Update() {
	// Cálculo del framerate
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Entrada
	processInput(window);

	// Clear con color oscuro para ver mejor
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // CAMBIADO
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// Cubemap
	{
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}
	glUseProgram(0);

	// DIBUJAR LIGHT DUMMIES
	{
		phonIlumShader->use();
		phonIlumShader->setMat4("projection", projection);
		phonIlumShader->setMat4("view", view);
		phonIlumShader->setVec3("eye", camera.Position);

		// Material emisivo para los dummies (brillantes)
		phonIlumShader->setVec4("MaterialAmbientColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		phonIlumShader->setVec4("MaterialDiffuseColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		phonIlumShader->setVec4("MaterialSpecularColor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		phonIlumShader->setFloat("transparency", 1.0f);

		// Sin luces para los dummies (auto-iluminados)
		phonIlumShader->setInt("numLights", 0);

		// Dibujar un dummy por cada luz
		for (size_t i = 0; i < globalLights.size(); ++i) {
			glm::mat4 lightModel = glm::mat4(1.0f);
			lightModel = glm::translate(lightModel, globalLights[i].Position);
			lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Escala pequeña
			phonIlumShader->setMat4("model", lightModel);

			lightDummy->Draw(*phonIlumShader);
		}
	}
	glUseProgram(0);

	// MONO (CASA)
	{
		phonIlumShader->use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		phonIlumShader->setMat4("projection", projection);
		phonIlumShader->setMat4("view", view);
		phonIlumShader->setVec3("eye", camera.Position);

		// Los materiales ahora se aplican automáticamente por mesh dentro de Model::Draw()

		// luces
		UploadLightsMax4(phonIlumShader, globalLights);

		// model
		glm::mat4 monsterHouseModel = glm::mat4(1.0f);
		monsterHouseModel = glm::rotate(monsterHouseModel, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		phonIlumShader->setMat4("model", monsterHouseModel);

		monsterHouse->Draw(*phonIlumShader);
	}

	glUseProgram(0);
	// glfw: swap buffers 
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	return true;
}

// Procesamos entradas del teclado
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

// glfw: Actualizamos el puerto de vista si hay cambios del tamaño
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Callback del movimiento y eventos del mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: Complemento para el movimiento y eventos del mouse
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
