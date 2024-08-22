#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadTexture(const char *path);
void renderScene(const Shader &shader,const glm::vec3& lightPos);
void renderModel(const Shader &modelShader,glm::vec3 modelPosition,float modelScale,int sc);
void renderCube();
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool shadows = true;
bool shadowsKeyPressed = true;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 carpetPosition = glm::vec3(1.0f, -2.53f, 0.0f);
float carpetScale =15.0f;
// ako je carpet model rug a ne kilim
//glm::vec3 carpetPosition = glm::vec3(0.0f, -2.5f, 0.0f);
//float carpetScale =4.2f;
glm::vec3 bedPosition = glm::vec3(1.0f, -0.5f, -3.0f);
float bedScale =1.9f;
glm::vec3 closetPosition = glm::vec3(3.0f, -2.55f, 4.4f);
float closetScale =1.9f;
glm::vec3 lampPosition = glm::vec3(1.0f, 2.5f, 0.0f);
float lampScale =1.0f;
glm::vec3 doorPosition = glm::vec3(-2.0f, -2.7f, 4.5f);
float doorScale =0.2f;
glm::vec3 boxPosition = glm::vec3(-6.0f, -2.6f, -3.0f);
float boxScale =0.5f;
glm::vec3 picturePosition = glm::vec3(-6.8f, 1.0f, 0.0f);
float pictureScale =1.5f;
bool ImGuiEnabled = false;
bool CameraMouseMovementUpdateEnabled = true;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
//    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // ako hocu da ogranicim pomeranje kursorom (hover) stavim na normal
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
//    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------

    Shader shader("resources/shaders/point_shadows.vs", "resources/shaders/point_shadows.fs");
    Shader simpleDepthShader("resources/shaders/point_shadows_depth.vs", "resources/shaders/point_shadows_depth.fs", "resources/shaders/point_shadows_depth.gs");
    Shader modelShader("resources/shaders/model_lighting.vs", "resources/shaders/model_lighting.fs");
    Shader instancingShader("resources/shaders/instancing.vs", "resources/shaders/instancing.fs");

    // load textures
    unsigned int wallTexture = loadTexture(FileSystem::getPath("resources/textures/wall_white.jpg").c_str());
    // load models
    // -----------
    Model bedModel("resources/objects/children_bed/scene.gltf");
    bedModel.SetShaderTextureNamePrefix("material.");
    Model closetModel("resources/objects/old_closet/scene.gltf");
    closetModel.SetShaderTextureNamePrefix("material.");
    Model lampModel("resources/objects/ceiling_lamp/scene.gltf"); //crveno iz nekog razloga
    lampModel.SetShaderTextureNamePrefix("material.");
    Model carpetModel("resources/objects/kilim/scene.gltf");
//    Model carpetModel("resources/objects/rug/scene.gltf");
    carpetModel.SetShaderTextureNamePrefix("material.");
    Model doorModel("resources/objects/stuff_in_my_room_door/scene.gltf");
    doorModel.SetShaderTextureNamePrefix("material.");
    Model boxModel("resources/objects/toy_box/scene.gltf");
    boxModel.SetShaderTextureNamePrefix("material.");
    Model flowerModel("resources/objects/cosmos_flower/scene.gltf");
    flowerModel.SetShaderTextureNamePrefix("material.");
//    Model windowModel("resources/objects/w1/window/w1.obj"); //crveno sve
    Model pictureModel("resources/objects/donut_picture_frame/scene.gltf");
    pictureModel.SetShaderTextureNamePrefix("material.");
//    Model pictureModel("resources/objects/window1/CW1a.obj");
//    pictureModel.SetShaderTextureNamePrefix("material.");

    //---

    // generate a large list of semi-random model transformation matrices
    // ------------------------------------------------------------------
    unsigned int amount = 100; // change amount
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[amount];
    float y_limit = 2.0f;
    float z_limit = 4.5f;
    int index = 0;
    float y_offset = (y_limit*2)/9;
    float z_offset = (z_limit*2)/9;
