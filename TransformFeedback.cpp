//Vertex Shader
#version 420 core

layout(location = 1) attribute vec3 position;
layout(location = 2) attribute vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 outColor1; //反馈转换过程中要捕获的输出变量
out vec4 outColor2; //反馈转换过程中要捕获的输出变量

void main(){
    outColor1 = vec4(1,2,3,4);
    outColor2 = vec4(5,6,7,8);
    gl_Position = projection * view * model * vec4(position, 1);
}







//Fragment Shader
#version 420 core

in vec4 outColor;

void main(){
    gl_FragColor = vec4(1);
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
        readShader(vID, "../GLSL/TFVS.glsl");
        readShader(fID, "../GLSL/TFFS.glsl");


        //3. COMPILE
        compileShader(vID, "vertex");
        compileShader(fID, "fragment");


        //5. ATTACH SHADERS TO PROGRAM
        glAttachShader(sID, vID);
        glAttachShader(sID, fID);

        //程序链接前指定从Shader输出的变量
//        vector<const char *> varyings = {"outColor1", "outColor2"};  //使用交叉写入
//        glTransformFeedbackVaryings(sID, varyings.size(), varyings.data(), GL_INTERLEAVED_ATTRIBS);

        const char *varyings[] = {"outColor1", "gl_NextBuffer", "outColor2"}; //使用独立存储(一个变量写入一个buffer)
        glTransformFeedbackVaryings(sID, 3, varyings, GL_INTERLEAVED_ATTRIBS);

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

        float triangle[8][6] = {
                {1,  -1, 1,  1, 0, 0},
                {1,  1,  1,  0, 1, 0},
                {-1, 1,  1,  0, 1, 1},
                {-1, -1, 1,  1, 0, 0},
                {1,  -1, -1, 0, 1, 0},
                {1,  1,  -1, 0, 0, 1},
                {-1, 1,  -1, 1, 0, 0},
                {-1, -1, -1, 0, 1, 0},
        };


        GLubyte index[6] = {0, 1, 2, 0, 2, 3}; //bottom

//        GLubyte index[24] = {
//                0, 1, 2, 3, //front
//                7, 6, 5, 4, //back
//                3, 2, 6, 7, //left
//                4, 5, 1, 0, //right
//                1, 5, 6, 2, //top
//                4, 0, 3, 7}; //bottom


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
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

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

        //投影变换
        float f = (sin(glfwGetTime() * 2) + 1.0f) / 2;
        glm::mat4 model(1);
//        model = glm::scale(model, glm::vec3(1, 1, 0) * f);
//        model = glm::rotate(model, glm::radians(360 * f), glm::vec3(0, 0, 1));
//        model = glm::translate(model, glm::vec3(2 * f, 0, 0));

        GLuint modelID = glGetUniformLocation(sID, "model");
        glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(model));

        //Draw Triangle
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

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







    //配置单个TFBO，多个变量输出到同一个TFBO
//    GLuint tfboID;
//    glGenBuffers(1, &tfboID);
//    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfboID);
//    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 2 * 8 * 3, NULL, GL_DYNAMIC_COPY);
//    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfboID); //绑定到索引0上







    //配置2个TFBO，从Shader输出的2个变量输出到各自的TFBO
    //使用独立存储（"outColor1", "gl_NextBuffer", "outColor2"）
    GLuint tfboID[2];
    glGenBuffers(2, &tfboID[0]);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfboID[0]);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 1 * 8 * 3, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfboID[0]); //绑定到索引0上

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfboID[1]);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(float) * 1 * 8 * 3, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, tfboID[1]); //绑定到索引1上






    glViewport(0, 0, width, height);

    //Bind Shader and Vertex Array Object
    glUseProgram(shader.sID);
    glBindVertexArray(shader.arrayID);

    while (!glfwWindowShouldClose(window)) {

        glClear(GL_COLOR_BUFFER_BIT);
//        glEnable(GL_RASTERIZER_DISCARD); //禁用光栅化








//        //配置查询对象，查询绘制图元的个数
//        GLuint query;
//        glGenQueries(1, &query);
//        glBeginQuery(GL_PRIMITIVES_GENERATED, query); //开始查询
//        shader.draw(width, height);
//        glEndQuery(GL_PRIMITIVES_GENERATED); //结束查询
//
//        //获取查询结果
//        GLuint primitives;
//        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);
//        cout << primitives << endl;















        //配置查询对象，查询转换反馈成功俘获的图元个数（转换反馈必须是所有顶点数据都获取到了才算成功获取此图元）
//        GLuint query;
//        glGenQueries(1, &query);
//        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query); //开始查询
//        glBeginTransformFeedback(GL_TRIANGLES); //开始转换反馈
//        shader.draw(width, height);
//        glEndTransformFeedback();  //结束转换反馈
//        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN); //结束查询
//
//        //获取查询结果
//        GLuint primitives;
//        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);
//        printf("%u primitives written!\n\n", primitives);




















//        //转换反馈
//        glBeginTransformFeedback(GL_TRIANGLES); //开始转换反馈
//        shader.draw(width, height);
//        glEndTransformFeedback();  //结束转换反馈

        //获取TFBO中的结果，有2种方法
        //方法一，从TFBO中获取结果，复制到容器中
//        vector<float> rtf(8, 5);
//        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfboID);
//        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * 8, rtf.data());
//
//        float *fp = rtf.data();
//        cout << "-----\n";
//        for (int i = 0; i < 2; ++i) {
//            cout << "rtf is ";
//            for (int j = 0; j < 4; ++j) {
//                cout << *fp++;
//            }
//            cout << "\n";
//        }
//        cout << "-----\n";
//        cout << glGetError() << endl;

        //方法二，使用DMA方式获取TFBO中的数据
//        float *fp = (float *)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
//        cout << "-----\n";
//        for (int i = 0; i < 2; ++i) {
//            cout << "rtf is ";
//            for (int j = 0; j < 4; ++j) {
//                cout << *fp++;
//            }
//            cout << "\n";
//        }
//        cout << "-----\n";
//        glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);





















        //独立存储的转换反馈（"outColor1", "gl_NextBuffer", "outColor2"）
        glBeginTransformFeedback(GL_TRIANGLES); //开始转换反馈
        shader.draw(width, height);
        glEndTransformFeedback();  //结束转换反馈

        //获取TFBO中的结果，有2种方法
        //方法一，从TFBO中获取结果，复制到容器中
        vector<float> rtf(8, 5);
        //导出GL_TRANSFORM_FEEDBACK_BUFFER绑定点的索引0中的数据
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfboID[0]);
        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * 4, rtf.data());
        //导出GL_TRANSFORM_FEEDBACK_BUFFER绑定点的索引1中的数据
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, tfboID[1]);
        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * 4, rtf.data()+4);//rtf偏移量为4

        //打印结果
        float *fp = rtf.data();
        cout << "-----\n";
        for (int i = 0; i < 2; ++i) {
            cout << "rtf is ";
            for (int j = 0; j < 4; ++j) {
                cout << *fp++;
            }
            cout << "\n";
        }
        cout << "-----\n";
//        cout << glGetError() << endl;







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
