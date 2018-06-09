//Vertex Shader
#version 460 core

layout(location = 1) attribute vec3 position;
layout(location = 2) attribute vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vsColor;
void main(){
    gl_Position = projection * view * model * vec4(position, 1);
    vsColor = color;
}





//Fragment Shader
#version 460 core

layout(location=0) out vec4 outColor1;
layout(location=1) out vec4 outColor2;

in vec3 vsColor;
void main(){
    outColor1 =  vec4(vsColor, 1);
    outColor2 =  vec4(0, 0.9, 0, 1);
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
#include "inc/lodepng.h"


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
    GLuint tID[2];
    GLuint fboID, rboID;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/blitVS.glsl");
        readShader(fID, "../GLSL/blitFS.glsl");


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


    void assignCubeData() {


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


//        glBindVertexArray(0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);


    }

    void assignFBO(int width, int height) {

        //创建FBO
        glGenFramebuffers(1, &fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);

        //创建多重采样纹理对象
        glGenTextures(2, tID);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tID[0]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB8, width, height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tID[1]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB8, width, height, GL_TRUE);

        //指定FBO的颜色buffer的附加点
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, tID[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D_MULTISAMPLE, tID[1], 0);

        //指定输出buffer
        GLenum bufs[2] = {GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(2, bufs);



        //创建多重采样RBO
        glGenRenderbuffers(1, &rboID);
        glBindRenderbuffer(GL_RENDERBUFFER, rboID);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        //将RBO绑定到FBO的深度buffer和模板buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboID);




        //检查完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR is " << glCheckFramebufferStatus(GL_FRAMEBUFFER)
                      << ", Framebuffer is not complete!" << std::endl;

    }


    void drawFBO(int width, int height) {

        //绘制FBO
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;

        //project变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));


        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(1, 2, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));


        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360.0f * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));

        //Draw Cube
        glDrawElements(GL_QUADS, 8, GL_UNSIGNED_BYTE, 0);


    }

};


int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

//    glfwWindowHint(GLFW_SAMPLES, 4);
    int width = 600, height = 480;
    GLFWwindow *window = glfwCreateWindow(width, height, "MSAA", NULL, NULL);
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
    shader.assignCubeData();
    shader.assignFBO(width, height);


    glDisable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    GLboolean b = glIsEnabled(GL_DEPTH_TEST);
    printf("GL_MULTISAMPLE is %d \n", b);

    glBindVertexArray(shader.arrayID);
    glUseProgram(shader.sID);
    while (!glfwWindowShouldClose(window)) {

        glBindFramebuffer(GL_FRAMEBUFFER, shader.fboID);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.drawFBO(width, height);


        glBindFramebuffer(GL_READ_FRAMEBUFFER, shader.fboID);
        glReadBuffer(GL_COLOR_ATTACHMENT3);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);

        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Unbind Vertex Array Object and Shader
    glBindVertexArray(0);
    glUseProgram(0);
    glDeleteFramebuffers(1, &shader.fboID); //删除FBO

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
