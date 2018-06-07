//Vertex Shader
#version 420 core

layout(location = 1) attribute vec3 position;
layout(location = 2) attribute vec3 color;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model[3];

out vec4 outColor;

void main(){
    gl_Position = projection * view * model[2] * model[1] * model[0] * vec4(position, 1);
    outColor = vec4(color , 1);
}





//Fragment Shader
#version 420 core

in vec4 outColor;

void main(){
    gl_FragColor = outColor;
}








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

    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/EBOVS.glsl");
        readShader(fID, "../GLSL/EBOFS.glsl");


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

        float cube[8][6] = {
                {1,  -1, 1,  1, 0, 0},
                {1,  1,  1,  0, 1, 0},
                {-1, 1,  1,  0, 0, 1},
                {-1, -1, 1,  1, 0, 0},
                {1,  -1, -1, 0, 1, 0},
                {1,  1,  -1, 0, 0, 1},
                {-1, 1,  -1, 1, 0, 0},
                {-1, -1, -1, 0, 1, 0}
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

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint elementID;
        glGenBuffers(1, &elementID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float[6]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float[6]), (void *) sizeof(float[3]));

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }


    void draw(int width, int height) {


        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //投影变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(1 * f, 0, 0));

        float *p;
        vector<float> models(48, 0);

        p = glm::value_ptr(scale);
        copy(p, p + 16, models.begin());
        p = glm::value_ptr(rotate);
        copy(p, p + 16, models.end() - 32);
        p = glm::value_ptr(translate);
        copy(p, p + 16, models.end() - 16);

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 3, GL_FALSE, models.data());


        //Draw Triangle
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);

    }
};

int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

    int width = 640, height = 480;
    GLFWwindow *window = glfwCreateWindow(width, height, "EBO", NULL, NULL);
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

    //Bind Shader and Vertex Array Object
    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.draw(width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Unbind Vertex Array Object and Shader
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);


    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


