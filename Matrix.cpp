//Vertex Shader
#version 420 core

layout(location = 1) attribute vec3 position;
layout(location = 2) attribute vec4 color;

uniform mat4 model[3];
uniform mat4 view;
uniform mat4 projection;

out vec4 outColor;

void main(){
    gl_Position = projection * view * model[2] * model[1] * model[0] * vec4(position, 1);
    outColor = color;
}





//Fragment Shader
#version 420 core

in vec4 outColor;

void main(){
    gl_FragColor = outColor;
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

    GLuint sID;
    GLuint arrayID;
    GLuint tID1, tID2;

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
    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/matrixVS.glsl");
        readShader(fID, "../GLSL/matrixFS.glsl");


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

        float triangle[3][5] = {
                {-1, -0.5, 1, 0, 0},
                {0,  1,    0, 1, 0},
                {1,  -0.5, 0, 0, 1}
        };

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, 2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[3][5]), triangle, GL_STATIC_DRAW);


        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[5]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float[5]), (void *) sizeof(float[2]));
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }


    void draw(int width, int height) {

        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //Bind Shader and Vertex Array Object
        glUseProgram(sID);
        glBindVertexArray(arrayID);

        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //投影变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));


        //model变换
        //方法三
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;


        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 0, 1));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

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



        //方法二
//        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
//        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0)* f);
//        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360*f), glm::vec3(0, 0, 1));
//        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(0.5*f, 0, 0));
//
//        GLuint scaleID = glGetUniformLocation(sID, "scale");
//        glUniformMatrix4fv(scaleID, 1, GL_FALSE, glm::value_ptr(scale));
//        GLuint rotateID = glGetUniformLocation(sID, "rotate");
//        glUniformMatrix4fv(rotateID, 1, GL_FALSE, glm::value_ptr(rotate));
//        GLuint translateID = glGetUniformLocation(sID, "translate");
//        glUniformMatrix4fv(translateID, 1, GL_FALSE, glm::value_ptr(translate));





        //方法一
//        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
//
//        glm::mat4 model(1);
//        model = glm::translate(model, glm::vec3(0.5*f, 0, 0));
//        model = glm::rotate(model, glm::radians(360*f), glm::vec3(0, 0, 1));
//        model = glm::scale(model, glm::vec3(1, 1, 0)* f);
//
//        GLuint modelID = glGetUniformLocation(sID, "model");
//        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));


        //Draw Triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);


        //Unbind Vertex Array Object and Shader
        glBindVertexArray(0);

    }

    void drawMultiple(int width, int height) {

        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Bind Shader and Vertex Array Object
        glUseProgram(sID);
        glBindVertexArray(arrayID);

        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {


                //view变换
                glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.0f), glm::vec3(2, 3, 0), glm::vec3(0, 1, 0));
                GLuint viewID = glGetUniformLocation(sID, "view");
                glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

                //投影变换
                glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
                GLuint projectionID = glGetUniformLocation(sID, "projection");
                glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));


                //model变换
                float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
                glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * f);
                glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 0, 1));
                glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(i, j, 0));

                //三个矩阵放在容器中一起发送到GPU
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
                glDrawArrays(GL_TRIANGLES, 0, 3);
            }
        }

        //Unbind Vertex Array Object and Shader
        glBindVertexArray(0);
        glUseProgram(0);
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

    while (!glfwWindowShouldClose(window)) {

        //绘制单三角形
        shader.draw(width, height);

        //绘制100个三角形
        shader.drawMultiple(width, height);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
