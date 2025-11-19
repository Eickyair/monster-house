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
Shader* basicShader;
// NUEVO: un shader por objeto (mismo VS/FS, instancias distintas)
Shader* monoShader;
Shader* toroideShader;
Shader* geoShader;

// Carga la información del modelo
Model* lightDummy;
Model* mono, * toroide, * geodesica;

// Cubemap
CubeMap* mainCubeMap;

// Materiales
Material material;

// Luces base y subconjuntos por objeto
std::vector<Light> gLights;
std::vector<Light> lightsMono;
std::vector<Light> lightsToroide;
std::vector<Light> lightsGeodesica;

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
	monoShader = new Shader(VS, FS);
	toroideShader = new Shader(VS, FS);
	geoShader = new Shader(VS, FS);

	basicShader = new Shader("shaders/10_vertex_simple.vs", "shaders/10_fragment_simple.fs");
	cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");

	// Modelos
	mono = new Model("models/mono.fbx");
	toroide = new Model("models/toroide_mod.fbx");
	geodesica = new Model("models/geodesica.fbx");
	lightDummy = new Model("models/IllumModels/lightDummy.fbx");

	// Cubemap
	std::vector<std::string> faces{
		"textures/cubemap/01/posx.png",
		"textures/cubemap/01/negx.png",
		"textures/cubemap/01/posy.png",
		"textures/cubemap/01/negy.png",
		"textures/cubemap/01/posz.png",
		"textures/cubemap/01/negz.png"
	};
	mainCubeMap = new CubeMap();
	mainCubeMap->loadCubemap(faces);

	// Configuración de luces (4 luces base)
	{
		Light light01;
		light01.Position = glm::vec3(0.0f, 2.0f, 1.0f);
		light01.Color = glm::vec4(0.1f, .9f, .01f, 1.0f);
		light01.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
		light01.alphaIndex = 30;
		gLights.push_back(light01);

		Light light02;
		light02.Position = glm::vec3(1.0f, 1.0f, 2.0f);
		light02.Color = glm::vec4(0.2f, .2f, .1f, 1.0f);
		light02.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
		light02.alphaIndex = 50;

		gLights.push_back(light02);

		Light light03;
		light03.Position = glm::vec3(5.0f, 2.0f, -5.0f);
		light03.Color = glm::vec4(0.1f, .1f, .1f, 1.0f);
		light03.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
		light01.alphaIndex = 75;
		gLights.push_back(light03);

		Light light04;
		light04.Position = glm::vec3(-5.0f, 2.0f, -5.0f);
		light04.Color = glm::vec4(0.1f, .1f, .1f, 1.0f);
		light04.Power = glm::vec4(60.0f, 60.0f, 60.0f, 1.0f);
		gLights.push_back(light04);
	}

	// Subconjuntos
	lightsMono = { gLights[0] };
	lightsToroide = { gLights[1] };
	lightsGeodesica = { gLights[2] };

	// Material
	material.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	material.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	material.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	material.transparency = 1.0f;

	// SoundEngine->play2D("sound/EternalGarden.mp3", true);

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

	// Clear
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// Cubemap
	{
		mainCubeMap->drawCubeMap(*cubemapShader, projection, view);
	}

	// ---------- DIBUJO POR OBJETO: SHADER + LUCES PROPIAS (max 4) ----------

	// MONO
	{
		monoShader->use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		monoShader->setMat4("projection", projection);
		monoShader->setMat4("view", view);
		monoShader->setVec3("eye", camera.Position);

		// material de plastico
		material.ambient = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);
		material.diffuse = glm::vec4(0.02f, 0.02f, 0.02f, 1.0f);
		material.specular = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);

		monoShader->setVec4("MaterialAmbientColor", material.ambient);
		monoShader->setVec4("MaterialDiffuseColor", material.diffuse);
		monoShader->setVec4("MaterialSpecularColor", material.specular);
		monoShader->setFloat("transparency", material.transparency);

		// luces (solo las de mono, máximo 4)
		UploadLightsMax4(monoShader, lightsMono);

		// model
		glm::mat4 modelMono = glm::mat4(1.0f);
		modelMono = glm::rotate(modelMono, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		monoShader->setMat4("model", modelMono);

		mono->Draw(*monoShader);
	}

	// TOROIDE
	{
		toroideShader->use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		toroideShader->setMat4("projection", projection);
		toroideShader->setMat4("view", view);
		toroideShader->setVec3("eye", camera.Position);
		// material de oro


		material.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
		material.diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
		material.specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
		toroideShader->setVec4("MaterialAmbientColor", material.ambient);
		toroideShader->setVec4("MaterialDiffuseColor", material.diffuse);
		toroideShader->setVec4("MaterialSpecularColor", material.specular);
		toroideShader->setFloat("transparency", material.transparency);

		UploadLightsMax4(toroideShader, lightsToroide);

		glm::mat4 modelTor = glm::mat4(1.0f);
		modelTor = glm::translate(modelTor, glm::vec3(3.0f, 0.0f, 0.0f));
		modelTor = glm::rotate(modelTor, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		toroideShader->setMat4("model", modelTor);

		toroide->Draw(*toroideShader);
	}

	// GEODÉSICA
	{
		geoShader->use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		geoShader->setMat4("projection", projection);
		geoShader->setMat4("view", view);
		geoShader->setVec3("eye", camera.Position);


		// CHROME (para geodésica)
		material.ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
		material.diffuse = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
		material.specular = glm::vec4(0.774597f, 0.774597f, 0.774597f, 1.0f);
		geoShader->setVec4("MaterialAmbientColor", material.ambient);
		geoShader->setVec4("MaterialDiffuseColor", material.diffuse);
		geoShader->setVec4("MaterialSpecularColor", material.specular);
		geoShader->setFloat("transparency", material.transparency);

		UploadLightsMax4(geoShader, lightsGeodesica);

		glm::mat4 modelGeo = glm::mat4(1.0f);
		modelGeo = glm::translate(modelGeo, glm::vec3(-3.0f, 0.0f, 0.0f));
		modelGeo = glm::rotate(modelGeo, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		geoShader->setMat4("model", modelGeo);

		geodesica->Draw(*geoShader);
	}

	glUseProgram(0);

	// Indicadores de luces
	{
		basicShader->use();
		basicShader->setMat4("projection", projection);
		basicShader->setMat4("view", view);

		glm::mat4 model;
		for (size_t i = 0; i < gLights.size(); ++i) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, gLights[i].Position);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
			basicShader->setMat4("model", model);
			lightDummy->Draw(*basicShader);
		}
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
