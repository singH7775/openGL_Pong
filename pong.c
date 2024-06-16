#include "pong.h"

float windowWidth = 1400, windowHeight = 800;
float y_dir = 0.0f;
unsigned int textShaderProgram;
GLuint textTexture;
FT_Library ft;
FT_Face face;
GLuint textVAO, textVBO;

void readKeyboard(GLFWwindow *window, float *y_direction)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && *y_direction < 0.9f) {
        *y_direction += 0.0002;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && *y_direction > -0.9f) {
        *y_direction -= 0.0002;
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
    static float lastHitTime = 0.0f;
    float currentTime = glfwGetTime(); // Ensuring no multiple hits in rapid succession

    if (*ballPOSx < -0.93f && *ballPOSx > -0.95f && *ballPOSy < (y_dir + 0.1f) && *ballPOSy > (y_dir - 0.1f)) {
        if (currentTime - lastHitTime >= 0.2f) {
            *xSpeed = -*xSpeed * 1.2f;
            lastHitTime = currentTime;
        }
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

void updateNPCPaddlePosition(float* npcPaddlePosY, float ballPosY, float npcPaddleHeight) {
    float paddleCenter = *npcPaddlePosY;
    float ballCenter = ballPosY;

    float distance = ballCenter - paddleCenter;
    float speed = distance * 0.0010f; // Adjust the multiplier to control the paddle speed

    *npcPaddlePosY += speed;

    // Limit the NPC paddle position within the window bounds
    if (*npcPaddlePosY > 1.0f - npcPaddleHeight / 2) {
        *npcPaddlePosY = 1.0f - npcPaddleHeight / 2;
    } else if (*npcPaddlePosY < -1.0f + npcPaddleHeight / 2) {
        *npcPaddlePosY = -1.0f + npcPaddleHeight / 2;
    }
}

void handleBallWallCollision(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, int *player1_score, int *player2_score, float radius, float npcPaddlePosY, float npcPaddleWidth, float npcPaddleHeight) {
    if (*ballPOSx > (0.95f - npcPaddleWidth - radius) && *ballPOSx < (0.95f - radius) &&
        *ballPOSy > (npcPaddlePosY - npcPaddleHeight / 2) && *ballPOSy < (npcPaddlePosY + npcPaddleHeight / 2)) {
        *xSpeed = -*xSpeed * 1.2f; // Increase ball speed on hit
    }

    // Score incrementing
    if (*ballPOSx > (1.0f - radius) || *ballPOSx < -(1.0f - radius)) {
        if (*ballPOSx > (1.0f - radius)) {
            (*player1_score)++;
        } else if (*ballPOSx < -(1.0f - radius)) {
            (*player2_score)++;
        }

        *ballPOSx = 0.0f;
        *ballPOSy = 0.0f;

        // Shoot the ball at angle that is not between 80/100 or 260/280
        float initialSpeed = SPEED;
        float angle;
        do {
            angle = ((float)rand() / RAND_MAX) * 2 * M_PI;
        } while ((angle >= (80.0f * M_PI / 180.0f) && angle <= (100.0f * M_PI / 180.0f)) ||
                (angle >= (260.0f * M_PI / 180.0f) && angle <= (280.0f * M_PI / 180.0f)));
        
        *xSpeed = initialSpeed * cos(angle);
        *ySpeed = initialSpeed * sin(angle);
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