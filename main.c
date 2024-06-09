#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define SPEED 0.0005

void framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH)
{
    glViewport(0, 0, fbW, fbH);
}

void readKeyboard(GLFWwindow *window, float *y_direction);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create Window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Pong", NULL, NULL); 
    if (!window) {
        printf("Failed to create window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    // Loading GLAD
    gladLoadGL();
    //glViewport(0, 0, 800, 600);

    const char* vertexShaderSrc = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform float y_dir;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y + y_dir, aPos.z, 1.0f);\n"
        "}\0";

    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); \n"
        "}\0";

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSrc, 0);
        glCompileShader(vertexShader);
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, 0, infoLog);
            printf("Failure in compiling vertex shader : %s\n", infoLog);
        }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, 0);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, 0, infoLog);
        printf("Failure in compiling fragment shader : %s\n,", infoLog);
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, 0, infoLog);
        printf("Failed to link to the shader : %s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
    -0.95f, -0.1f, 0.0f, // bottom left
    -0.95f,  0.1f, 0.0f, // top left
    -0.90f, -0.1f, 0.0f, // bottom right
    -0.90f,  0.1f, 0.0f  // top right
    };

    unsigned int indices[] = {
        1, 0, 2,
        1, 2, 3
    };

    // created vertex array and buffer object
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);


    // Fill the data and enable
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    float y_dir = 0.0f;

    // keep window open unless signaled otherwise
    while (!glfwWindowShouldClose(window)) {

        // Setting color of window and swapping the buffers to display
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        // read keyboard input
        readKeyboard(window, &y_dir);

        glUseProgram(shaderProgram);
        int y_dirLocation = glGetUniformLocation(shaderProgram, "y_dir");
        glUniform1f(glGetUniformLocation(shaderProgram, "y_dir"), y_dir);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void readKeyboard(GLFWwindow *window, float *y_direction)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && *y_direction < 0.88f) {
        *y_direction += SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && *y_direction > -0.88f) {
        *y_direction -= SPEED;
    }
}