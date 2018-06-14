//Vertex Shader
#version 420 core

layout(location = 1) attribute vec2 position;
layout(location = 2) attribute vec2 coord;
layout(location = 3) attribute vec3 color;

out vec4 vertexColor;
out vec2 coordiration;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    coordiration = coord;
    gl_Position = projection * view * model * vec4(position, 0, 1);
    vertexColor = vec4(color, 1);
}





//Fragment Shader
#version 430 core

layout(binding=2) uniform sampler2D sampler;

//SSBO将作为buffer块的数据源
layout(std140, binding=0) buffer Flag{
    float i;
};

in vec2 coordiration;
in vec4 vertexColor;

void main(){

    i = 0.68f; //修改buffer块中的变量
    vec4 texColor = texture(sampler, coordiration);
    gl_FragColor = vec4(vec3(texColor), vec2(0,0));

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


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/SSBOVS.glsl");
        readShader(fID, "../GLSL/SSBOFS.glsl");


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

        float triangle[3][7] = {
                {-1, -0.5f, 0,    0, 1, 0, 0},
                {0,  1,     1,    0, 0, 1, 0},
                {1,  -0.5f, 0.5f, 1, 0, 0, 1}
        };


        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[3][7]), triangle, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[7]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[7]), (void *) sizeof(float[2]));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float[7]), (void *) sizeof(float[4]));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }


    void assignTexture() {

        glActiveTexture(GL_TEXTURE2);
        glGenTextures(1, &tID1);
        glBindTexture(GL_TEXTURE_2D, tID1);

        vector<unsigned char> image; //the raw pixels
        unsigned width, height;
        unsigned error = lodepng::decode(image, width, height, "../resources/y.png");

        //if there's an error, display it
        if (error) cout << "decoder error " << error << ": " << lodepng_error_text(error) << endl;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

    //Bind Shader and Vertex Array Object
    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);


    //创建SSBO
    GLuint ssboID;
    glGenBuffers(1, &ssboID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float)*4, NULL, GL_DYNAMIC_COPY); //分配空间，不发送数据
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboID); //绑定到索引点0


    glViewport(0, 0, width, height);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        shader.draw(width, height);

        //准备数据容器
        float i[4] = {0.1f, 0.2f, 0.3f ,0.4f};
        //获取SSBO中的数据
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float[4]), i);
        //打印
        cout << i[0] << endl;
        cout << i[1] << endl;
        cout << i[2] << endl;
        cout << i[3] << endl;

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
