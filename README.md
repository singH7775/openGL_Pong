Basic little pong game I created using C, OpenGL graphics API, and other libraries such as GLFW and FreeType

Instructions to run this program (Note - I have built and ran this in vscode) : 

Before you begin, ensure you have the following installed on your system:

GCC compiler
Git
Visual Studio Code (or any preferred C/C++ IDE)

Dependencies
This project requires the following libraries:

-GLFW
-GLAD
-FreeType

Setup Instructions

1.Clone the repository:

git clone https://github.com/singH7775/openGL_Pong
cd opengl-pong

2.Install required libraries 

(Ubuntu/Debian):
sudo apt-get update
sudo apt-get install libglfw3-dev libfreetype6-dev

(Windows/Mac):
https://www.glfw.org/download
https://freetype.org/download.html

3.Set up GLAD:

Download GLAD from https://glad.dav1d.de/
Generate GLAD files with the following settings:

Language: C/C++
Specification: OpenGL
API: gl (Version 3.3 or higher)
Profile: Core

Extract the downloaded files
Copy glad.c to your project's src directory (Already in repo)
Copy glad and KHR folders to your project's include directory


4.Prepare a font file:

Copy a TrueType font file (e.g., Arial.ttf as I have used in my program) to a fonts directory in your project

5. Open up and IDE of your choice and link to these libraries, for example in my case I would be using the c_cpp_properties.json and tasks.json

c_cpp_properties.json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include",
                "/usr/local/include"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-x64"
        }
    ],
    "version": 4
}

tasks.json
{
    "version": "2.0.0",
    "tasks": [
        {
            "type": "shell",
            "label": "gcc build active file",
            "command": "/usr/bin/gcc",
            "args": [
                "-g",
                "${workspaceFolder}/main.c",
                "${workspaceFolder}/pong.c",
                "${workspaceFolder}/src/glad.c",
                "-I${workspaceFolder}/include",
                "-I/usr/include/freetype2",
                "-lglfw",
                "-lGL",
                "-lm",
                "-ldl",
                "-lfreetype",
                "-o",
                "${workspaceFolder}/pong"
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}

Make sure to have glad.c included in the project directory and run your build task.

Notes
- There may be a chance that the ball may be too slow, or the player paddles may be to slow, you can change the speeds to your liking with the constant defined in the pong.h file named NPC_PADDLE_SPEED and PLAYER_PADDLE_SPEED
- During the build process, the compiler may spit out that the fonts are not loaded. Make sure you have a fonts path, for example in my enviorment my fonts are placed "/home/har/myFonts/ariali.ttf", you can edit this on line 207 of main.c
- Any othe complilation errors, just make sure you have all libraries installed and linked and are using a c/c++ compiler

contact : harsingh10000@gmail.com / discord : dark691400