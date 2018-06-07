//Vertex Shader
#version 430 core

layout(location = 1) attribute vec3 position;
layout(location = 2) attribute vec2 coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
void main(){
    gl_Position = projection * view * model * vec4(position, 1);
    texCoord = coord;
}




//Fragment Shader
#version 430 core

layout(binding =0) uniform sampler2D sampler1;
layout(binding =1) uniform sampler2D sampler2;

in vec2 texCoord;

void main(){
    gl_FragColor= texture(sampler1, texCoord);
}





//FBO Fragment Shader
#version 430 core

layout(location=0) out vec4 outColor1;
layout(location=1) out vec4 outColor2;

void main(){
    outColor1 =  vec4(0.8f, 0, 0, 1);
    outColor2 =  vec4(0, 0.9f, 0, 1);
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

struct Bitmap {

    int width;
    int height;
    short BitsPerPixel;
    vector<unsigned char> pixels;

    Bitmap(const char *FilePath) : width(0), height(0), BitsPerPixel(0) {
        load(FilePath);
    }

    //adapted from http://stackoverflow.com/questions/20595340/loading-a-tga-bmp-file-in-c-opengl
    void load(const char *FilePath) {
        fstream hFile;

        //search for file by going up file directory tree up to 5 times
        int attempts = 0;
        string nfilepath = FilePath;
        while (!hFile.is_open() && attempts < 5) {
            hFile.open(nfilepath.c_str(), ios::in | ios::binary);
            nfilepath = "../" + nfilepath;
            attempts += 1;
        }
        if (!hFile.is_open()) throw invalid_argument("Error: File Not Found.");

        hFile.seekg(0, ios::end);
        int Length = hFile.tellg();
        hFile.seekg(0, ios::beg);


        vector<uint8_t> FileInfo(Length);
        hFile.read(reinterpret_cast<char *>(FileInfo.data()), 54);

        if (FileInfo[0] != 'B' && FileInfo[1] != 'M') {
            hFile.close();
            throw invalid_argument("Error: Invalid File Format. Bitmap Required.");
        }

        if (FileInfo[28] != 24 && FileInfo[28] != 32) {
            hFile.close();
            throw invalid_argument("Error: Invalid File Format. 24 or 32 bit Image Required.");
        }

        BitsPerPixel = FileInfo[28];
        width = FileInfo[18] + (FileInfo[19] << 8);
        height = FileInfo[22] + (FileInfo[23] << 8);
        uint32_t pixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
        uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
        pixels.resize(size);

        hFile.seekg(pixelsOffset, ios::beg);
        hFile.read(reinterpret_cast<char *>(pixels.data()), size);
        hFile.close();
    }

};

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

    string fragmentShader;
    GLuint sID;
    GLuint arrayID;
    GLuint tID[2];
    GLuint fboID = 99;


    Shader(string fragmentShader = "msaaFS.glsl") {
        this->fragmentShader = "../GLSL/" + fragmentShader;
    }

    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/msaaVS.glsl");
        readShader(fID, fragmentShader);


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


        float cube[8][5] = {
                {1,  -1, 1,  0, 0},
                {1,  1,  1,  1, 0},
                {-1, 1,  1,  1, 1},
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 24, index, GL_STATIC_DRAW);

        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float[5]), 0);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float[5]), (void *) sizeof(float[3]));


//        glBindVertexArray(0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);


    }

    void assignTexture(int width, int height) {

        glGenTextures(2, &tID[0]); //创建2个纹理

        //配置第1个纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tID[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); //只分配空间，数据留空
        glGenerateMipmap(GL_TEXTURE_2D);



        //配置第2个纹理
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tID[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);  //只分配空间，数据留空
        glGenerateMipmap(GL_TEXTURE_2D);

    }

    void draw(int width, int height) {


        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;


        //project变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));


        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));

        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1) * 1.0f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * 1.0f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(scale));

        //Draw QUAD
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);


    }

    void assignFBO(int width, int height) {

        //创建FBO
        glGenFramebuffers(1, &fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);


        //设置颜色buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tID[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tID[1], 0);

        GLenum bufs[2] = {GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(2, bufs);


        //设置深度buffer和模板buffer
        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


        //检查完整性
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR is " << glCheckFramebufferStatus(GL_FRAMEBUFFER)
                      << ", Framebuffer is not complete!" << std::endl;


        //        glDeleteFramebuffers(1, &fboID); //删除FBO
    }

    void drawFBO(int width, int height) {

        //绘制FBO
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;

        //project变换
        glm::mat4 projection = glm::perspective(45.0f, (float) width / height, 0.1f, 100.f);
        GLuint projectionID = glGetUniformLocation(sID, "projection");
        glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projection));



        //view变换
        glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 10.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        GLuint viewID = glGetUniformLocation(sID, "view");
        glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(view));


        //model变换
        glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(1, 1, 1) * f);
        glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(360 * f), glm::vec3(0, 1, 0));
        glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(rotate));


        //Draw Cube
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, 0);
//        cout << "FBO error is " << glGetError() << endl;



        //读取FBO中的内容
//        glReadBuffer(GL_COLOR_ATTACHMENT2);
//        vector<float> pixels(width * height * 4, 0.1f);
//        glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels.data());

        //打印附加点2的像素数据
//        int n = 0;
//        for (int i = 0; i < width; ++i) {
//            for (int j = 0; j < height; ++j) {
//
//                for (int k = 0; k < 4; ++k) { //每个像素有RGBA四个值
//
//                cout << pixels[n++] << ",";
//
//                }
//
//            }
//
//        }



        //保存附加点3中的数据到文件
//        glReadBuffer(GL_COLOR_ATTACHMENT3);
//        vector<GLubyte> image(width * height * 4, 222);
//        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
//        //保存到文件
//        unsigned error = lodepng::encode("../resources/xx.png", image, width, height);
//        cout <<error<< endl;


    }

};


int main() {

    if (!glfwInit()) exit(EXIT_FAILURE);

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


    Shader shaderFBO("msaaFBOFS.glsl");
    shaderFBO.createShader();
    shaderFBO.assignCubeData();
    shaderFBO.assignTexture(width, height);
    shaderFBO.assignFBO(width, height);


    Shader shader;
    shader.createShader();


    glBindVertexArray(shaderFBO.arrayID);

    while (!glfwWindowShouldClose(window)) {

        //绑定FBO
        glBindFramebuffer(GL_FRAMEBUFFER, shaderFBO.fboID);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //渲染到FBO
        glUseProgram(shaderFBO.sID);
        shaderFBO.drawFBO(width, height);






        //解绑FBO,绑定默认缓冲帧
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST); //默认缓冲帧只绘制长方形，然后显示FBO中的纹理，所以不需要深度测试
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        //渲染
        glUseProgram(shader.sID);
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


