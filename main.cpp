#define MAX_VERTEX 1000

#include "FourierCalculator.h"

#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void initOpenGLData(uint32_t& vertex_id) {
    glCreateBuffers(1, &vertex_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * MAX_VERTEX, nullptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
}

void initShader(uint32_t& shader) {
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "uniform mat4 MVP;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = MVP * vec4(pos.x, pos.y, 0.0f, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(vec3(1.0f, 1.0f, 1.0f), 1.0f);\n"
        "}\n\0";

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

int main() {
    size_t n;
    std::cout << "Enter value of N (higher the N better the result):" << std::endl;
    std::cin >> n;

    if (!glfwInit())
    {
        std::cerr << "FATAL ERROR: Failed to initialize Opengl" << std::endl;
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "FATAL ERROR: Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Fourier Transform", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwTerminate();
        return -1;
    }

    struct windowdata {
        bool mousedown = false;
        bool drawn = false;
    } data;

    glfwSetWindowUserPointer(window, &data);

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        if (action == GLFW_PRESS)
            static_cast<windowdata*>(glfwGetWindowUserPointer(window))->mousedown = true;
        else if (action == GLFW_RELEASE) {
            static_cast<windowdata*>(glfwGetWindowUserPointer(window))->mousedown = false;
            static_cast<windowdata*>(glfwGetWindowUserPointer(window))->drawn = true;
        }
        });

    uint32_t shader, vertex_id, res_vertex_id, vertex_count = 0;
    initShader(shader);
    initOpenGLData(vertex_id);
    glfwSwapInterval(1);

    datatype prevtime = glfwGetTime(), polling_rate = 0.01, draw_speed = 0.1;
    int height, width;
    glm::mat4 MVP;
    bool calculated = false;

    std::vector<vertex> vec, res;

    while (!glfwWindowShouldClose(window)) {

        datatype curtime = glfwGetTime();

        if (calculated && curtime - prevtime > draw_speed) {
            vertex_count++;
            prevtime = curtime;
            if (vertex_count >= res.size()) vertex_count = 0;
        }

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (data.mousedown && vertex_count < MAX_VERTEX && !data.drawn && curtime - prevtime > polling_rate) {
            vertex_count++;
            double x, y;
            int w_x = 0, w_y = 0;
            glfwGetCursorPos(window, &x, &y);
            glfwGetWindowSize(window, &w_x, &w_y);
            vec.emplace_back(x - w_x/2.0f, -y + w_y/2.0f);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex) * vertex_count, sizeof(vertex), &vec.back());
            prevtime = curtime;
        }

        glUseProgram(shader);

        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        MVP = glm::ortho(-(float)width / 2.0f, (float)width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

        glDrawArrays(GL_LINE_STRIP, 1, vertex_count);

        if (data.drawn && !calculated) {
            float a0;
            std::vector<std::complex<datatype>> Cn;
            calc(vec, n, 1, Cn);
            res = plot(Cn, 1, vec.size());

            calculated = true;
            vertex_count = 0;

            glBufferData(GL_ARRAY_BUFFER, res.size() * sizeof(vertex), res.begin()._Ptr, GL_STATIC_DRAW);
        }
        glDrawArrays(GL_LINE_STRIP, 1, vertex_count);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertex_id);
    if(calculated)  glDeleteBuffers(1, &res_vertex_id);

    glfwTerminate();
}
