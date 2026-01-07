module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

export module window;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

export class Window {
public:
    GLFWwindow* handle;
    int width, height;

    Window(int w, int h, const char* title) : width(w), height(h) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(-1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        handle = glfwCreateWindow(width, height, title, NULL, NULL);
        if (handle == NULL) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(-1);
        }

        glfwMakeContextCurrent(handle);
        glfwSetFramebufferSizeCallback(handle, framebuffer_size_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            exit(-1);
        }
        
        glViewport(0, 0, width, height);
    }

    ~Window() {
        glfwDestroyWindow(handle);
        glfwTerminate();
    }

    bool isOpen() {
        return !glfwWindowShouldClose(handle);
    }

    void swapBuffers() {
        glfwSwapBuffers(handle);
    }

    void pollEvents() {
        glfwPollEvents();
    }

    void processInput() {
        if (glfwGetKey(handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(handle, true);
    }
};