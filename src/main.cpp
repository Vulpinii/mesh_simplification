#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdio>
#include <string>

#ifdef __linux__
    #include <unistd.h>
    #define GetCurrentDir getcwd
#else
    #include <direct.h>
    #define GetCurrentDir _getcwd
#endif

// include project files
#include "Mesh.hpp"
#include "MeshRenderer.hpp"

// settings
#define GRID        0
#define OCTREE      1
#define MIN_GRID    2
#define MAX_GRID    100
#define MIN_OCTREE  5
#define MAX_OCTREE  150

unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
[[maybe_unused]] float lastX, lastY;
bool firstMouse = true;
double cursorXpos, cursorYpos;
float generationTime = 0.0f;
bool regenerate(false), backToOriginal(false);
unsigned short currentMode = GRID;
int maxNumberPerLeaf(MIN_OCTREE), girdResolution(MAX_GRID);
bool showValence(false), wireFrame(false), lighting(true);
int camPlacement(0); float lightPlacement(0.5);
unsigned int meshVertices(0);
std::string loadedObjName, lastLoadedObjName;

// math
bool nearlyEqual(double a, double b, double epsilon);

// System
std::string getCurrentWorkingDirectory ();
void little_sleep(std::chrono::milliseconds us);

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// GUI Staff
void CherryTheme() ;
void renderGui();

