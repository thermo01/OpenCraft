#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shader sources
const char* vertexShaderSource = R"(
    #version 120

    attribute vec3 aPos;
    uniform mat4 MVP;

    void main() {
        gl_Position = MVP * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 120

    void main() {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Red color
    }
)";

// Camera parameters
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;
float cameraSensitivity = 0.1f;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

// Function to handle mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    xoffset *= cameraSensitivity;
    yoffset *= cameraSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Function to handle keyboard input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Function to compile shaders
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shaderID = glCreateShader(type);
    glShaderSource(shaderID, 1, &source, NULL);
    glCompileShader(shaderID);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
    }

    return shaderID;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Fly Camera Example", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set mouse input callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Compile vertex and fragment shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link shaders into a shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    // Delete shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);

        // Render here
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Set up projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

        // Set up camera transformation
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Set up model transformation (identity for now)
        glm::mat4 model = glm::mat4(1.0f);

        // Combine matrices
        glm::mat4 mvp = projection * view * model;

        // Use shader program
        glUseProgram(shaderProgram);

        // Set MVP uniform
        unsigned int MVPLocation = glGetUniformLocation(shaderProgram, "MVP");
        glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        // Render triangle
        glBegin(GL_POLYGON);
        glVertex3f(-0.5f, -0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glEnd();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
