//Computer Shader
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
            imageStore(image, ivec2(0,1), uvec4(0,255,0,255));break;
        case 2:
            imageStore(image, ivec2(1,0), uvec4(0,0,255,255));break;
        default:
            imageStore(image, ivec2(1,1), uvec4(0,255,255,255));
    }
}




//Vertex Shader
#version 430 core

layout(location = 1)attribute vec3 position;
layout(location = 2)attribute vec2 coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vsCoord;
void main(){
    gl_Position = projection * view * model * vec4(position, 1);
    vsCoord = coord;
}





//Fragment Shader
#version 430 core
layout(binding=1) uniform sampler2D texture;

in vec2 vsCoord;
out vec4 fsColor;

void main(){
    vec4 c = texture( texture, vsCoord);
    fsColor =  c;
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


    void createShader(bool flag = false) {  //构造函数增加一个标记变量

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint cID = glCreateShader(GL_COMPUTE_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/csVS.glsl");
        readShader(fID, "../GLSL/csFS.glsl");
        readShader(cID, "../GLSL/csCS.glsl");


        //3. COMPILE
        compileShader(vID, "vertex");
        compileShader(fID, "fragment");
        compileShader(cID, "compute");


        //5. ATTACH SHADERS TO PROGRAM
        if (flag) {  //Compute Shader是单Shader管线，flag是标记位，用于判断到底编译哪种Shader
            glAttachShader(sID, cID);
        } else {
            glAttachShader(sID, vID);
            glAttachShader(sID, fID);
        }


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

        //激活纹理单元
        glActiveTexture(GL_TEXTURE1);

        //指定图片宽高
        int width = 2, height = 2;

        //生成纹理
        glGenTextures(1, &tID);
        glBindTexture(GL_TEXTURE_2D, tID);
        //指定纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        //分配空间，不发送数据（等会在计算Shader中计算纹理数据）
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);

        //绑定纹理图片单元
        // 纹理单元和纹理图片单元使用同一个纹理对象做为数据源
        // 所以等会在Compute Shader中更新图片单元时，底层纹理对象中的数据也会发送改变
        // 最后再用使用绘制Shader渲染更新后的纹理
        glBindImageTexture(1, tID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI);

        cout << "error is " << glGetError() << endl;

    }


    void draw(int width, int height, float f) {

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

        //Draw Cube
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);
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



    //创建计算Shader
    Shader computeShader;
    computeShader.createShader(true);
    computeShader.assignTexture(); //分配纹理（纹理单元和纹理图片单元使用同一个纹理对象）
    glUseProgram(computeShader.sID);
    glDispatchCompute(1, 1, 1); //分发任务给计算Shader


    //创建PBO,用于读取更新后的纹理数据，以检查是否纹理对象已经更新
    GLuint pboID;
    glGenBuffers(1, &pboID);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboID);
    glBufferData(GL_PIXEL_PACK_BUFFER, 2 * 2 * 4, NULL, GL_DYNAMIC_DRAW);

    //绑定图片纹理单元2，读取修改后的数据
    glBindImageTexture(1, computeShader.tID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *) 0);
    GLubyte *p = (GLubyte *) glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_WRITE);

    //打印4个像素
    printf("\n---1----\n"); //打印第1个像素
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("\n-------\n");

    printf("\n---2----\n"); //打印第2个像素
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("\n-------\n");

    printf("\n---3----\n"); //打印第3个像素
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("\n-------\n");

    printf("\n---4----\n"); //打印第4个像素
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("%d,", *p++);
    printf("\n-------\n");
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);



    //创建绘制Shader
    // 上面已经用计算Shader生成了图片,下面的主循环中使用绘制Shader渲染
    // 绘制Shader不用指定纹理，因为前面调用computeShader.assignTexture()时已经指定
    Shader drawShader;
    drawShader.createShader();
    drawShader.assignCubeData();
    glUseProgram(drawShader.sID);
    glBindVertexArray(drawShader.arrayID);

    glViewport(0, 0, width, height);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        //渲染
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        drawShader.draw(width, height, f);


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