// **************
// MAIN
int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow((int)SCR_WIDTH, (int)SCR_HEIGHT, "Mesh Simplification", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // setup callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // get current working directory
    std::string currentPath = getCurrentWorkingDirectory();
	std::cout << "Current working directory is " << currentPath << std::endl;

    // create mesh
    Mesh tridimodel = Mesh((currentPath+"/assets/models/teddy.off").c_str());
    Mesh originalmodel = tridimodel;

    // process
    tridimodel.compute_smooth_vertex_normals(0);
    tridimodel.compute_vertex_valences();

    // create shader
    Shader shader = Shader((currentPath+"/assets/shaders/vertex_shader.glsl").c_str(),
                           (currentPath+"/assets/shaders/fragment_shader.glsl").c_str());

    // create renderer
    MeshRenderer mrenderer = MeshRenderer(shader.ID, tridimodel);


    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // setup Dear ImGui theme
    CherryTheme();
    ImGui::GetStyle().WindowMinSize = ImVec2((float)SCR_WIDTH*0.2f, (float)SCR_HEIGHT);
    ImGui::GetIO().FontGlobalScale = 1 + float(SCR_WIDTH)/(1920);
    ImGui::GetIO().Fonts->AddFontFromFileTTF("../assets/fonts/Roboto-Light.ttf", 16.0f);

    // ------------------
    bool notsimplify = false;

    // RENDER LOOP -----
    while (!glfwWindowShouldClose(window))
    {
        if(loadedObjName != lastLoadedObjName){
            lastLoadedObjName = loadedObjName;
            tridimodel = Mesh((currentPath+"/assets/models/"+loadedObjName+".off").c_str());
            maxNumberPerLeaf = MIN_OCTREE;
            girdResolution = MAX_GRID;
            notsimplify = true;
            originalmodel = tridimodel;
            regenerate = true;
        }
        if(regenerate){
            regenerate = false;
            tridimodel = originalmodel;
            if (!notsimplify) {
                switch (currentMode) {
                    case GRID :
                        tridimodel.simplify(girdResolution);
                        break;
                    case OCTREE :
                        tridimodel.adaptiveSimplify(maxNumberPerLeaf);
                        break;
                    default: break;
                }
            }
            tridimodel.compute_smooth_vertex_normals(0);
            tridimodel.compute_vertex_valences();
            mrenderer.tridimodel = tridimodel;
            mrenderer.updateBuffers();
            notsimplify = false;
        }
        if(backToOriginal){
            backToOriginal = false;
            tridimodel = originalmodel;
            tridimodel.compute_smooth_vertex_normals(0);
            tridimodel.compute_vertex_valences();
            mrenderer.tridimodel = tridimodel;
            mrenderer.updateBuffers();
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
        
        glClearColor(50.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        if(wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // renderer mesh
        mrenderer.draw(camPlacement, lightPlacement, showValence, lighting);
        meshVertices = tridimodel.getNumberOfVertices();

        // feed inputs to dear imgui, start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        renderGui();

        // Render dear imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        little_sleep(std::chrono::milliseconds(25));
    }
    // clean up
    mrenderer.cleanUp();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    ImGui::GetIO().FontGlobalScale = 1 + float(SCR_WIDTH)/(1920);
    ImGui::GetStyle().WindowMinSize = ImVec2((float)SCR_WIDTH*0.2f, (float)SCR_HEIGHT);
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float) xpos;
        lastY = (float) ypos;
        firstMouse = false;
    }

    lastX = (float) xpos;
    lastY = (float) ypos;
}


// glfw: mouse button callback
// ---------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) 
    {
       //getting cursor position
       glfwGetCursorPos(window, &cursorXpos, &cursorYpos);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
// From https://github.com/procedural/gpulib/blob/master/gpulib_imgui.h
void CherryTheme() {
    // cherry colors, 3 intensities
#define HI(v)   ImVec4(0.35f, 0.35f, 0.35f, v)
#define MED(v)  ImVec4(0.20, 0.20, 0.20, v)
#define LOW(v)  ImVec4(0.25, 0.25, 0.25, v)
    // backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.15, 0.15f, 0.15f, v)
    // text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

    auto &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text]                  = TEXT(0.78f);
    style.Colors[ImGuiCol_TextDisabled]          = TEXT(0.28f);
    style.Colors[ImGuiCol_WindowBg]              = BG( 0.9f);//ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]               = BG( 0.58f);
    style.Colors[ImGuiCol_PopupBg]               = BG( 0.9f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = HI( 0.25f); // slider background
    style.Colors[ImGuiCol_FrameBgHovered]        = MED( 0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = MED( 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = LOW( 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]         = HI( 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = BG( 0.75f);
    style.Colors[ImGuiCol_MenuBarBg]             = MED( 0.57f);
    style.Colors[ImGuiCol_ScrollbarBg]           = BG( 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = MED( 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = MED( 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    style.Colors[ImGuiCol_ButtonHovered]         = MED( 0.86f);
    style.Colors[ImGuiCol_ButtonActive]          = MED( 1.00f);
    style.Colors[ImGuiCol_Header]                = MED( 0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = MED( 0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = HI( 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = MED( 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = MED( 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = TEXT(0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = MED( 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = TEXT(0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = MED( 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = MED( 0.43f);
    style.Colors[ImGuiCol_ModalWindowDarkening]  = BG( 0.73f);

    style.WindowPadding            = ImVec2(6, 4);
    style.WindowRounding           = 0.0f;
    style.FramePadding             = ImVec2(5, 2);
    style.FrameRounding            = 3.0f;
    style.ItemSpacing              = ImVec2(7, 1);
    style.ItemInnerSpacing         = ImVec2(1, 1);
    style.TouchExtraPadding        = ImVec2(0, 0);
    style.IndentSpacing            = 6.0f;
    style.ScrollbarSize            = 12.0f;
    style.ScrollbarRounding        = 16.0f;
    style.GrabMinSize              = 20.0f;
    style.GrabRounding             = 2.0f;

    style.WindowTitleAlign.x = 0.50f;

    style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
    style.FrameBorderSize = 0.0f;
    style.WindowBorderSize = 1.0f;
}


void little_sleep(std::chrono::milliseconds us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
        //if (!awake) std::cout << "sleep" << std::endl;
        //else {std::cout << "-------- AWAKE" << std::endl; break;}
    } while (std::chrono::high_resolution_clock::now() < end);
}

bool nearlyEqual(double a, double b, double epsilon)
{
    // if the distance between a and b is less than epsilon, then a and b are "close enough"
    return std::abs(a - b) <= epsilon;
}

std::string getCurrentWorkingDirectory (){
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))){return std::string();}

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    std::string currentPath = std::string(cCurrentPath);
    int found = currentPath.find("build");
    currentPath = currentPath.substr(0, found);
    unsigned int i = 0;
    while (i < currentPath.size()){
        if (currentPath[i] == '\\') {currentPath[i] = '/';}
        ++i;
    }
    return currentPath;
}

void renderGui(){
    if(ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove)){
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetWindowSize(ImVec2(400, (float)SCR_HEIGHT));
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::Text(("Number of vertices : "+std::to_string(meshVertices)).c_str());
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Text("Structure : ");
        ImGui::SameLine();
        if(ImGui::RadioButton("Grid", currentMode == GRID) ) currentMode = GRID;
        ImGui::SameLine();
        if(ImGui::RadioButton("Octree", currentMode == OCTREE) ) currentMode = OCTREE;

        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        if(currentMode == GRID) {
            ImGui::Text("Grid resolution");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.96f);
            ImGui::SliderInt("", &girdResolution, MIN_GRID, MAX_GRID);
        }

        if(currentMode == OCTREE) {
            ImGui::Text("Max vertices per leaf");
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.96f);
            ImGui::SliderInt("", &maxNumberPerLeaf, MIN_OCTREE, MAX_OCTREE);
        }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        {
            ImGui::SetCursorPosX(ImGui::GetWindowSize().x*0.2f);
            if (ImGui::Button("Simplify", ImVec2(ImGui::GetWindowSize().x*0.5f, 0.0f))) regenerate = true;
        }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        ImGui::Checkbox("Valence", &showValence);
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
        ImGui::Checkbox("Wireframe", &wireFrame);
        ImGui::Dummy(ImVec2(0.0f, 3.0f));
        ImGui::Checkbox("Lighting", &lighting);

        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        ImGui::SetNextTreeNodeOpen(true);
        if(ImGui::CollapsingHeader("Rotation"))
        {
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.77f);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SliderInt("Model", &camPlacement, 0, 360);
            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SliderFloat("Light", &lightPlacement, 0, 2);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Load")) {
                if (ImGui::MenuItem("Arma")){loadedObjName = "arma1";}
                if (ImGui::MenuItem("Camel")){loadedObjName = "camel";}
                if (ImGui::MenuItem("Elephant")){loadedObjName = "elephant";}
                if (ImGui::MenuItem("Sphere")){loadedObjName = "sphere";}
                if (ImGui::MenuItem("Suzanne")){loadedObjName = "suzanne";}
                if (ImGui::MenuItem("Teddy")){loadedObjName = "teddy";}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Original")) {
                backToOriginal = true;
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();

}