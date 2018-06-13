//Vertex Shader
#version 430 core

layout(location = 1) attribute vec2 position;
layout(location = 2) attribute vec2 coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
void main(){
    gl_Position = projection * view * model * vec4(position, 0, 1);
    texCoord = coord;
}





//Fragment Shader
#version 430 core

layout(binding=1) uniform sampler2DArray sampler;
in vec2 texCoord;

out vec4 FragColor;
void main(){
    FragColor = texture(sampler, vec3(texCoord, 0)); //使用第0层

//    FragColor = texture(sampler, vec3(vec2(0,0), 1)); //使用第1层


}






//CPP source code
#include "inc/GLEW/glew.h"
#include "inc/GLFW/glfw3.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#define GLM_FORCE_RADIANS

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
        readShader(vID, "../GLSL/taVS.glsl");
        readShader(fID, "../GLSL/taFS.glsl");


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


        float cube[8][8] = {
                {1,  -1, 1,  0, 0, 1, 0, 0},
                {1,  1,  1,  1, 0, 0, 1, 0},
                {-1, 1,  1,  1, 1, 0, 0, 1},
                {-1, -1, 1,  0, 1, 1, 0, 0},
                {1,  -1, -1, 0, 0, 0, 1, 0},
                {1,  1,  -1, 1, 0, 0, 0, 1},
                {-1, 1,  -1, 1, 1, 1, 0, 0},
                {-1, -1, -1, 0, 1, 0, 1, 0}
        };

        GLubyte index[24] = {
                0, 1, 2, 3, //front
                7, 6, 5, 4, //back
                3, 2, 6, 7, //left
                4, 5, 1, 0, //right
                1, 5, 6, 2, //top
                4, 0, 3, 7}; //bottom


//            //6-------------/5
//          //  .           // |
//        //2--------------1   |
//        //    .          |   |
//        //    .          |   |
//        //    .          |   |
//        //    .          |   |
//        //    7.......   |   /4
//        //               | //
//        //3--------------/0


        glUseProgram(sID);

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint elementID;
        glGenBuffers(1, &elementID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLubyte), index, GL_STATIC_DRAW);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[8][8]), cube, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float[8]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[8]), (void *) sizeof(float[3]));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float[8]), (void *) sizeof(float[5]));

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);

    }

    void assignTexture() {
        glGetError();

        GLubyte texels[128] =
                {
                        // Texels for first image.
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,
                        255, 0, 0,  255,

                        // Texels for second image.
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                        0, 255, 0,  255,
                };

        //创建数组纹理
        glActiveTexture(GL_TEXTURE1);
        GLuint texture;
        glGenTextures(1,&texture);
        glBindTexture(GL_TEXTURE_2D_ARRAY,texture);

        //配置纹理数组参数
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAX_LEVEL, 2);




        //使用不变存储
//        GLsizei width = 4;
//        GLsizei height = 4;
//        GLsizei layerCount = 2;
//        GLsizei mipLevelCount = 3;
//        glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, width, height, layerCount);
//        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, layerCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);
//        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 0, width/2, height/2, layerCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);
//        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, 0, width/4, height/4, layerCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);










        //可变存储
        GLsizei width = 4;
        GLsizei height = 4;
        GLsizei depth = 2;
        GLsizei mipLevelCount = 3;
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        cout << glGetError() << endl;

        //手动生成纹理级别
//        glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
//        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height,     depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1.data());
//        glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width/2, height/2, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1.data());
//        glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, GL_RGBA8, width/4, height/4, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1.data());


    }

    void draw(int width, int height, float f) {

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));


        //Draw Cube
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);


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
    shader.assignTexture();

    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);


    glViewport(0, 0, width, height);

    //project变换
    glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
    GLuint projectionID = glGetUniformLocation(shader.sID, "projection");
    glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

    //view变换
    glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    GLuint viewID = glGetUniformLocation(shader.sID, "view");
    glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);


        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        shader.draw(width, height, f);

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