// dole desno: (7.1f, -2.0f, 4.5f);
// gore desno: (7.1f, 2.0f, 4.5f);
// gore levo: (7.1f, 2.0f, -4.5f);
// dole levo: (7.1f, -2.0f, -4.5f);
    for(float i=-y_limit;i<=y_limit;i+=y_offset){
        for(float j=-z_limit;j<=z_limit;j+=z_offset){
            glm::mat4 model = glm::mat4(1.0f);
            float x = 7.1f;
            float y = i;
            float z = j;
            if((index/10)%2==0){
                z+= z_offset/2;
            }
            model = glm::translate(model, glm::vec3(x, y, z));
            float scale = 0.05f;
            model = glm::scale(model, glm::vec3(scale));
            model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.0f, -1.0f, 0.0f));
            modelMatrices[index] = model;
            index++;
        }
    }
    // configure instanced array
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < flowerModel.meshes.size(); i++)
    {
        unsigned int VAO = flowerModel.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
//----

    PointLight pointLight;
    pointLight.position = glm::vec3(0.0f, 1.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(1.0, 1.0, 1.0);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(2, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    // nece se koristiti za renderovanje boja nego samo za renderovanje dubina
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "ERROR::FRAMEBUFFER:: Depth map framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("diffuseTexture1", 2);
    shader.setInt("depthMap", 1);

    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
    lightPos = lampPosition;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render scene
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms; //ovde se nalaze svih 6 transformacija koje transformisu u koordinate prostora svetla
        //lookAt matrica je jedinstvena za svaku stranu (view matrica) -> pozicija kamere, tacka u koju kamera gleda i vektor na gore
        // pozicija je uvek ista, za svaku stranu
        // menja se tacka u koju gledamo - to ce biti centar svake strane cubemape
        // vektor na gore isto zavisi od strane do strane
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", far_plane);
        simpleDepthShader.setVec3("lightPos", lightPos);
        renderScene(simpleDepthShader,lightPos);

        //ako je carpet model rug a ne kilim
//        renderModel(simpleDepthShader,carpetPosition,carpetScale, 2);
        renderModel(simpleDepthShader,carpetPosition,carpetScale, 0);
        carpetModel.Draw(simpleDepthShader);
        renderModel(simpleDepthShader,bedPosition,bedScale, 0);
        bedModel.Draw(simpleDepthShader);
        renderModel(simpleDepthShader,closetPosition,closetScale,1);
        closetModel.Draw(simpleDepthShader);
        renderModel(simpleDepthShader,lampPosition,lampScale,0);
        lampModel.Draw(simpleDepthShader);
        renderModel(simpleDepthShader,doorPosition,doorScale,0);
        doorModel.Draw(simpleDepthShader);
        renderModel(simpleDepthShader,boxPosition,boxScale,2);
        boxModel.Draw(simpleDepthShader);
        renderModel(simpleDepthShader, picturePosition, pictureScale, 4);
        pictureModel.Draw(simpleDepthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set lighting uniforms
        shader.setVec3("lightPos",lightPos);
        shader.setVec3("viewPos",camera.Position);
        shader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader,lightPos);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        modelShader.use();

        // render bed model
        glDisable(GL_CULL_FACE);
        renderModel(modelShader,bedPosition,bedScale,0);
        modelShader.setInt("reverse_normals", 1);
        bedModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        // render carpet model
        glDisable(GL_CULL_FACE);
        //ako je carpet model rug a ne kilim
//        renderModel(modelShader,carpetPosition,carpetScale,2);
        renderModel(modelShader,carpetPosition,carpetScale,0);
        modelShader.setInt("reverse_normals", 1);
        carpetModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        // render door model
        glDisable(GL_CULL_FACE);
        renderModel(modelShader,doorPosition,doorScale,0);
        modelShader.setInt("reverse_normals", 1);
        doorModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        // render box model
        glDisable(GL_CULL_FACE);
        renderModel(modelShader,boxPosition,boxScale,2);
        modelShader.setInt("reverse_normals", 1);
        boxModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        //render closet model
        glDisable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 1);
        renderModel(modelShader,closetPosition,closetScale,1);
        closetModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        //render lamp model
        renderModel(modelShader,lampPosition,lampScale,0);
        glDisable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 1);
        lampModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        // render window model
        glDisable(GL_CULL_FACE);
        renderModel(modelShader,picturePosition,pictureScale,4);
        modelShader.setInt("reverse_normals", 1);
        pictureModel.Draw(modelShader);
        glEnable(GL_CULL_FACE);
        modelShader.setInt("reverse_normals", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap); //KOMENTAR 4. isto je i bez ove dve linije

        //----
        instancingShader.use();
        instancingShader.setMat4("projection", projection);
        instancingShader.setMat4("view", view);
        instancingShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flowerModel.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (unsigned int i = 0; i < flowerModel.meshes.size(); i++)
        {
            glBindVertexArray(flowerModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, flowerModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }
        //---
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed)
    {
        shadows = !shadows;
        shadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        shadowsKeyPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
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
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        ImGuiEnabled = !ImGuiEnabled;
        if (ImGuiEnabled) {
            CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}


void renderModel(const Shader &modelShader,glm::vec3 modelPosition,float modelScale,int sc)
{
    float near_plane = 1.0f;
    float far_plane = 25.0f;
    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

    PointLight pointLight;
    pointLight.position = glm::vec3(0.0f, 1.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(1.0, 1.0, 1.0);
    pointLight.specular = glm::vec3(0.0, 0.0, 0.0);


    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;
    modelShader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
    modelShader.setFloat("far_plane", far_plane);
    pointLight.position = lightPos; //glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
    modelShader.setVec3("pointLight.position", pointLight.position);
    modelShader.setVec3("pointLight.ambient", pointLight.ambient);
    modelShader.setVec3("pointLight.diffuse", pointLight.diffuse);
    modelShader.setVec3("pointLight.specular", pointLight.specular);
    modelShader.setFloat("pointLight.constant", pointLight.constant);
    modelShader.setFloat("pointLight.linear", pointLight.linear);
    modelShader.setFloat("pointLight.quadratic", pointLight.quadratic);
    modelShader.setVec3("viewPosition", camera.Position);
    modelShader.setFloat("material.shininess", 32.0f);
    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                  (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    modelShader.setMat4("projection", projection);
    modelShader.setMat4("view", view);
    modelShader.setInt("depthMap", 1);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model,
                           modelPosition); // translate it down so it's at the center of the scene
    if(sc==1){
        //closet
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(-1.0f, 0.0f, 0.0f));
    }
    if(sc==2){
        //box
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.0f, 0.0f, 1.0f));

    }
    if(sc==3){
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.0f, -1.0f, 0.0f));
    }
    if(sc==4){
        //picture
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.0f, 1.0f, 0.0f));
    }

    model = glm::scale(model, glm::vec3(modelScale));    // it's a bit too big for our scene, so scale it down
    modelShader.setMat4("model", model);
}

// renders the 3D scene
// --------------------
void renderScene(const Shader &shader, const glm::vec3& lightPos)
{
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
// scale to have lower height of the room
    glm::vec3 scaleFactors = glm::vec3(7.0f, 2.5f, 5.0f);
    model = glm::scale(model, scaleFactors);
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    renderCube(); // room
    shader.setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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
