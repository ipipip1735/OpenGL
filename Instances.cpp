//Vertex Shader
#version 420 core

layout(location = 1) attribute vec2 position;
layout(location = 2) attribute vec3 color;
layout(location = 3) attribute mat4 offset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 vertexColor;

void main(){
    gl_Position = projection * view * offset * vec4(position, 0, 1);
    vertexColor = vec4(color,1);
}







//Fragment Shader
#version 420 core

in vec4 vertexColor;

void main(){
    gl_FragColor = vertexColor;
}





//CPP source code
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#define GLM_FORCE_RADIANS

#include "inc/GLEW/glew.h"
#include "inc/GLFW/glfw3.h"
#include "lib/GLM/gtc/type_ptr.hpp"
#include "lib/GLM/gtc/matrix_transform.hpp"

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
    GLuint tID1, tID2;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);

        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/instanceVS.glsl");
        readShader(fID, "../GLSL/instanceFS.glsl");

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

        float triangle[4][5] = {
                {-0.5, 0.5,  1, 0, 0},
                {-0.5, -0.5, 0, 1, 0},
                {0.5,  -0.5, 0, 0, 1},
                {0.5,  0.5,  1, 0, 0},
        };

        GLubyte index[6] = {
                0, 1, 2,
                2, 0,3};


        int n = 0;
        glm::mat4 offset[100];

        for (int i = 0; i < 10; ++i) {

            for (int j = 0; j < 10; ++j) {
                offset[n++] = glm::translate(glm::mat4(1), glm::vec3(i, j, 0));
            }
        }

        //绑定VAO
        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);


        //配置EBO
        GLuint elementID;
        glGenBuffers(1, &elementID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);



        //配置VBO，保存前2个属性（position和color）的值
        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[4][5]), triangle, GL_STATIC_DRAW);
        //配置VAO，指定前2个属性（position和color）的数据结构
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[5]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float[5]), (void *) sizeof(float[2]));



        //再配置一个VBO，保存offset属性的值（4X4矩阵消耗4个Location）
        GLuint instanceID;
        glGenBuffers(1, &instanceID);
        glBindBuffer(GL_ARRAY_BUFFER, instanceID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(offset), offset, GL_STATIC_DRAW);
        //配置VAO，指定4X4矩阵的数据结构
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(float[16]), 0);
        glVertexAttribDivisor(3, 1); //每实例更新一次数据

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(float[16]), (void *) (sizeof(float[4]) * 1));
        glVertexAttribDivisor(4, 1); //每实例更新一次数据

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(float[16]), (void *) (sizeof(float[4]) * 2));
        glVertexAttribDivisor(5, 1); //每实例更新一次数据

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(float[16]), (void *) (sizeof(float[4]) * 3));
        glVertexAttribDivisor(6, 1); //每实例更新一次数据



        //解绑VAO
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }


    void draw(int width, int height) {

        //view变换
//        glm::mat4 view = glm::lookAt(glm::vec3(0,0, 5.0f), glm::vec3(0,0, 0), glm::vec3(0, 1, 0));
        glm::mat4 view = glm::lookAt(glm::vec3(4.5, 4.7, 10.0f), glm::vec3(4.5, 4.7, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));


        //投影变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));


        //model变换
        glm::mat4 model(1);
        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));


        //绘制数组（不使用实例绘制）
//        glDrawArrays(GL_TRIANGLES, 0, 3);
        //绘制100个数组实例，即绘制100个三角形
        glDrawArraysInstanced(GL_TRIANGLES, 1, 3, 100);


        //绘制索引（不使用实例绘制）
//        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        //绘制100个实例索引，即绘制100个正方形（EBO中有6个顶点，即2个三角形构成的正方形）
//        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, 100);




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


    Shader shader;
    shader.createShader();
    shader.assignData();

    glViewport(0, 0, width, height);
    //Bind Shader and Vertex Array Object
    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        shader.draw(width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Unbind Vertex Array Object and Shader
    glBindVertexArray(0);
    glUseProgram(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
