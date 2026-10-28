// 7-апта: координаттар кеңістіктері, MVP, проекция
// Тереңдік тесті ЖОҚ (8-аптада қосылады): артқы қырлар алда көрінуі қалыпты жағдай.
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

int gWidth = 800, gHeight = 600;

// Басқару (тапсырмалар үшін)
bool  gOrtho = false;   // O : перспектива <-> ортография (2-тапсырма)
bool  gThree = false;   // K : 1 куб <-> 3 куб (3-тапсырма)
float gFov  = 45.0f;    // 1,2,3,4 : 30, 45, 90, 120 (1-тапсырма)
float gNear = 0.1f;     // Q,W,E,R : 0.1, 1.0, 2.5, 4.0
float gFar  = 100.0f;   // A,S,D   : 100, 5, 2

const char* vsSrc = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vColor;
void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

const char* fsSrc = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// 36 вершина, әр қырдың өз түсі
float vertices[] = {
    // артқы қыр (-z) қызыл
    -0.5f,-0.5f,-0.5f, 1.0f,0.0f,0.0f,
    -0.5f, 0.5f,-0.5f, 1.0f,0.0f,0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,0.0f,0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,0.0f,0.0f,
     0.5f,-0.5f,-0.5f, 1.0f,0.0f,0.0f,
    -0.5f,-0.5f,-0.5f, 1.0f,0.0f,0.0f,
    // алдыңғы қыр (+z) жасыл
    -0.5f,-0.5f, 0.5f, 0.0f,1.0f,0.0f,
     0.5f,-0.5f, 0.5f, 0.0f,1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 0.0f,1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 0.0f,1.0f,0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,1.0f,0.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,1.0f,0.0f,
    // сол қыр (-x) көк
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f,1.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f,1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,0.0f,1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,0.0f,1.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,0.0f,1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f,1.0f,
    // оң қыр (+x) сары
     0.5f,-0.5f,-0.5f, 1.0f,1.0f,0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f,0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f,0.0f,
     0.5f,-0.5f, 0.5f, 1.0f,1.0f,0.0f,
     0.5f,-0.5f,-0.5f, 1.0f,1.0f,0.0f,
    // төменгі қыр (-y) көгілдір
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
     0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
     0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
     0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,1.0f,1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f,1.0f,
    // жоғарғы қыр (+y) қызғылт
    -0.5f, 0.5f,-0.5f, 1.0f,0.0f,1.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,0.0f,1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f,1.0f,
     0.5f, 0.5f,-0.5f, 1.0f,0.0f,1.0f,
    -0.5f, 0.5f,-0.5f, 1.0f,0.0f,1.0f,
};

void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    gWidth = w;
    gHeight = h;
    glViewport(0, 0, w, h);
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
        case GLFW_KEY_O: gOrtho = !gOrtho; std::cout << (gOrtho ? "ortho\n" : "perspective\n"); break;
        case GLFW_KEY_K: gThree = !gThree; break;
        case GLFW_KEY_1: gFov = 30.0f;  break;
        case GLFW_KEY_2: gFov = 45.0f;  break;
        case GLFW_KEY_3: gFov = 90.0f;  break;
        case GLFW_KEY_4: gFov = 120.0f; break;
        case GLFW_KEY_Q: gNear = 0.1f; break;
        case GLFW_KEY_W: gNear = 1.0f; break;
        case GLFW_KEY_E: gNear = 2.5f; break;
        case GLFW_KEY_R: gNear = 4.0f; break;
        case GLFW_KEY_A: gFar = 100.0f; break;
        case GLFW_KEY_S: gFar = 5.0f;   break;
        case GLFW_KEY_D: gFar = 2.0f;   break;
    }
    std::cout << "fov=" << gFov << " near=" << gNear << " far=" << gFar << "\n";
}

GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[512]; glGetShaderInfoLog(s, 512, nullptr, log); std::cerr << log << "\n"; }
    return s;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 7: MVP", nullptr, nullptr);
    if (!window) { std::cerr << "Window failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "GLAD failed\n"; return -1;
    }

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Uniform орындарын циклге дейін кэштеу
    GLint locModel = glGetUniformLocation(prog, "uModel");
    GLint locView  = glGetUniformLocation(prog, "uView");
    GLint locProj  = glGetUniformLocation(prog, "uProjection");

    std::cout << "O ortho | K 1/3 куб | 1-4 fov | Q W E R near | A S D far\n";

    glm::vec3 positions[] = {
        glm::vec3(-1.5f, 0.0f,  0.0f),
        glm::vec3( 0.0f, 0.0f, -1.0f),
        glm::vec3( 1.5f, 0.0f,  0.0f)
    };
    float speeds[] = { 1.0f, 0.6f, 1.6f };

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);

        float angle = (float)glfwGetTime();
        float aspect = (gHeight > 0) ? (float)gWidth / (float)gHeight : 1.0f;

        glm::mat4 projection;
        if (gOrtho)
            projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, gNear, gFar);
        else
            projection = glm::perspective(glm::radians(gFov), aspect, gNear, gFar);

        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
                                     glm::vec3(0.0f, 0.0f, 0.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));

        // projection мен view ортақ, бір рет жіберіледі
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(vao);

        if (!gThree) {
            // Негізгі бөлім: координат басындағы бір куб, Y осінен айналады
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        } else {
            // 3-тапсырма: үш куб, әрқайсысы өз орнында, өз жылдамдығымен
            for (int i = 0; i < 3; i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, positions[i]);
                model = glm::rotate(model, angle * speeds[i], glm::vec3(0.0f, 1.0f, 0.0f));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}