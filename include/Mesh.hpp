#ifndef MESH_HPP
#define MESH_HPP

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <float.h>
#include <algorithm>
#include <memory>
// Include GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include "Octree.hpp"
#include <unordered_map>

// BOX structure for bounding box
struct BOX {
    glm::vec2 xpos, ypos, zpos;
    BOX():xpos(glm::vec2(0.0)), ypos(glm::vec2(0.0)), zpos(glm::vec2(0.0)){}
    // return dimensions of the box
    glm::vec3 dimension() {return glm::vec3(xpos.y - xpos.x, ypos.y - ypos.x, zpos.y - zpos.x);}
};

class Mesh {
public:
    // constructors
    Mesh();
    Mesh(const char * filename);
    // destructor
    ~Mesh();

    // compute normals for each vertex depending on weight_type criteria
    // @weight_type : 0 for uniform, 1 for area of triangles, 2 for angle of triangle
    void compute_smooth_vertex_normals(int weight_type);

    // calculate the number of neighbors of a vertex and
    // normalize it depending on maximum valence of the mesh
    void compute_vertex_valences();

    // simplify vertices of the mesh this based on given resolution
    void simplify (unsigned int resolution);

    // simplify vertices of the mesh this based on octree
    // @numOfPerLeafVertices :  number of vertices per leaf
    void adaptiveSimplify (unsigned int numOfPerLeafVertices);


    // variables of a mesh
    //
    // P0 ---- P1       indices :           0 1 2 1 2 3
    //  \    /  \       triangles:          (0, 1, 2) (1, 2, 3)
    //   \  /    \      indexed_vertices:   P0 P1 P2 P3
    //    P2 --- P3     valences:            2  3  3  2
    //
    int weight = 0;
    std::vector<float> valence_field;
    std::vector<unsigned short> indices;
    std::vector<unsigned int> valences;
    std::vector<glm::vec3> indexed_vertices, indexed_normals;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<std::vector<unsigned short> > triangles;
    BOX bounding_box;

    unsigned int getNumberOfVertices(){return indexed_vertices.size();}

private:
    // pointer to octree
    std::shared_ptr<Octree> m_octree;

    // normalize valences using maximul valence of the mesh
    void generate_valence_field ();

    // compute normals for each triangles and stock in triangle_normals
    void compute_triangle_normals ( const std::vector<glm::vec3> & vertices,
                                    const std::vector<std::vector<unsigned short> > & triangles,
                                    std::vector<glm::vec3> & triangle_normals);

    // compute normals for each vertex depending on weight_type criteria
    // and stock in vertex_normals
    // @weight_type : 0 for uniform, 1 for area of triangles, 2 for angle of triangle
    void compute_smooth_vertex_normals (const std::vector<glm::vec3> & vertices,
                                        const std::vector<std::vector<unsigned short> > & triangles,
                                        unsigned int weight_type,
                                        std::vector<glm::vec3> & vertex_normals);

    // create a list of numbers of vertices around each one
    void collect_one_ring ( const std::vector<glm::vec3> & vertices,
                            const std::vector<std::vector<unsigned short> > & triangles,
                            std::vector<std::vector<unsigned short> > & one_ring) ;

    // assign number of vertices around to corresponding vertex
    void compute_vertex_valences (  const std::vector<glm::vec3> & vertices,
                                    const std::vector<std::vector<unsigned short> > & triangles,
                                    std::vector<unsigned int> & valences) ;

    // recursive function of adaptiveSimplify function
    // using the Quadratic Error Function
    void adaptiveSimplifyRec (std::shared_ptr<Octree> oc, unsigned int numOfPerLeafVertices, std::vector<unsigned int> & vertices_to_repr, std::vector<glm::vec3> & repr_indexed_vertices, std::vector<std::vector<unsigned short>> in_triangles);

    // load file of format OFF with given filename
    bool load_OFF_file (const std::string & filename, std::vector< glm::vec3 > & vertices,
                        std::vector< glm::vec3 > & normals, std::vector< unsigned short > & indices,
                        std::vector< std::vector<unsigned short > > & triangles, glm::vec2 & xpos,
                        glm::vec2 & ypos, glm::vec2 & zpos);

    // calculate plane equation using 3 vertices
    glm::vec4 equation_plane(float x1, float y1, float z1,
                             float x2, float y2, float z2,
                             float x3, float y3, float z3);
};

#endif
