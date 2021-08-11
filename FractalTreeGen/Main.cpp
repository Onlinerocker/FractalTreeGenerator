#include <iostream>
#include <vector>
#include <deque>

#include "SDL.h"
#include "GL\glew.h"
#include "SDL_opengl.h"

#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include <glm/glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Rendering.h"

#define PI 3.14159265359f

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

bool fullscreen = false;
bool running = true;

GLuint vao, vbo, ebo;
GLuint program, vert, frag;

struct Branch {
    float radRatio;
    float angX;
    float angY;

    glm::vec4 r;
    glm::vec3 base;
    glm::vec3 rUp;
    glm::vec3 dir;

    int faces;
    int level;
};

/* 
*
* FUNCTION: createBranch
* 
* [Params]
* 
* br: Structure describing the branch to create
* 
* verts: Vector to be filled by function with the meshes vertices
* inds: Vector to be filled by function with the meshes indices
* 
* normal: vec3 to fill calculated normal with
* up: vec3 to fill calculated up with
* right: vec3 to fill calculated right with
* 
*/

void createBranch(const Branch& br, std::vector<Rendering::Vertex>& verts, std::vector<GLuint>& inds, glm::vec3& normal, glm::vec3& up, glm::vec3& right)
{
    size_t vSize = verts.size();
    float faces = static_cast<float>(br.faces);
    float radDelta = (2.0f * PI) / faces;
    glm::mat4 rrot = glm::rotate(glm::mat4(1.0f), radDelta, normalize(br.rUp));
    glm::mat4 rrot1 = glm::rotate(glm::mat4(1.0f), radDelta, normalize(br.dir));
    glm::vec4 r1 = br.r * br.radRatio;
    glm::vec4 r = br.r;

    Rendering::Vertex vBranch{ glm::vec3(0), glm::vec3(0), glm::vec3(0.34, 0.23, 0.1) };
    for (int i = 0; i < faces; i++) {

        glm::vec3 a = br.base + glm::vec3(r.x, r.y, r.z);
        glm::vec3 b = br.base + br.dir + (glm::vec3(r1.x, r1.y, r1.z));
        r = rrot * r;
        r1 = rrot1 * r1;
        glm::vec3 c = br.base + glm::vec3(r.x, r.y, r.z);
        glm::vec3 d = br.base + br.dir + (glm::vec3(r1.x, r1.y, r1.z));

        up = b - a;
        right = c - a;
        normal = glm::cross(up, right);
        normal = glm::normalize(normal);
        vBranch.norm = normal;

        vBranch.pos = a; verts.push_back(vBranch);
        vBranch.pos = b; verts.push_back(vBranch);
        vBranch.pos = c; verts.push_back(vBranch);
        vBranch.pos = d;  verts.push_back(vBranch);

        /* Tri 1*/
        inds.push_back(vSize + i * 4);
        inds.push_back(vSize + i * 4 + 1);
        inds.push_back(vSize + i * 4 + 3);

        /* Tri 2 */
        inds.push_back(vSize + i * 4);
        inds.push_back(vSize + i * 4 + 3);
        inds.push_back(vSize + i * 4 + 2);
    }
}

/*
*
* FUNCTION: generateMesh
*
* [Params]
*
* verts: Vector to be filled by function with the meshes vertices
* inds: Vector to be filled by function with the meshes indices
*
* radRatio: The ratio of a given branches base radius to top radius
* angX: Degree (euler) each branch will rotate around the X-axis per iteration
* angY: Degree (euler) each branch will roate around the Y-axis per iteration
*
* r: Vector representing the radius of the branches base
* base: The base's center location
* rUp: Vector pointing directly up from the branche's base
* dir: Direction the branch is pointing
*
* faceCount: Amount of quad faces making up the branch
* maxLevel: Maximum recursion depth
* level: Current recursion depth
* leafColor: Color of the leaves
*
 */
