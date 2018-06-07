//Vertex Shader
#version 430 core

attribute vec4 position;

void main(){
    gl_Position = position;
}




//Fragment Shader
#version 430 core

void main(){
    gl_FragColor = vec4(1.0,0,0,1);
}




//CPP source code
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#include "inc/GLEW/glew.h"
#include "inc/GLFW/glfw3.h"


using namespace std;


class Shader {
    void readShader(GLuint id, string file) {
        ifstream ifs(file);
        stringstream ss;
        ifs >> ss.rdbuf();
        file = ss.str();
        const char *shader = file.data();
        glShaderSource(id, 1, &shader, NULL);

    }

    void compileShader(GLuint id, string type) {
        glCompileShader(id);

        GLint comp;
        glGetShaderiv(id, GL_COMPILE_STATUS, &comp);
        if (comp == GL_FALSE) {
            cout << type << " Shader Compilation FAILED" << endl;
            GLchar messages[256];
            glGetShaderInfoLog(id, sizeof(messages), 0, &messages[0]);
            cout << messages;
        }
    }


public:
    GLuint sID;
    GLuint arrayID;
    Shader(){
    }

    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/triangleVS.glsl");
        readShader(fID, "../GLSL/triangleFS.glsl");


        //3. COMPILE
        compileShader(vID, "vertex");
        compileShader(fID, "fragment");


        //5. ATTACH SHADERS TO PROGRAM
        glAttachShader(sID, vID);
        glAttachShader(sID, fID);


        //6. LINK PROGRAM
        glLinkProgram(sID);

        //7. CHECK FOR LINKING ERRORS
        GLint linkStatus;
        glGetProgramiv(sID, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            cout << "Shader Linking FAILED" << endl;
            GLchar messages[256];
            glGetProgramInfoLog(sID, sizeof(messages), 0, &messages[0]);
            cout << messages;
        }

        //8. CHECK FOR LINKING STATUS
        glValidateProgram(sID);
        GLint validateStatus;
        glGetProgramiv(sID, GL_VALIDATE_STATUS, &validateStatus);
        cout << "Link: " << linkStatus << "  Validate: " << validateStatus << endl;
        if (linkStatus == GL_FALSE) {
            cout << "Shader Validation FAILED" << endl;
            GLchar messages[256];
            glGetProgramInfoLog(sID, sizeof(messages), 0, &messages[0]);
            cout << messages;
        }



        //9. DELETE SHADER
        glDeleteShader(vID);
        glDeleteShader(fID);
    }


    void assignData() {

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        float triangle[3][2] = {
                {-1, -.5},
                {0,  1},
                {1,  -.5}
        };

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[3][2]), triangle, GL_STATIC_DRAW);

        GLuint positionID;
        positionID = glGetAttribLocation(sID, "position");
        glEnableVertexAttribArray(positionID);
        glVertexAttribPointer(positionID, 2, GL_FLOAT, GL_FALSE, sizeof(float[2]), 0);

        glGenVertexArrays(1, &arrayID);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


    }

    void draw(int width, int height) {
        //Draw Triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

};


int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

    int width = 640, height = 480;
    GLFWwindow *window = glfwCreateWindow(width, height, "triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowPos(window, 350, 170);
    glfwMakeContextCurrent(window);
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cout << glewGetErrorString(err) << endl;
    }

    //读取数据
    Shader shader;
    shader.createShader();
    shader.assignData();


    glViewport(0, 0, width, height);
    glUseProgram(shader.sID);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        shader.draw(width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glUseProgram(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



