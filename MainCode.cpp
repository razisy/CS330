// =======================
// main.cpp  (Scroll = speed only)
// =======================

#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderManager.h"
#include "SceneManager.h"

static const uint32_t SCR_WIDTH = 1280;
static const uint32_t SCR_HEIGHT = 720;
static GLFWwindow* gWindow = nullptr;

static glm::vec3 camPos = glm::vec3(0.0f, 3.0f, 9.0f);
static glm::vec3 camFront = glm::vec3(0.0f, -0.2f, -1.0f);
static glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

static float yaw = -90.0f;
static float pitch = -10.0f;
static float fov = 45.0f;            // fixed (no zoom)
static float moveSpeed = 4.0f;         // mouse wheel changes this only

static bool firstMouse = true;
static double lastX = SCR_WIDTH * 0.5;
static double lastY = SCR_HEIGHT * 0.5;
static bool usePerspective = true;

static ShaderManager* gShader = nullptr;
static SceneManager* gScene = nullptr;

static void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

static void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos);
    lastX = xpos; lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity; yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    front.y = sinf(glm::radians(pitch));
    front.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    camFront = glm::normalize(front);
}

static void scroll_callback(GLFWwindow*, double, double yoffset)
{
    // change speed only (no FOV zoom)
    moveSpeed = glm::clamp(moveSpeed + static_cast<float>(yoffset) * 0.5f, 0.5f, 20.0f);
}

static void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float v = moveSpeed * dt;
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += v * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= v * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= v * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += v * right;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camPos += v * camUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camPos -= v * camUp;

    static bool pWasDown = false;
    bool pDown = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (pDown && !pWasDown) usePerspective = !usePerspective;
    pWasDown = pDown;
}

static bool initGLFW()
{
    if (!glfwInit()) return false;
#ifndef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    return true;
}

static bool initGLEW()
{
    glewExperimental = GL_TRUE;
    GLenum err = glewInit(); glGetError();
    return (err == GLEW_OK);
}

int main()
{
    if (!initGLFW()) return EXIT_FAILURE;

    gWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "7-1 FinalProject and Milestones", nullptr, nullptr);
    if (!gWindow) { glfwTerminate(); return EXIT_FAILURE; }

    glfwMakeContextCurrent(gWindow);
    glfwSetFramebufferSizeCallback(gWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(gWindow, mouse_callback);
    glfwSetScrollCallback(gWindow, scroll_callback);
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!initGLEW()) return EXIT_FAILURE;
    glEnable(GL_DEPTH_TEST);

    gShader = new ShaderManager();
    gShader->LoadShaders("../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl");
    gShader->use();

    gScene = new SceneManager(gShader);
    gScene->PrepareScene();

    // lighting
    gShader->setIntValue("bUseLighting", 1);
    gShader->setVec3Value("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.25f));
    gShader->setVec3Value("dirLight.ambient", glm::vec3(0.12f));
    gShader->setVec3Value("dirLight.diffuse", glm::vec3(0.60f));
    gShader->setVec3Value("dirLight.specular", glm::vec3(0.70f));
    gShader->setVec3Value("pointLight.position", glm::vec3(6.0f, 2.3f, 1.0f));
    gShader->setVec3Value("pointLight.ambient", glm::vec3(0.05f, 0.04f, 0.03f));
    gShader->setVec3Value("pointLight.diffuse", glm::vec3(0.95f, 0.82f, 0.60f));
    gShader->setVec3Value("pointLight.specular", glm::vec3(0.95f, 0.88f, 0.70f));
    gShader->setFloatValue("pointLight.constant", 1.0f);
    gShader->setFloatValue("pointLight.linear", 0.09f);
    gShader->setFloatValue("pointLight.quadratic", 0.032f);
    gShader->setFloatValue("material.shininess", 32.0f);

    float lastTime = static_cast<float>(glfwGetTime());
    while (!glfwWindowShouldClose(gWindow))
    {
        float now = static_cast<float>(glfwGetTime());
        float dt = now - lastTime;
        lastTime = now;

        processInput(gWindow, dt);

        glClearColor(0.07f, 0.09f, 0.15f, 1.0f);  // dark blue-gray
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glm::mat4 proj = usePerspective
            ? glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f)
            : glm::ortho(-8.0f, 8.0f, -4.5f, 4.5f, 0.1f, 100.0f);

        gShader->use();
        gShader->setMat4Value("view", view);
        gShader->setMat4Value("projection", proj);
        gShader->setVec3Value("viewPos", camPos);

        gScene->RenderScene();

        glfwSwapBuffers(gWindow);
        glfwPollEvents();
    }

    delete gScene;  delete gShader;
    glfwTerminate();
    return EXIT_SUCCESS;
}
