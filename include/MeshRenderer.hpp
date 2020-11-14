#ifndef MESHRENDERER_HPP
#define MESHRENDERER_HPP


// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <set>
#include <algorithm>

// Include Glad
#include <glad/glad.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>

#include "Shader.hpp"
#include "Mesh.hpp"

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

class MeshRenderer {
public:
    // constructor
    MeshRenderer(unsigned int shaderID, Mesh& mesh);
    
    // destructor
    ~MeshRenderer();

    // draw mesh
    void draw(int camPlacement, float lightPlacement, bool show_valence, bool lighting) const;
    
    // update mesh's vertices
    void updateBuffers();
    
    // clean all vao / vbo / shader
    void cleanUp();
    
    // mesh to be rendered
    Mesh tridimodel;

private:
    GLuint VertexArrayID;
    GLuint programID;
    
    GLuint MatrixID, ViewMatrixID, ModelMatrixID;
    GLuint LightID;
    
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint valencefieldbuffer;
    GLuint elementbuffer;
};
#endif
