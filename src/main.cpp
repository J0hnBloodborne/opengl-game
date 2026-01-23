#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>

import game;
import resource_manager;
import window;

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

Game* Breakout;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

int main(int argc, char *argv[])
{
    Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout");

    glfwSetKeyCallback(window.handle, key_callback);
    glfwSetMouseButtonCallback(window.handle, mouse_button_callback);
    glfwSetCursorPosCallback(window.handle, cursor_position_callback);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Breakout = new Game(SCREEN_WIDTH, SCREEN_HEIGHT);
    Breakout->Init();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    Breakout->State = GAME_MENU;

    while (window.isOpen())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        window.pollEvents();
        Breakout->ProcessInput(deltaTime);
        Breakout->Update(deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Breakout->Render();

        window.swapBuffers();
    }

    ResourceManager::Clear();
    delete Breakout;

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            Breakout->Keys[key] = true;
        else if (action == GLFW_RELEASE)
            Breakout->Keys[key] = false;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button >= 0 && button < 3)
    {
        if (action == GLFW_PRESS)
            Breakout->MouseButtons[button] = true;
        else if (action == GLFW_RELEASE) {
            Breakout->MouseButtons[button] = false;
            Breakout->MouseProcessed[button] = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Breakout->MousePos = glm::vec2((float)xpos, (float)ypos);
}
