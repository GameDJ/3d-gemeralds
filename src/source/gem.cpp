#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "camera.h"
#include "model.h"
#include "filesystem.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 2048;//2560;
const unsigned int SCR_HEIGHT = 1152;//1440;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 6.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float angle = 0.0f;
const float rotateDivisor = 28.0f;
float shiftDis = 0.0f;

// Revolution
int revolveMode = 0;
const float revolveDivisor = 32.0f;
bool rButtonLock = false;
float revolveOffset = 0.0f;
const float revolveHeight = 0.4f;
const float revolveHeightSpeedMult = 1.3f;

// Wireframe
bool wireframe_enabled = false;
bool lButtonLock = false;
float lineWidth = 10.0f;
float lineWidthMaxDistance = 10.0f;

//lighting
//glm::vec3 lightPos(1.2f, 0.2f, 2.0f);
glm::vec3 lightPos(-7.5f, 2.0f, -10.0f);


// diamond dimensions (each from origin)
const float innerRadius = 0.5f;
const float outerRadius = 0.7f;
const float outerHeight = 0.35f;
const float innerHeight = 0.5f;
const float pointHeight = -0.5f;

const float sqrt3 = sqrt(3);
const glm::vec2 innerCorner = glm::vec2(innerRadius / 2, sqrt3 * innerRadius / 2);
const glm::vec2 outerCorner = glm::vec2(outerRadius / 2, sqrt3 * outerRadius / 2);

// assists with calculation of midpoints
const glm::vec2 midCorner = glm::vec2((outerCorner.x + outerRadius) / 2, outerCorner.y / 2);

// Normals
const glm::vec3 midNormal = glm::normalize(glm::vec3(sqrt3 * 21.0f / 400.0f, sqrt3 * 7.0f / 100.0f, 21.0f / 400.0f));
const glm::vec3 midCenterNormal = glm::normalize(glm::vec3(0.0f, sqrt3 / 20.0f, 0.075f));
const glm::vec3 bottomNormal = glm::normalize(glm::vec3(sqrt3 * 119.0f / 400.0f, sqrt3 * -49.0f / 200.0f, 119.0f / 400.0f));
const glm::vec3 bottomCenterNormal = glm::normalize(glm::vec3(0.0f, sqrt3 * -0.245f, 0.595f));

// for calculating gem locations
const float PI = 3.1415926f;
const float gemDist = 2.5f;

// gem colors,locations
map<int, glm::vec3> gemLocs = {
    {0, glm::vec3(0.0f, 0.0f, gemDist)},
    {1, glm::vec3(gemDist *  sin(2 * PI / 7), 0.0f, gemDist * cos(2 * PI / 7))},
    {2, glm::vec3(gemDist *  sin(4 * PI / 7), 0.0f, gemDist * cos(4 * PI / 7))},
    {3, glm::vec3(gemDist *  sin(6 * PI / 7), 0.0f, gemDist * cos(6 * PI / 7))},
    {4, glm::vec3(gemDist * -sin(6 * PI / 7), 0.0f, gemDist * cos(6 * PI / 7))},
    {5, glm::vec3(gemDist * -sin(4 * PI / 7), 0.0f, gemDist * cos(4 * PI / 7))},
    {6, glm::vec3(gemDist * -sin(2 * PI / 7), 0.0f, gemDist * cos(2 * PI / 7))},
};

const float colorMult = 2.1f;

