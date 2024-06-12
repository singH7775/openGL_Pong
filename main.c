#include <stdio.h>
#include <math.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define M_PI 3.14159265358979323846
#define SPEED 0.0004
float y_dir = 0.0f;

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
    float windowWidth = 1400, windowHeight = 800;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Pong", NULL, NULL); 
    if (!window) {
        printf("Failed to create window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    // Loading GLAD
    gladLoadGL();

    // Shaders code
    const char* vertexShaderSrc = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform float player_y_dir;\n"
        "uniform vec2 ballPos;\n"
        "void main() {\n"
        "   if (aPos.x < -0.8) {\n"
        "       gl_Position = vec4(aPos.x, aPos.y + player_y_dir, aPos.z, 1.0f);\n"
        "   } else {\n"
        "       gl_Position = vec4(aPos.x + ballPos.x, aPos.y + ballPos.y, aPos.z, 1.0f);\n"
        "   }\n"
        "}\0";

    const char* fragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); \n"
        "}\0";

    // Creating shaders and linking to one program
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

    // Vertices for player 1
    float P1vertices[] = {
    -0.95f, -0.1f, 0.0f, // bottom left
    -0.95f,  0.1f, 0.0f, // top left
    -0.93f, -0.1f, 0.0f, // bottom right
    -0.93f,  0.1f, 0.0f  // top right
    };

    unsigned int indices[] = {
        1, 0, 2,
        1, 2, 3
    };

    // created vertex array / buffer object / element buffer object
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);


    // Fill the data and enable
    glBufferData(GL_ARRAY_BUFFER, sizeof(P1vertices), P1vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Creating game ball
    const int numSegments = 300;
    float circleVertices[numSegments * 3];
    float radius = 0.03f;
    float ballPOSx = 0;
    float ballPOSy = 0;
    float xSpeed = 0;
    float ySpeed = 0;

    // To create random direction for ball in start
    srand(time(NULL));
    float angle;
    float randomDirection = (float)rand() / RAND_MAX;
    if (randomDirection < 0.5f) {
    // Allow the ball to shoot at angles of 240 to 120 (left)
    angle = ((float)rand() / RAND_MAX) * (M_PI * 4 / 3) + (M_PI * 2 / 3);
    xSpeed = -SPEED * cos(angle);
    ySpeed = -SPEED * sin(angle);
    } else {
    // Allow the ball to shoot at angles of 300 to 60 (right)
    angle = ((float)rand() / RAND_MAX) * (M_PI * 2 / 3) - (M_PI * 5 / 3);
    xSpeed = SPEED * cos(angle);
    ySpeed = SPEED * sin(angle);
    }

    // Vertices for the ball
    for (int i = 0; i < numSegments; i++) {
        float angle = i * (2.0f * M_PI / numSegments);
        circleVertices[i * 3] = radius * cos(angle);
        circleVertices[i * 3 + 1] = radius * sin(angle);
        circleVertices[i * 3 + 2] = 0.0f;
    }

    // VA and BO for ball
    unsigned int circleVAO, circleVBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);

    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Load score
    int width, height, nrChannels;
    unsigned char *data = stbi_load("Number1.png", &width, &height, &nrChannels, 0);
    if (data == NULL) {
        printf("Failed to load texture image: Number1.png\n");
    }

    // Determine the correct format based on the number of channels
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    float ScoreVertices[] = {
    // positions        // colors           // texture coords
    0.1f, 0.9f, 0.0f,   1.0f, 0.0f, 0.0f,   0.8f, 0.2f, // top right
    0.1f, 0.7f, 0.0f,   0.0f, 1.0f, 0.0f,   0.8f, 0.8f, // bottom right
    -0.1f, 0.7f, 0.0f,  0.0f, 0.0f, 1.0f,   0.2f, 0.8f, // bottom left
    -0.1f, 0.9f, 0.0f,  1.0f, 1.0f, 0.0f,   0.2f, 0.2f  // top left
    };

    unsigned int ScoreIndices[] = {
    0, 1, 3,   // First triangle
    1, 2, 3    // Second triangle
    };

    // Shaders code for SCORE
    const char* SCOREvertexShaderSrc = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexCoord;\n"
        "out vec3 ourColor;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "       gl_Position = vec4(aPos, 1.0);\n"
        "       ourColor = aColor;\n"
        "       TexCoord = aTexCoord;\n"
        "}\0";

    const char* SCOREfragmentShaderSrc = 
        "#version 330 core\n"
        "out vec4 fragColor;\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {\n"
        "   fragColor = texture(ourTexture, TexCoord);\n"
        "}\0";

    // Creating shaders and linking to a seperate program
    unsigned int SCOREvertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(SCOREvertexShader, 1, &SCOREvertexShaderSrc, NULL);
    glCompileShader(SCOREvertexShader);

    glGetShaderiv(SCOREvertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(SCOREvertexShader, 512, NULL, infoLog);
        printf("Failure in compiling score vertex shader: %s\n", infoLog);
    }

    unsigned int SCOREfragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(SCOREfragmentShader, 1, &SCOREfragmentShaderSrc, NULL);
    glCompileShader(SCOREfragmentShader);

    glGetShaderiv(SCOREfragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(SCOREfragmentShader, 512, NULL, infoLog);
        printf("Failure in compiling score fragment shader: %s\n", infoLog);
    }

    unsigned int SCOREshaderProgram = glCreateProgram();
    glAttachShader(SCOREshaderProgram, SCOREvertexShader);
    glAttachShader(SCOREshaderProgram, SCOREfragmentShader);
    glLinkProgram(SCOREshaderProgram);

    glGetProgramiv(SCOREshaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(SCOREshaderProgram, 512, NULL, infoLog);
        printf("Failed to link the score shader program: %s\n", infoLog);
    }
    
    // Deleting shaders after linking with new program
    glDeleteShader(SCOREvertexShader);
    glDeleteShader(SCOREfragmentShader);

    // Creating vertex array object
    unsigned int ScoreVAO;
    glGenVertexArrays(1, &ScoreVAO);
    glBindVertexArray(ScoreVAO);

    // Buffer object
    unsigned int ScoreVBO;
    glGenBuffers(1, &ScoreVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ScoreVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ScoreVertices), ScoreVertices, GL_STATIC_DRAW);

    // Specifying layout and format of the object
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Element buffer object
    unsigned int ScoreEBO;
    glGenBuffers(1, &ScoreEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ScoreEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ScoreIndices), ScoreIndices, GL_STATIC_DRAW);


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // keep window open unless signaled otherwise
    while (!glfwWindowShouldClose(window)) {

        // Setting color of window and swapping the buffers to display
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        // read keyboard input
        readKeyboard(window, &y_dir);

        // Displaying the score
        glUseProgram(SCOREshaderProgram);
        glUniform1i(glGetUniformLocation(SCOREshaderProgram, "ourTexture"), 0);
        glBindVertexArray(ScoreVAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ScoreEBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Loading shader program
        glUseProgram(shaderProgram);

        // Draw player
        int player_y_dirLocation = glGetUniformLocation(shaderProgram, "player_y_dir");
        glUniform1f(player_y_dirLocation, y_dir);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Update ball uniform position
        int ballPosLocation = glGetUniformLocation(shaderProgram, "ballPos");
        glUniform2f(ballPosLocation, ballPOSx, ballPOSy);

        // Draw circle
        glBindVertexArray(circleVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments);

        glfwSwapBuffers(window);

        // Check for collisions with the player's paddle
        if (ballPOSx < -0.93f && ballPOSx > -0.95f && ballPOSy < (y_dir + 0.1f) && ballPOSy > (y_dir - 0.1f))
        {
            xSpeed = -xSpeed;
        }

        // Update ball pos
        ballPOSx += xSpeed;
        ballPOSy += ySpeed;

        // Making sure ball stays within the game
        if (ballPOSx > (1.0f - radius) || ballPOSx < -(1.0f - radius)) {
            ballPOSx = 0.0f;
            ballPOSy = 0.0f;

            float angle;
            float randomDirection = (float)rand() / RAND_MAX;
            if (randomDirection < 0.5f) {
                // Allow the ball to shoot at angles of 240 to 120 (left)
                angle = ((float)rand() / RAND_MAX) * (M_PI * 4 / 3) + (M_PI * 2 / 3);
                xSpeed = -SPEED * cos(angle);
                ySpeed = -SPEED * sin(angle);
            } else {
                // Allow the ball to shoot at angles of 300 to 60 (right)
                angle = ((float)rand() / RAND_MAX) * (M_PI * 2 / 3) + (M_PI * 5 / 3);
                xSpeed = SPEED * cos(angle);
                ySpeed = SPEED * sin(angle);
            }
        }
        if (ballPOSy > (1.0f - radius)) {
            ballPOSy = 1.0f - radius;
            ySpeed = -ySpeed;
        }
        else if (ballPOSy < -(1.0f - radius)) {
            ballPOSy = -(1.0f - radius);
            ySpeed = -ySpeed;
        }

        glfwPollEvents();

    }

    // Cleaning up the score
    glDeleteVertexArrays(1, &ScoreVAO);
    glDeleteBuffers(1, &ScoreVBO);
    glDeleteTextures(1, &texture);
    glDeleteProgram(SCOREshaderProgram);

    // Cleaning up the game
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void readKeyboard(GLFWwindow *window, float *y_direction)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && *y_direction < 0.9f) {
        *y_direction += SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && *y_direction > -0.9f) {
        *y_direction -= SPEED;
    }
}