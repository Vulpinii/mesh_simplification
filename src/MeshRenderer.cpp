#include "MeshRenderer.hpp"

MeshRenderer::MeshRenderer(unsigned int shaderID, Mesh& mesh)
    : VertexArrayID(0), vertexbuffer(0), uvbuffer(0), normalbuffer(0), valencefieldbuffer(0), elementbuffer(0)
{
    tridimodel = mesh;
    
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    programID = shaderID;
    
    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_vertices.size() * sizeof(glm::vec3), &tridimodel.indexed_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_uvs.size() * sizeof(glm::vec2), &tridimodel.indexed_uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_normals.size() * sizeof(glm::vec3), &tridimodel.indexed_normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &valencefieldbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, valencefieldbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.valence_field.size() * sizeof(float), &tridimodel.valence_field[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tridimodel.indices.size() * sizeof(unsigned short), &tridimodel.indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
}

MeshRenderer::~MeshRenderer() = default;

void MeshRenderer::draw(int camPlacement, float lightPlacement, bool show_valence, bool lighting) const
{
    // Use our shader
    glUseProgram(programID);
    
    // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 ViewMatrix       = glm::lookAt(
                glm::vec3(0+cos(0.5*3.1415)*3.0,0,0+sin(0.5*3.1415)*3.0), // Camera is at (4,3,3), in World Space
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 ModelMatrix      = glm::mat4(1.0f);
    float decalageX = -(tridimodel.bounding_box.xpos.y - ((tridimodel.bounding_box.xpos.y + abs(tridimodel.bounding_box.xpos.x)) / 2.0f) ); 
    float decalageY = -(tridimodel.bounding_box.ypos.y - ((tridimodel.bounding_box.ypos.y + abs(tridimodel.bounding_box.ypos.x)) / 2.0f) ); 
    float decalageZ = -(tridimodel.bounding_box.zpos.y - ((tridimodel.bounding_box.zpos.y + abs(tridimodel.bounding_box.zpos.x)) / 2.0f) ); 
    ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0/(tridimodel.bounding_box.ypos.y+decalageY)));
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(decalageX + 0.5, decalageY, decalageZ -0.5) );
    ModelMatrix = glm::rotate(ModelMatrix, glm::radians((float)camPlacement), glm::vec3(0,1,0));
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

    glm::vec3 lightPos = glm::vec3(4,4,4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    
    lightPos = glm::vec3(0.0f+cos((double)lightPlacement)*4.0f,4.0f,0.0f+sin((double)lightPlacement)*4.0f);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    glUniform1i(glGetUniformLocation(programID, "show_valence"), (int)show_valence); 
    glUniform1i(glGetUniformLocation(programID, "no_lighting"), (int)!lighting); 

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
                0,                  // attribute
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                nullptr            // array buffer offset
                );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
                1,                                // attribute
                2,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                nullptr                          // array buffer offset
                );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
                2,                                // attribute
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                nullptr                          // array buffer offset
                );


    // A faire : créer le 4eme attribute buffer : valence_field
    //***********************************************//
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, valencefieldbuffer);
    glVertexAttribPointer(
                3,                                // attribute
                1,                                // size
                GL_FLOAT,                         // type
                GL_TRUE,                         // normalized?
                0,                                // stride
                nullptr                          // array buffer offset
                );
    //***********************************************//

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
                GL_TRIANGLES,      // mode
                tridimodel.indices.size(),    // count
                GL_UNSIGNED_SHORT,   // type
                nullptr           // element array buffer offset
                );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void MeshRenderer::updateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_vertices.size() * sizeof(glm::vec3), &tridimodel.indexed_vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_uvs.size() * sizeof(glm::vec2), &tridimodel.indexed_uvs[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.indexed_normals.size() * sizeof(glm::vec3), &tridimodel.indexed_normals[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, valencefieldbuffer);
    glBufferData(GL_ARRAY_BUFFER, tridimodel.valence_field.size() * sizeof(float), &tridimodel.valence_field[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tridimodel.indices.size() * sizeof(unsigned short), &tridimodel.indices[0] , GL_STATIC_DRAW);
}

void MeshRenderer::cleanUp()
{
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);

    //***********************************************//
    glDeleteBuffers(1, &valencefieldbuffer);

    //***********************************************//
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
}