glm::vec3 colors[7] = {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IT356 Final Project (Derek Jennings)", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    Shader shader("../../src/shader/gem.vert", "../../src/shader/gem.frag");
    Shader skyboxShader("../../src/shader/skybox.vert", "../../src/shader/skybox.frag");
    Shader wireShader("../../src/shader/gem.vert", "../../src/shader/basic.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    //float cubeVertices[] = {
    //    // positions          // normals
    //    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    //     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    //     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    //     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    //    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    //    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    //
    //    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    //     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    //    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    //    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    //
    //    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    //    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    //    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    //    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    //    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    //    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    //
    //     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    //     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
    //     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
    //     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
    //     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    //     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    //
    //    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    //     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    //     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    //     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    //    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    //    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
    //
    //    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    //     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    //     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    //    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    //    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    //};

    /// 6-sided gem
    /// "right" = +x, "up" = +y, "front" = +z
    float gemVertices[] = {
        //positions
            //normals

        //-innerRadius / 2, innerHeight, -sqrt3 * innerRadius / 2,  // left top back
        //-innerRadius / 2, innerHeight,  sqrt3* innerRadius / 2,  // left top front
        // innerRadius / 2, innerHeight, -sqrt3 * innerRadius / 2,  // right top back
        // innerRadius / 2, innerHeight,  sqrt3* innerRadius / 2,  // right top front

        //-outerRadius / 2, outerHeight, -sqrt3 * outerRadius / 2,  // left top back
        //-outerRadius / 2, outerHeight,  sqrt3* outerRadius / 2,  // left top front
        // outerRadius / 2, outerHeight, -sqrt3 * outerRadius / 2,  // right top back
        // outerRadius / 2, outerHeight,  sqrt3* outerRadius / 2,  // right top front

        //////////////////////////// LIST OF VERTICES /////////////////////////////////
        
        ///// top (hexagon)
        //-innerRadius, innerHeight, 0.0f,  // left top center (inner)
        //-innerCorner.x, innerHeight, -innerCorner.y,  // left top back
        //-innerCorner.x, innerHeight,  innerCorner.y,  // left top front
        // innerCorner.x, innerHeight, -innerCorner.y,  // right top back
        // innerCorner.x, innerHeight,  innerCorner.y,  // right top front
        // innerRadius, innerHeight, 0.0f,  // right top center

        ///// Sides
        //-outerRadius, outerHeight, 0.0f,  // left side center
        //-outerCorner.x, outerHeight, -outerCorner.y,  // left side back
        //-outerCorner.x, outerHeight,  outerCorner.y,  // left side front
        // outerCorner.x, outerHeight, -outerCorner.y,  // right side back
        // outerCorner.x, outerHeight,  outerCorner.y,  // right side front
        // outerRadius, outerHeight, 0.0f,  // right side center

        ///// Bottom point
        //0.0f, pointHeight, 0.0f,  // bottom center point

        ///// Midpoints; used for setting up the inner triangles of a face
        //-midCorner.x, outerHeight, -midCorner.y,  // leftish side backish
        // 0.0f, outerHeight, -outerCorner.y,  // center side back
        // midCorner.x, outerHeight, -midCorner.y,  // rightish side backish
        // midCorner.x, outerHeight,  midCorner.y,  // rightish side frontish
        // 0.0f, outerHeight, outerCorner.y,  // center side front
        //-midCorner.x, outerHeight,  midCorner.y,  // leftish side frontish

        ///////////////////////////////////////////////////////////////////////////////

        /// Face: top (hexagon)
        // Tri: left
        -innerRadius, innerHeight, 0.0f,  // left top center
            0.0f, 1.0f, 0.0f,  // normal pointing up
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
            0.0f, 1.0f, 0.0f,
        -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
            0.0f, 1.0f, 0.0f,
        // Tri: left center back
        -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
            0.0f, 1.0f, 0.0f,
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
            0.0f, 1.0f, 0.0f,
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            0.0f, 1.0f, 0.0f,
        // Tri: right center front
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            0.0f, 1.0f, 0.0f,
        -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
            0.0f, 1.0f, 0.0f,
         innerCorner.x, innerHeight,  innerCorner.y,  // right top front
            0.0f, 1.0f, 0.0f,
        // Tri: right
         innerCorner.x, innerHeight,  innerCorner.y,  // right top front
            0.0f, 1.0f, 0.0f,
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            0.0f, 1.0f, 0.0f,
         innerRadius, innerHeight, 0.0f,  // right top center
            0.0f, 1.0f, 0.0f,

        /// Face: left upper side back
        // Tri: leftest lower backish
        -innerRadius, innerHeight, 0.0f,  // left top center
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        -outerRadius, outerHeight, 0.0f,  // left side center
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        -midCorner.x, outerHeight, -midCorner.y,  // leftish side backish
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        // Tri: lefter upper backisher
        -midCorner.x, outerHeight, -midCorner.y,  // leftish side backish
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        -innerRadius, innerHeight, 0.0f,  // left top center
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        // Tri: leftish lower backerest
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        -midCorner.x, outerHeight, -midCorner.y,  // leftish side backish
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
            //-0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL left upper back
            -midNormal.x, midNormal.y, -midNormal.z,  // left upper back
        
        /// Face: center upper side back
        // Tri: left lower back
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
         0.0f, outerHeight, -outerCorner.y,  // center side back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
        // Tri: center upper back
         0.0f, outerHeight, -outerCorner.y,  // center side back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
        // Tri: right lower back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
         0.0f, outerHeight, -outerCorner.y,  // center side back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
            //0.0f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL center upper back
            0.0f, midCenterNormal.y, -midCenterNormal.z,  // center upper back
        
        /// Face: right upper side back
        // Tri: rightish lower backerest
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
         midCorner.x, outerHeight, -midCorner.y,  // rightish side backish
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
        // Tri: right upper backer
         midCorner.x, outerHeight, -midCorner.y,  // rightish side backish
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
         innerRadius, innerHeight, 0.0f,  // right top center
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
        // Tri: rightest lower backish
         innerRadius, innerHeight, 0.0f,  // right top center
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
            midNormal.x, midNormal.y, -midNormal.z,  // right upper back
         midCorner.x, outerHeight, -midCorner.y,  // rightish side backish
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
             midNormal.x, midNormal.y, -midNormal.z,  // right upper back
         outerRadius, outerHeight, 0.0f,  // right side center
            //0.707f, 0.707f, -0.707f,  // PLACEHOLDER NORMAL right upper back
             midNormal.x, midNormal.y, -midNormal.z,  // right upper back
        
        /// Face: right upper side front
        // Tri: rightest lower frontish
         outerRadius, outerHeight, 0.0f,  // right side center
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
         innerRadius, innerHeight, 0.0f,  // right top center
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
         midCorner.x, outerHeight, midCorner.y,  // rightish side frontish
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
        // Tri: righter upper fronter
         midCorner.x, outerHeight, midCorner.y,  // rightish side frontish
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
         innerRadius, innerHeight, 0.0f,  // right top center
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
         innerCorner.x, innerHeight, innerCorner.y,  // right top front
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
        // Tri: rightish lower fronterest
         innerCorner.x, innerHeight, innerCorner.y,  // right top front
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
         midCorner.x, outerHeight, midCorner.y,  // rightish side frontish
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
         outerCorner.x, outerHeight, outerCorner.y,  // right side front
            //0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL right upper front
            midNormal.x, midNormal.y, midNormal.z,  // right upper front
        
        /// Face: center upper side front
        // Tri: rightish lower side front
         outerCorner.x, outerHeight, outerCorner.y,  // right side front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
         innerCorner.x, innerHeight, innerCorner.y,  // right top front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
         0.0f, outerHeight, outerCorner.y,  // center side front
             //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
        // Tri: center upper side front
         0.0f, outerHeight, outerCorner.y,  // center side front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
         innerCorner.x, innerHeight, innerCorner.y,  // right top front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
             0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
         -innerCorner.x, innerHeight, innerCorner.y,  // left top front
             //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
        // Tri: leftish lower side front
         -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
         0.0f, outerHeight, outerCorner.y,  // center side front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
         -outerCorner.x, outerHeight,  outerCorner.y,  // left side front
            //0.0f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL center upper front
            0.0f, midCenterNormal.y, midCenterNormal.z,  // center upper front
        
        /// Face: left upper side front
        // Tri: leftish lower side fronterest
         -outerCorner.x, outerHeight, outerCorner.y,  // left side front
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
         -innerCorner.x, innerHeight, innerCorner.y,  // left top front
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
         -midCorner.x, outerHeight, midCorner.y,  // leftish side frontish
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
        // Tri: lefter upper side fronter
         -midCorner.x, outerHeight, midCorner.y,  // leftish side frontish
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
         -innerCorner.x, innerHeight, innerCorner.y,  // left top front
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
         -innerRadius, innerHeight, 0.0f,  // left top center (inner)
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
        // Tri: leftest lower side frontish
         -innerRadius, innerHeight, 0.0f,  // left top center (inner)
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
         -midCorner.x, outerHeight, midCorner.y,  // leftish side frontish
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front
         -outerRadius, outerHeight, 0.0f,  // left side center
            //-0.707f, 0.707f, 0.707f,  // PLACEHOLDER NORMAL left upper front
            -midNormal.x, midNormal.y, midNormal.z,  // left upper front

        /// Faces: lower sides
        // Face: left lower back
        -outerRadius, outerHeight, 0.0f,  // left side center
            //-0.707f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL left lower back
            -bottomNormal.x, bottomNormal.y, -bottomNormal.z,  // left lower back
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
            //-0.707f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL left lower back
            -bottomNormal.x, bottomNormal.y, -bottomNormal.z,  // left lower back
        0.0f, pointHeight, 0.0f,  // bottom center point
            //-0.707f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL left lower back
            -bottomNormal.x, bottomNormal.y, -bottomNormal.z,  // left lower back
        // Face: center lower back
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
            //0.0f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL center lower back
            0.0f, bottomCenterNormal.y, -bottomCenterNormal.z,  // center lower back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
            //0.0f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL center lower back
            0.0f, bottomCenterNormal.y, -bottomCenterNormal.z,  // center lower back
        0.0f, pointHeight, 0.0f,  // bottom center point
            //0.0f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL center lower back
            0.0f, bottomCenterNormal.y, -bottomCenterNormal.z,  // center lower back
        // Face: right lower back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
             //0.707f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL right lower back
            bottomNormal.x, bottomNormal.y, -bottomNormal.z,  // right lower back
         outerRadius, outerHeight, 0.0f,  // right side center
             //0.707f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL right lower back
            bottomNormal.x, bottomNormal.y, -bottomNormal.z,  // right lower back
         0.0f, pointHeight, 0.0f,  // bottom center point
             //0.707f, -0.707f, -0.707f,  // PLACEHOLDER NORMAL right lower back
            bottomNormal.x, bottomNormal.y, -bottomNormal.z,  // right lower back
        // Face: right lower front
         outerRadius, outerHeight, 0.0f,  // right side center
             //0.707f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL right lower front
            bottomNormal.x, bottomNormal.y, bottomNormal.z,  // right lower front
         outerCorner.x, outerHeight,  outerCorner.y,  // right side front
             //0.707f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL right lower front
            bottomNormal.x, bottomNormal.y, bottomNormal.z,  // right lower front
         0.0f, pointHeight, 0.0f,  // bottom center point
             //0.707f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL right lower front
            bottomNormal.x, bottomNormal.y, bottomNormal.z,  // right lower front
        // Face: center lower front
         outerCorner.x, outerHeight,  outerCorner.y,  // right side front
            //0.0f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL center lower front
            0.0f, bottomCenterNormal.y, bottomCenterNormal.z,  // right lower front
        -outerCorner.x, outerHeight,  outerCorner.y,  // left side front
            //0.0f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL center lower front
            0.0f, bottomCenterNormal.y, bottomCenterNormal.z,  // right lower front
         0.0f, pointHeight, 0.0f,  // bottom center point
            //0.0f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL center lower front
            0.0f, bottomCenterNormal.y, bottomCenterNormal.z,  // right lower front
        // Face: left lower front
        -outerCorner.x, outerHeight, outerCorner.y,  // left side front
             //-0.707f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL left lower front
            -bottomNormal.x, bottomNormal.y, bottomNormal.z,  // right lower front
        -outerRadius, outerHeight, 0.0f,  // left side center
             //-0.707f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL left lower front
            -bottomNormal.x, bottomNormal.y, bottomNormal.z,  // right lower front
        0.0f, pointHeight, 0.0f,  // bottom center point
             //-0.707f, -0.707f, 0.707f,  // PLACEHOLDER NORMAL left lower front
            -bottomNormal.x, bottomNormal.y, bottomNormal.z,  // right lower front

    };
    float gemEdges[] = {
        /// top (hexagon)
        // Edge: left back
        -innerRadius, innerHeight, 0.0f,  // left top center
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
        // Edge: back
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
        // Edge: right back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
         innerRadius, innerHeight, 0.0f,  // right top center
        // Edge: right front
         innerRadius, innerHeight, 0.0f,  // right top center
         innerCorner.x, innerHeight,  innerCorner.y,  // right top front
        // Edge: front
         innerCorner.x, innerHeight,  innerCorner.y,  // right top front
        -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
        // Edge: left front
        -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
        -innerRadius, innerHeight, 0.0f,  // left top center

        /// upper sides (verticals)
        // Edge: left center
        -innerRadius, innerHeight, 0.0f,  // left top center
        -outerRadius, outerHeight, 0.0f,  // left side center
        // Edge: left back
        -innerCorner.x, innerHeight, -innerCorner.y,  // left top back
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
        // Edge: right back
         innerCorner.x, innerHeight, -innerCorner.y,  // right top back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
        // Edge: right center
         innerRadius, innerHeight, 0.0f,  // right top center
         outerRadius, outerHeight, 0.0f,  // right side center
        // Edge: right front
         innerCorner.x, innerHeight,  innerCorner.y,  // right top front
         outerCorner.x, outerHeight,  outerCorner.y,  // right side front
        // Edge: left front
        -innerCorner.x, innerHeight,  innerCorner.y,  // left top front
        -outerCorner.x, outerHeight,  outerCorner.y,  // left side front

        /// sides (horizontals)
        // Edge: left side back
        -outerRadius, outerHeight, 0.0f,  // left side center
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
        // Edge: center side back
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
        // Edge: right side back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
         outerRadius, outerHeight, 0.0f,  // right side center
        // Edge: right side front
         outerRadius, outerHeight, 0.0f,  // right side center
         outerCorner.x, outerHeight,  outerCorner.y,  // right side front
        // Edge: center side front
         outerCorner.x, outerHeight,  outerCorner.y,  // right side front
        -outerCorner.x, outerHeight,  outerCorner.y,  // left side front
        // Edge: left side front
        -outerCorner.x, outerHeight,  outerCorner.y,  // left side front
        -outerRadius, outerHeight, 0.0f,  // left side center

        /// lower sides
        // Edge: left lower center
        -outerRadius, outerHeight, 0.0f,  // left side center
         0.0f, pointHeight, 0.0f,  // bottom center point
        // Edge: left lower back
         0.0f, pointHeight, 0.0f,  // bottom center point
        -outerCorner.x, outerHeight, -outerCorner.y,  // left side back
        // Edge: right lower back
         outerCorner.x, outerHeight, -outerCorner.y,  // right side back
         0.0f, pointHeight, 0.0f,  // bottom center point
        // Edge: right lower center
         0.0f, pointHeight, 0.0f,  // bottom center point
         outerRadius, outerHeight, 0.0f,  // right side center
        // Edge: right lower front
         outerCorner.x, outerHeight,  outerCorner.y,  // right side front
         0.0f, pointHeight, 0.0f,  // bottom center point
        // Edge: left lower front
         0.0f, pointHeight, 0.0f,  // bottom center point
        -outerCorner.x, outerHeight, outerCorner.y,  // left side front
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // gem VAO
    unsigned int gemVAO, gemVBO;
    glGenVertexArrays(1, &gemVAO);
    glGenBuffers(1, &gemVBO);
    glBindVertexArray(gemVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gemVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gemVertices), &gemVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // gem edges VAO
    unsigned int edgeVAO, edgeVBO;
    glGenVertexArrays(1, &edgeVAO);
    glGenBuffers(1, &edgeVBO);
    glBindVertexArray(edgeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gemEdges), &gemEdges, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
    {
        FileSystem::getPath("resources/textures/land_skybox/vz_classic_land_right.png"),
        FileSystem::getPath("resources/textures/land_skybox/vz_classic_land_left.png"),
        FileSystem::getPath("resources/textures/land_skybox/vz_classic_land_up.png"),
        FileSystem::getPath("resources/textures/land_skybox/vz_classic_land_down.png"),
        FileSystem::getPath("resources/textures/land_skybox/vz_classic_land_front.png"),
        FileSystem::getPath("resources/textures/land_skybox/vz_classic_land_back.png")
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("skybox", 0);
    //shader.setVec3("objectColor", glm::vec3(0.7f, 1.5f, 0.7f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.8f, 1.0f));

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    
    glLineWidth(lineWidth);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        // sort the transparent gems before rendering
        std::map<float, std::pair<int, glm::vec3>> sorted;
        for (std::map<int, glm::vec3>::reverse_iterator it = gemLocs.rbegin(); it != gemLocs.rend(); ++it)
        {
            float distance = glm::length(camera.Position - it->second);
            sorted[distance] = std::pair<int, glm::vec3> {it->first, it->second};
        }

        // Render the gems in order
        for (std::map<float, std::pair<int, glm::vec3>>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            shader.use();

            // Material
            shader.setVec3("material.ambient", glm::vec3(0.0f, 0.1f, 0.06f));
            shader.setVec3("material.diffuse", glm::vec3(0.07568f, 0.61424f, 0.07568f));
            shader.setVec3("material.specular", glm::vec3(0.633f, 0.727811f, 0.633f));
            shader.setFloat("material.shininess", 6.0f);

            // Lighting
            glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
            glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); //decrease the influence
            glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); //low influence
            shader.setVec3("light.position", lightPos);
            shader.setVec3("light.ambient", ambientColor);
            shader.setVec3("light.diffuse", diffuseColor);
            shader.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));

            // Transformations
            glm::mat4 model = glm::mat4(1.0f);
            if (revolveMode >= 1)
            {
                angle += deltaTime / rotateDivisor;
            }
            if (revolveMode >= 2) {
                revolveOffset += deltaTime / revolveDivisor;

                // Rotating first makes them all orbit around the origin
                model = glm::rotate(model, revolveOffset, glm::vec3(0.0f, 1.0f, 0.0f));
                
                float gemTimeOffset = (PI * 2.0f * (float)(it->second.first)) / 7.0f;
                //// Calculate new positions
                //float newX = cos(revolveOffset + gemTimeOffset) * gemDist;
                //gemLocs[it->second.first].x = newX;
                //float newZ = sin(revolveOffset + gemTimeOffset) * gemDist;
                //gemLocs[it->second.first].z = newZ;

                // move gems up and down
                if (revolveMode >= 3) {
                    // Update height location of each gem
                    float gemHeightOffset = revolveHeight * sin(PI * (revolveOffset * revolveHeightSpeedMult) + gemTimeOffset);
                    it->second.second.y = gemHeightOffset;
                    gemLocs[it->second.first].y = gemHeightOffset;
                    //model = glm::translate(model, glm::vec3(0.0f, gemHeightOffset, 0.0f));
                }
                model = glm::translate(model, it->second.second); // move to initial position
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else {
                // Rotating first makes them all orbit around the origin
                model = glm::rotate(model, revolveOffset, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, it->second.second); // move to initial position
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            //model = glm::translate(model, glm::vec3(it->second.second.x, it->second.second.y, it->second.second.z));

            shader.setMat4("model", model);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("cameraPos", camera.Position);
            //shader.setVec3("objectColor", glm::vec3(1.0f, 1.8f, 1.0f));
            shader.setVec3("objectColor", colors[it->second.first] * colorMult);
            glBindVertexArray(gemVAO);
            glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            glDrawArrays(GL_TRIANGLES, 0, 84);
            //glBindVertexArray(0);

            // wireframe edges
            if (wireframe_enabled) {
                // Adjust line width based on distance
                if (it->first > lineWidthMaxDistance)
                    glLineWidth(1);
                else
                    glLineWidth(lineWidth - (lineWidth * it->first) / lineWidthMaxDistance);

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                //shader.setVec3("objectColor", glm::vec3(0.0f, 0.0f, 0.0f));
                wireShader.use();
                wireShader.setMat4("model", model);
                wireShader.setMat4("view", view);
                wireShader.setMat4("projection", projection);
                wireShader.setVec3("cameraPos", camera.Position);
                //wireShader.setVec3("myColor", glm::vec3(0.0f, 0.0f, 0.0f));
                //wireShader.setVec3("myColor", glm::vec3(0.23f, 0.48f, 0.28f));
                wireShader.setVec3("myColor", (colors[it->second.first]-0.5f)*0.5f + 0.5f);
                glBindVertexArray(edgeVAO);
                glActiveTexture(GL_TEXTURE0);
                //glDrawArrays(GL_TRIANGLES, 0, 84);
                glDrawArrays(GL_LINES, 0, 48);
                glBindVertexArray(0);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &gemVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &gemVBO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    shiftDis = deltaTime / 1.0f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, shiftDis);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, shiftDis);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, shiftDis);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, shiftDis);

    // Rotate the gem
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!rButtonLock)
            revolveMode++;
        rButtonLock = true;
        if (revolveMode > 3)
            revolveMode = 0;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        revolveMode = 0;
    }
    else {
        rButtonLock = false;
    }

    // Up/Down movement
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, shiftDis);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, shiftDis);

    // Enable wireframe
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!lButtonLock) {
            if (wireframe_enabled)
                wireframe_enabled = false;
            else
                wireframe_enabled = true;
            lButtonLock = true;
        }
    }
    else
        lButtonLock = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

