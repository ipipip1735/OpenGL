//Vertex Shader
#version 420 core

layout(location = 1) attribute vec2 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(position, 0, 1);
}





//Fragment Shader
#version 420 core

layout(std140, binding=2) uniform Color{
    vec3 i;
    float f;
    vec3 j;
    float g;
};

void main(){

//    gl_FragColor = vec4(i, 1); //测试i
//    gl_FragColor = vec4(j, 1); //测试f
//    gl_FragColor = vec4(f, 0,0,1); //测试j
//    gl_FragColor = vec4(g, 0,0,1); //测试g

    gl_FragColor = vec4(i+j, (f + g));
}






//CPP source code
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <lodepng.h>

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
    GLuint uboID;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/UBOVS.glsl");
        readShader(fID, "../GLSL/UBOFS.glsl");


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

        float triangle[3][2] = {
                {-1, -0.5f},
                {0,  1},
                {1,  -0.5f}
        };

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[2]), 0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }


    void draw(int width, int height) {


        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //projection变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * 5.0f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360.0f * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(0, 0, -5));

//        glm::mat4 model = translate * scale;
        glm::mat4 model(1);

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));


        //Draw Triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

    }

    void assignUBO() {

        //准备数据
        float v[3] = {1, 0, 0.1};
        float f = 0.9f;
        float j[3] = {0, 1, 0};
        float g = 0.5f;

        //创建UBO
        glGenBuffers(1, &uboID);
        glBindBuffer(GL_UNIFORM_BUFFER, uboID);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboID);
        //分配空间，但不加载数据
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, NULL, GL_STATIC_DRAW);


        //给UBO指定4个数据，这些数据将传递给Shader的uniform块
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(v), &v);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 3, sizeof(f), &f);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4, sizeof(j), &j);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 7, sizeof(g), &g);
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
    shader.assignUBO();

    //Bind Shader and Vertex Array Object
    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);


    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);


    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //准备数据
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        float u[3] = {1, 0, f};
        float x = 0.9f;
        float v[3] = {0, 1, f};
        float y = 0.5f;

        glBindBuffer(GL_UNIFORM_BUFFER, shader.uboID);

        //更新UBO中的数据
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(u), &u);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 3, sizeof(x), &x);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4, sizeof(v), &v);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 7, sizeof(y), &y);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

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


