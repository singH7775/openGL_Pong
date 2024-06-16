#include "pong.h"

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
        "uniform float npcPaddlePos;\n"
        "void main() {\n"
        "   if (aPos.x < -0.8) {\n"
        "       gl_Position = vec4(aPos.x, aPos.y + player_y_dir, aPos.z, 1.0f);\n"
        "   } else if (aPos.x > 0.8) {\n"
        "       gl_Position = vec4(aPos.x, aPos.y + npcPaddlePos, aPos.z, 1.0f);\n"
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

    // Creating NPC paddle
    float npcPaddleWidth = 0.02f;
    float npcPaddleHeight = 0.2f;
    float npcPaddlePosX = 0.95f - npcPaddleWidth;
    float npcPaddlePosY = 0.0f;

    float npcPaddleVertices[] = {
    npcPaddlePosX, npcPaddlePosY - npcPaddleHeight / 2, 0.0f,
    npcPaddlePosX, npcPaddlePosY + npcPaddleHeight / 2, 0.0f,
    npcPaddlePosX + npcPaddleWidth, npcPaddlePosY - npcPaddleHeight / 2, 0.0f,
    npcPaddlePosX + npcPaddleWidth, npcPaddlePosY + npcPaddleHeight / 2, 0.0f
    };

    unsigned int npcPaddleVAO, npcPaddleVBO;
    glGenVertexArrays(1, &npcPaddleVAO);
    glGenBuffers(1, &npcPaddleVBO);

    glBindVertexArray(npcPaddleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, npcPaddleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(npcPaddleVertices), npcPaddleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // Creating game ball
    const int numSegments = 500;
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

    // Text shaders
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

            // Update NPC paddle position
            updateNPCPaddlePosition(&npcPaddlePosY, ballPOSy, npcPaddleHeight);

            // Draw NPC paddle
            int npcPaddlePosLocation = glGetUniformLocation(shaderProgram, "npcPaddlePos");
            glUniform1f(npcPaddlePosLocation, npcPaddlePosY);
            glBindVertexArray(npcPaddleVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			// Update ball uniform position
			int ballPosLocation = glGetUniformLocation(shaderProgram, "ballPos");
			glUniform2f(ballPosLocation, ballPOSx, ballPOSy);

			// Draw ball
			glBindVertexArray(circleVAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments);

			updateBallPosition(&ballPOSx, &ballPOSy, &xSpeed, &ySpeed, radius, y_dir);
            handleBallWallCollision(&ballPOSx, &ballPOSy, &xSpeed, &ySpeed, &player1_score, &player2_score, radius, npcPaddlePosY, npcPaddleWidth, npcPaddleHeight);
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
    glDeleteBuffers(1, &npcPaddleVBO);
    glDeleteVertexArrays(1, &npcPaddleVAO);
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