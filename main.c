#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define M_PI 3.14159265358979323846
#define SPEED 0.0002

float windowWidth = 1400, windowHeight = 800;
float y_dir = 0.0f;
unsigned int textShaderProgram;
GLuint textTexture;
FT_Library ft;
FT_Face face;
GLuint textVAO, textVBO;

void readKeyboard(GLFWwindow *window, float *y_direction);
void handleGameOver(int player1_score, int player2_score, bool* gameOver);
void displayGameOverMessage(int winningPlayer);
void handlePlayAgainButton(GLFWwindow* window, int* player1_score, int* player2_score, bool* gameOver, float* ballPOSx, float* ballPOSy);
void updateBallPosition(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, float radius, float y_dir);
void handleBallWallCollision(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, int *player1_score, int *player2_score, float radius);
void renderText(const char *text, float x, float y, float scale);
void framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH);

int main()
{
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create Window
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Pong", NULL, NULL); 
    if (!window) {
        printf("Failed to create window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    // Loading GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

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
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
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
    do {
        angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    } while ((angle >= (80.0f * M_PI / 180.0f) && angle <= (100.0f * M_PI / 180.0f)) ||
            (angle >= (260.0f * M_PI / 180.0f) && angle <= (280.0f * M_PI / 180.0f)));

    float randomDirection = (float)rand() / RAND_MAX;
    if (randomDirection < 0.5f) {
        xSpeed = -SPEED * cos(angle);
        ySpeed = -SPEED * sin(angle);
    } else {
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

    if (FT_Init_FreeType(&ft))
    {
        fprintf(stderr, "Could not init FreeType Library\n");
        return -1;
    }

    if(FT_New_Face(ft, "C:/myFonts/ariali.ttf", 0, &face))
    {
        fprintf(stderr, "Failed to load font\n");
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 36);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Ensure pixel storage mode is set

    glGenTextures(1, &textTexture);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const char *textVertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec4 vertex;\n"
        "out vec2 TexCoords;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "       gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
        "       TexCoords = vertex.zw;\n"
        "}\0";

    const char *textFragmentShaderSource = "#version 330 core\n"
        "in vec2 TexCoords;\n"
        "out vec4 color;\n"
        "uniform sampler2D text;\n"
        "void main() {\n"
        "       vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
        "       color = vec4(1.0, 1.0, 1.0, 1.0) * sampled;\n"
        "}\0";
    
    unsigned int textVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(textVertexShader);
    glGetShaderiv(textVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, 0, infoLog);
        printf("Failure in compiling vertex shader : %s\n", infoLog);
    }

    unsigned int textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(textFragmentShader);
    glGetShaderiv(textFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, 0, infoLog);
        printf("Failure in compiling vertex shader : %s\n", infoLog);
    }

    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVertexShader);
    glAttachShader(textShaderProgram, textFragmentShader);
    glLinkProgram(textShaderProgram);
    glGetProgramiv(textShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
    glGetProgramInfoLog(textShaderProgram, 512, NULL, infoLog);
    printf("Failed to link text shader program: %s\n", infoLog);
    }

    // Create VAO and VBO for rendering text
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteShader(textVertexShader);
    glDeleteShader(textFragmentShader);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    bool gameOver = false;
    
    // keep window open unless signaled otherwise
    while (!glfwWindowShouldClose(window)) {
		// Setting color of window and swapping the buffers to display
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT);

		char score_player[20];
		char score_player2[20];
		static int player1_score = 0;
		static int player2_score = 0;

		handleGameOver(player1_score, player2_score, &gameOver);

		if (!gameOver) {
			// read keyboard input
			readKeyboard(window, &y_dir);

			// Display scores
			snprintf(score_player, sizeof(score_player), "%d", player1_score);
			snprintf(score_player2, sizeof(score_player2), "%d", player2_score);

			float score_playerX = 60.0f;
			float score_player2X = windowWidth - 100.0f; 
			renderText(score_player, score_playerX, windowHeight - 50.0f, 1.0f);
			renderText(score_player2, score_player2X, windowHeight - 50.0f, 1.0f);

			// Target score
			const char* winningScore = "Target Score: 6";
			float winningScoreX = (windowWidth - strlen(winningScore) * 20) / 2;
			renderText(winningScore, winningScoreX, windowHeight - 100.0f, 1.0f);

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

			updateBallPosition(&ballPOSx, &ballPOSy, &xSpeed, &ySpeed, radius, y_dir);
            handleBallWallCollision(&ballPOSx, &ballPOSy, &xSpeed, &ySpeed, &player1_score, &player2_score, radius);
		} else {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                handlePlayAgainButton(window, &player1_score, &player2_score, &gameOver, &ballPOSx, &ballPOSy);
            }
        }

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    // Cleaning up the game
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(textShaderProgram);
    glDeleteTextures(1, &textTexture);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

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

void handleGameOver(int player1_score, int player2_score, bool* gameOver) {
    static int winningPlayer = 0;
    if (!*gameOver && (player1_score >= 6 || player2_score >=6)) {
        *gameOver = true;
        winningPlayer = (player1_score >= 6) ? 1 : 2;
    }

    if (*gameOver) {
        displayGameOverMessage(winningPlayer);
    }
}

