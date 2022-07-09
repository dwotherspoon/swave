#include <renderer/renderer.h>

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void renderer_key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    printf("INFO: Key %i, action %i\n", key, action);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

int renderer_test() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(640, 480, "SWave GL Test", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        printf("ERROR: Failed to create GLFW window.\n");
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, renderer_key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("ERROR: Couldn't initialise GLAD.\n");
        return -1;
    }


    // Define the viewport dimensions
    glViewport(0, 0, 640, 480);

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    printf("INFO: GLFW verison is %i.%i.%i\n", major, minor, rev);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminates GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}
