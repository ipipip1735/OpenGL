//Vertex Shader
#version 430 core

layout(location = 1) attribute vec3 position;
layout(location = 2) attribute vec2 coord;
layout(location = 3) attribute vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoord;
out vec4 outColor;
void main(){
    gl_Position = projection * view * model * vec4(position, 1);
    texCoord = coord;
    outColor = vec4(color, 1);
}





//Fragment Shader
#version 420 core

layout(binding=1, rgba8ui)uniform uimage2D image;

in vec2 texCoord;
in vec4 outColor;

out vec4 FragColor;
void main(){

    vec4 c = imageLoad(image, ivec2(1,1));
        FragColor = c /255;  //uimage2D是GLubety类型必须标准化，否则全白显示
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

    GLuint sID;
    GLuint arrayID;
    GLuint tID;


    void createShader() {

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);


        //2. LOAD SHADER SOURCE CODE
        readShader(vID, "../GLSL/PBOVS.glsl");
        readShader(fID, "../GLSL/PBOFS.glsl");


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


        float cube[8][8] = {
                {1,  -1, 1,  0, 0, 1, 0, 0},
                {1,  1,  1,  5, 0, 0, 1, 0},
                {-1, 1,  1,  5, 1, 0, 0, 1},
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
    }

    void assignTexture() {
        glGetError();

        //创建纹理对象
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &tID);
        //配置纹理参数
        glBindTexture(GL_TEXTURE_2D, tID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);


        //准备数据，8通道2X2图片
        int width=2, height=2;
        vector<GLubyte> image(width * height * 4, 128); //初值为128

        //创建PBO
        GLuint pboID;
        glGenBuffers(1, &pboID);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(GLubyte)*image.size() , image.data(), GL_STATIC_DRAW);

        //发送PBO数据到GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)0);
        glGenerateMipmap(GL_TEXTURE_2D); //生成纹理贴图

        //绑定图片纹理单元
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

    shader.assignCubeData();
    shader.assignTexture();

    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    glViewport(0, 0, width, height);
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
