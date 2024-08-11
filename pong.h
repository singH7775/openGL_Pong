#ifndef PONG_H
#define PONG_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define M_PI 3.14159265358979323846
#define SPEED 0.0035
#define NPC_PADDLE_SPEED 0.05f
#define PLAYER_PADDLE_SPEED 0.01

extern float windowWidth; 
extern float windowHeight;
extern float y_dir;
extern unsigned int textShaderProgram;
extern GLuint textTexture;
extern FT_Library ft;
extern FT_Face face;
extern GLuint textVAO, textVBO;

void readKeyboard(GLFWwindow *window, float *y_direction);
void handleGameOver(int player1_score, int player2_score, bool* gameOver);
void displayGameOverMessage(int winningPlayer);
void handlePlayAgainButton(GLFWwindow* window, int* player1_score, int* player2_score, bool* gameOver, float* ballPOSx, float* ballPOSy);
void updateBallPosition(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, float radius, float y_dir);
void updateNPCPaddlePosition(float* npcPaddlePosY, float ballPosY, float npcPaddleHeight);
void handleBallWallCollision(float *ballPOSx, float *ballPOSy, float *xSpeed, float *ySpeed, int *player1_score, int *player2_score, float radius, float npcPaddlePosY, float npcPaddleWidth, float npcPaddleHeight);
void renderText(const char *text, float x, float y, float scale);
void framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH);

#endif