void displayGameOverMessage(int winningPlayer) {
    char gameOverText[20];
    snprintf(gameOverText, sizeof(gameOverText), "Player %d wins!", winningPlayer);
    float gameOverTextX = (windowWidth - strlen(gameOverText) * 20) / 2;
    float gameOverTextY = windowHeight / 2;
    renderText(gameOverText, gameOverTextX, gameOverTextY, 1.5f);

    // Displaying play again button
    const char* playAgainText = "Play Again";
    float playAgainTextWidth = strlen(playAgainText) * 20;
    float playAgainTextX = (windowWidth - playAgainTextWidth) / 2;
    float playAgainTextY = gameOverTextY - 50.0f;
    renderText(playAgainText, playAgainTextX, playAgainTextY, 1.0f);
}

void handlePlayAgainButton(GLFWwindow* window, int* player1_score, int* player2_score, bool* gameOver, float* ballPOSx, float* ballPOSy) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Convert mouse coordinates to OpenGL coordinates
    float openglX = (float)mouseX;
    float openglY = windowHeight - (float)mouseY;

    // Check if the mouse click is within the button area
    const char* playAgainText = "Play Again";
    float playAgainTextWidth = strlen(playAgainText) * 20;
    float playAgainTextX = (windowWidth - playAgainTextWidth) / 2;
    float playAgainTextY = windowHeight / 2 - 50.0f;
    float buttonWidth = playAgainTextWidth;
    float buttonHeight = 40.0f;

    if (openglX >= playAgainTextX && openglX <= playAgainTextX + buttonWidth &&
        openglY >= playAgainTextY && openglY <= playAgainTextY + buttonHeight) {
        // Reset game state
        *player1_score = 0;
        *player2_score = 0;
        *gameOver = false;
        *ballPOSx = 0.0f;
        *ballPOSy = 0.0f;
    }
}

void updateBallPosition(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, float radius, float y_dir) {
    if (*ballPOSx < -0.93f && *ballPOSx > -0.95f && *ballPOSy < (y_dir + 0.1f) && *ballPOSy > (y_dir - 0.1f)) {
        *xSpeed = -*xSpeed;
    }

    // Update ball position
    *ballPOSx += *xSpeed;
    *ballPOSy += *ySpeed;

    // Creating boundary for the ball
    if (*ballPOSy > (1.0f - radius)) {
        *ballPOSy = 1.0f - radius;
        *ySpeed = -*ySpeed;
    }
    else if (*ballPOSy < -(1.0f - radius)) {
        *ballPOSy = -(1.0f - radius);
        *ySpeed = -*ySpeed;
    }
}

void handleBallWallCollision(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, int *player1_score, int *player2_score, float radius) {
    if (*ballPOSx > (1.0f - radius) || *ballPOSx < -(1.0f - radius)) {
        if (*ballPOSx > (1.0f - radius)) {
            (*player1_score)++;
        } else if (*ballPOSx < -(1.0f - radius)) {
            (*player2_score)++;
        }

        *ballPOSx = 0.0f;
        *ballPOSy = 0.0f;

        // Shoot the ball at angle that is not between 80/100 or 260/280
        float angle;
        do {
            angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
        } while ((angle >= (80.0f * M_PI / 180.0f) && angle <= (100.0f * M_PI / 180.0f)) ||
                (angle >= (260.0f * M_PI / 180.0f) && angle <= (280.0f * M_PI / 180.0f)));
        
        *xSpeed = SPEED * cos(angle);
        *ySpeed = SPEED * sin(angle);
    }
}

void renderText(const char *text, float x, float y, float scale) {
    // Activate the text shader program
    glUseProgram(textShaderProgram);
    glUniform1i(glGetUniformLocation(textShaderProgram, "text"), 0);

    // Set the projection matrix
    float projection[16] = {
        2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / windowHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, projection);

    // Bind the VAO for rendering the text quads
    glBindVertexArray(textVAO);

    // Iterate over each character in the text string
    for (const char *p = text; *p; p++) {
        // Load the glyph for the character
        if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
            continue;

        // Generate a texture for the glyph bitmap
        glBindTexture(GL_TEXTURE_2D, textTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // Calculate vertex and texture coordinates
        GLfloat xpos = x + face->glyph->bitmap_left * scale;
        GLfloat ypos = y - (face->glyph->bitmap.rows - face->glyph->bitmap_top) * scale;
        GLfloat w = face->glyph->bitmap.width * scale;
        GLfloat h = face->glyph->bitmap.rows * scale;

        GLfloat vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos,     ypos,     0.0f, 1.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        // Render the glyph texture over the quad
        glBindTexture(GL_TEXTURE_2D, textTexture);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance cursors for the next glyph
        x += (face->glyph->advance.x >> 6) * scale;
    }

    // Unbind the VAO and shader program
    glBindVertexArray(0);
    glUseProgram(0);
}

void framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH)
{
    glViewport(0, 0, fbW, fbH);
}