void generateMesh(std::vector<Rendering::Vertex>& verts, std::vector<GLuint>& inds, float radRatio, float angX, float angY, glm::vec4 r, 
                  glm::vec3 base, glm::vec3 rUp, glm::vec3 dir, int faceCount, int maxLevel, int& level, const glm::vec3& leafColor)
{
    /* Normal, up, and right vector to be used by leaves later */
    glm::vec3 normal(0);
    glm::vec3 up(0);
    glm::vec3 right(0);

    /* Setup the trunk */
    Branch br{ radRatio, angX, angY, r, base, rUp, dir, faceCount, level };
    std::deque<Branch> q;
    q.push_back(br);

    /* Try to create branches while the queue is not empty*/
    while (!q.empty())
    {
        Branch& curBranch = q.front();
        createBranch(curBranch, verts, inds, normal, up, right);
        q.pop_front();

        /* If we aren't at the max iteration, queue more branches */
        if (curBranch.level < maxLevel)
        {
            /* Branch direction rotation matrices*/
            glm::mat4 rdir = glm::rotate(glm::mat4(1.0), glm::radians(curBranch.angY), glm::vec3(0, 0, 1));
            glm::mat4 rdir1 = glm::rotate(glm::mat4(1.0), glm::radians(-curBranch.angY), glm::vec3(0, 0, 1));
            glm::mat4 rdir2 = glm::rotate(glm::mat4(1.0), glm::radians(curBranch.angX), glm::vec3(1, 0, 0));
            glm::mat4 rdir3 = glm::rotate(glm::mat4(1.0), glm::radians(-curBranch.angX), glm::vec3(1, 0, 0));

            /* Apply branch direction rotation matrices*/
            glm::vec4 newDir = rdir * glm::vec4(curBranch.dir, 1.0);
            glm::vec4 newDir1 = rdir1 * glm::vec4(curBranch.dir, 1.0);
            glm::vec4 newDir2 = rdir2 * glm::vec4(curBranch.dir, 1.0);
            glm::vec4 newDir3 = rdir3 * glm::vec4(curBranch.dir, 1.0);

            Branch br1{ curBranch.radRatio, curBranch.angX, curBranch.angY, curBranch.r * radRatio, curBranch.base + curBranch.dir, curBranch.dir, glm::vec3(newDir.x, newDir.y, newDir.z), curBranch.faces * 0.75, curBranch.level+1 };
            Branch br2{ curBranch.radRatio, curBranch.angX, curBranch.angY, curBranch.r * radRatio, curBranch.base + curBranch.dir, curBranch.dir, glm::vec3(newDir1.x, newDir1.y, newDir1.z), curBranch.faces * 0.75, curBranch.level+1 };
            Branch br3{ curBranch.radRatio, curBranch.angX, curBranch.angY, curBranch.r * radRatio, curBranch.base + curBranch.dir, curBranch.dir, glm::vec3(newDir2.x, newDir2.y, newDir2.z), curBranch.faces * 0.75, curBranch.level+1 };
            Branch br4{ curBranch.radRatio, curBranch.angX, curBranch.angY, curBranch.r * radRatio, curBranch.base + curBranch.dir, curBranch.dir, glm::vec3(newDir3.x, newDir3.y, newDir3.z), curBranch.faces * 0.75, curBranch.level+1 };

            q.push_back(br1);
            q.push_back(br2);
            q.push_back(br3);
            q.push_back(br4);
        }
        /* Else create leaf */
        else {
            size_t vSize = verts.size();

            float lo = 0.01 * length(r);
            float l = 16.0 * length(r);
            l = glm::clamp(l, 0.2f, 0.4f);

            up = normalize(up);
            right = normalize(right);

            glm::vec3 aMod = glm::normalize(-right) * l;
            glm::vec3 bMod = glm::normalize(up) * l;
            glm::vec3 cMod = glm::normalize(right) * l;
            glm::vec3 dMod = glm::normalize(-up) * l;

            glm::vec3 color = leafColor;
            Rendering::Vertex v{ glm::vec3(0), glm::vec3(0), color };
            auto AddLeaf = [&verts, &inds, &curBranch, &vSize, &lo, &color, &v](glm::vec3& aMod, glm::vec3& bMod,
                glm::vec3& cMod, glm::vec3& dMod, float n) {
                    glm::vec3 a = curBranch.base + curBranch.dir + aMod;
                    glm::vec3 b = curBranch.base + curBranch.dir + bMod;
                    glm::vec3 c = curBranch.base + curBranch.dir + cMod;
                    glm::vec3 d = curBranch.base + curBranch.dir + dMod;

                    glm::vec3 normal = n * glm::cross(glm::normalize(b - a), glm::normalize(d - a));
                    normal = glm::normalize(normal);

                    a -= (normal * lo);
                    b -= (normal * lo);
                    c -= (normal * lo);
                    d -= (normal * lo);

                    v.norm = normal;

                    v.pos = a; verts.push_back(v);
                    v.pos = b; verts.push_back(v);
                    v.pos = c; verts.push_back(v);
                    v.pos = d;  verts.push_back(v);

                    /* Tri 1*/
                    inds.push_back(vSize);
                    inds.push_back(vSize + 1);
                    inds.push_back(vSize + 2);

                    /* Tri 2 */
                    inds.push_back(vSize);
                    inds.push_back(vSize + 2);
                    inds.push_back(vSize + 3);
            };

            /* Front side of leaf */
            AddLeaf(aMod, bMod, cMod, dMod, 1.0);

            vSize = verts.size();

            /* Back side of leaf */
            AddLeaf(aMod, bMod, cMod, dMod, -1.0);
        }

    }
}

