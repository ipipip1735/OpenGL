//Compute Shader
#version 430 core

//指定并发执行次数
layout (local_size_x = 2, local_size_y = 2) in;

layout(binding=1) uniform writeonly uimage2D image; //定义image变量接受纹理图片单元传来的数据

void main(void)
{
    //通过调用索引判断如何构建纹理数据
    switch(gl_LocalInvocationIndex){
        case 0:
            imageStore(image, ivec2(0,0), uvec4(255,0,0,255));break;
        case 1:
            imageStore(image, ivec2(1,0), uvec4(0,255,0,255));break;
        case 2:
            imageStore(image, ivec2(1,1), uvec4(0,0,255,255));break;
        default:
            imageStore(image, ivec2(0,1), uvec4(0,255,255,255));
    }
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
    GLuint tID;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint cID = glCreateShader(GL_COMPUTE_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(cID, "../GLSL/csCS.glsl");


        //3. COMPILE
        compileShader(cID, "compute");


        //5. ATTACH SHADERS TO PROGRAM
        glAttachShader(sID, cID);


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
        glDeleteShader(cID);
    }


    void assignData() {

        glGenVertexArrays(1, &arrayID);
        glBindVertexArray(arrayID);

        float triangle[4][8] = {
                {-1, -.5, 0,   0, 3, 0, 0, 1},
                {0,  1,   0,   3, 3, 0, 0, 1},
                {1,  -.5, 0.5, 3, 3, 0, 0, 1}
        };

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[3][8]), triangle, GL_STATIC_DRAW);


        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float[8]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[8]), (void *) sizeof(float[2]));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float[8]), (void *) sizeof(float[4]));


        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


    }

    void assignCubeData() {


        float cube[8][5] = {
                {1,  -1, 1,  0, 0},
                {1,  1,  1,  5, 0},
                {-1, 1,  1,  5, 1},
                {-1, -1, 1,  0, 1},
                {1,  -1, -1, 0, 0},
                {1,  1,  -1, 1, 0},
                {-1, 1,  -1, 1, 1},
                {-1, -1, -1, 0, 1}
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLubyte), index, GL_STATIC_DRAW);


        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float[8][5]), cube, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float[5]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[8]), (void *) sizeof(float[3]));

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }

    void assignTexture() {
        glGetError();

        glActiveTexture(GL_TEXTURE1);
        int width = 2, height = 2;
        vector<GLubyte> image(width * height * 4, 120);
        glGenTextures(1, &tID);
        glBindTexture(GL_TEXTURE_2D, tID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);


        glBindImageTexture(1, tID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI);

        cout << "error is " << glGetError() << endl;
    }


    void draw(int width, int height, float f) {
//        GLuint samplerID = glGetUniformLocation(sID, "sampler");
//        glUniform1i(samplerID, 0);

        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //投影变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));


        //Draw Triangle
//        glDrawArrays(GL_TRIANGLES, 0, 3);

        //Draw Cube
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);


    }

    void drawTriangle(int width, int height, float f) {


        GLuint samplerID = glGetUniformLocation(sID, "sampler");
        glUniform1i(samplerID, 0);

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 0) * 5.0f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(0, 0, -5));

//        glm::mat4 model = translate * scale;

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));


        //Draw Triangle
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, 0);
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


    Shader computeShader;
    computeShader.createShader();
    computeShader.assignTexture();
    glUseProgram(computeShader.sID);



    //方法一
    glDispatchCompute(8, 19, 1); //分发命令，启动Compute Shader
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); //创建sync对象，也表示此栅栏被插入到CPU端的命令队列
    GLenum em = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000 * 1000 * 1); //阻塞程序，等待sync对象被GPU处理
    //打印状态
    switch (em)
    {
        case GL_ALREADY_SIGNALED:
            cout << "GL_ALREADY_SIGNALED" << endl;
            break;
        case GL_TIMEOUT_EXPIRED:
            cout << "GL_TIMEOUT_EXPIRED" << endl;
            break;
        case GL_CONDITION_SATISFIED:
            cout << "GL_CONDITION_SATISFIED" << endl;
            break;
        case GL_WAIT_FAILED:
            cout << "GL_WAIT_FAILED" << endl;
            break;

        default:
            cout << "default" << endl;
    }



//    //方法二（需要手动调用Flush()保证Sync对象被发送到GPU）
//    glDispatchCompute(1024, 1024, 1); //分发命令，启动Compute Shader
//    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); //创建sync对象，也表示此栅栏被插入到CPU端的命令队列
//    glFlush(); //手动同步（保证sync对象被发送到GPU），刷新CPU端命令缓存（将导致进程阻塞直到所有命令都被发送到GPU命令队列）
//    glWaitSync(sync, 0, 1000 * 1000 * 1); //等待，直到sync对象被GPU标记



    glUseProgram(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