int main(int argc, char* argv[])
{
    SDL_Window* window;
    SDL_GLContext context;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        std::cout << "[ERR] SDL could not init" << std::endl;

    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    SCREEN_WIDTH = SCREEN_WIDTH > displayMode.w ? displayMode.w : SCREEN_WIDTH;
    SCREEN_HEIGHT = SCREEN_HEIGHT > displayMode.h ? displayMode.h : SCREEN_HEIGHT;

    window = SDL_CreateWindow("Procedrual Tree Generator", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
    context = SDL_GL_CreateContext(window); //create GL context

    if (!context) std::cout << "[ERR] SDL Could not create GL context!" << std::endl;

    int vsync = 1;
    int swapSuc = SDL_GL_SetSwapInterval(vsync);
    if (swapSuc >= 0) std::cout << "VSync is " << (vsync ? "on\n" : "off\n");
    else std::cout << SDL_GetError() << std::endl;

    // GLEW SETUP
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "[ERR] GLEW: " << err << std::endl;
        return 1;
    }

    std::cout << "OpenGL v" << glGetString(GL_VERSION) << "\n";

    /* Setup ImGUI */

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");

    /* Generate mesh and setup vertex buffer */
    std::vector<Rendering::Vertex> verts;
    std::vector<GLuint> inds;
    glm::vec4 r(0, 0, 1.0, 0.0);

    /* Generate inital mesh */
    int lvl = 0;
    int lastIt = 5;
    glm::vec3 leafColor(0.03, 0.48, 0.09);
    verts.reserve(10000000);
    inds.reserve(15000000);
    generateMesh(verts, inds, 0.5f, 25.0f, 25.0f, r, glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0.0f, 3.5f, 0.0f)*r.z, 50, 5, lvl, leafColor);
    
    std::cout << "s " << verts.size() << "\n";

    /* Create shaders and setup buffers */
    Rendering::createShaderProgram(frag, vert, program);
    Rendering::createBuffers(vao, vbo, ebo);
    Rendering::setupVertexBuffer(&verts[0], verts.size(), &inds[0], inds.size(), vao, vbo, ebo);

    /* Get shader uniform locations */
    glUseProgram(program);
    GLint uni = glGetUniformLocation(program, "transform");
    GLint rot = glGetUniformLocation(program, "rot");

    /* Setup MVP matrix */
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, 0, 3));
   
    /* Settings */
    glm::vec3 cameraPos(0,10.0f,75.0f);
    float modelYRot = 0.0f;
    float radRatio = 0.5f;
    int iterations = 5;
    float angX = 25.0f;
    float angY = 25.0f;
    int faces = 50;

    /* Main Loop */
    while (running) {

        /* Input */
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type)
            {

            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    running = false;
                    break;
                }

            }
        }

        /* Bind and Clear Framebuffer */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        //glEnable(GL_FRAMEBUFFER_SRGB);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Draw mesh */
        {
            view = glm::lookAt(cameraPos, cameraPos + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
            
            glm::mat4 mod = glm::rotate(model, glm::radians(modelYRot), glm::vec3(0, 1, 0));
            glm::mat4 mat = proj * view * mod;

            glUniformMatrix4fv(uni, 1, GL_FALSE, glm::value_ptr(mat));
            glUniformMatrix4fv(rot, 1, GL_FALSE, glm::value_ptr(mod));
            Rendering::draw(vao, inds.size());
        }
   
        /* ImGui Interface */
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();

            ImGui::Begin("Settings");
            ImGui::SliderFloat3("Camera Position", &cameraPos[0], -100.0f, 100.0f);
            ImGui::SliderFloat("Object Rotation", &modelYRot, -180.0f, 180.0f);

            ImGui::InputInt("Iterations", &iterations);
            ImGui::SliderInt("Faces", &faces, 50, 250);
            ImGui::SliderFloat("Radius Ratio", &radRatio, 0.0f, 1.0f);
            ImGui::SliderFloat("X Angle", &angX, 0.0f, 60.0f);
            ImGui::SliderFloat("Y Angle", &angY, 0.0f, 60.0f);
            ImGui::ColorPicker3("Leaf Color", &leafColor.x);
            if (ImGui::Button("Recompute"))
            {
                verts.clear();
                inds.clear();
                lvl = 0;

                generateMesh(verts, inds, radRatio, angX, angY, r, glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0.0f, 3.5f, 0.0f)* r.z, faces, iterations, lvl, leafColor);
                Rendering::setupVertexBuffer(&verts[0], verts.size(), &inds[0], inds.size(), vao, vbo, ebo);
            }

            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }


        SDL_GL_SwapWindow(window);
    }

    /* Clean up */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

