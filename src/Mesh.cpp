#include "Mesh.hpp"

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// constructors
Mesh::Mesh()= default;

Mesh::Mesh(const char * filename)
{
    bounding_box = BOX();

    load_OFF_file(filename, indexed_vertices, indexed_normals, indices, triangles,
                  bounding_box.xpos, bounding_box.ypos, bounding_box.zpos);
    std::cout << "**********\nBounding box :" << std::endl;
    std::cout << "(xmin, xmax) = (" << bounding_box.xpos.x << ", " << bounding_box.xpos.y << ")" << std::endl;
    std::cout << "(ymin, ymax) = (" << bounding_box.ypos.x << ", " << bounding_box.ypos.y << ")" << std::endl;
    std::cout << "(zmin, zmax) = (" << bounding_box.zpos.x << ", " << bounding_box.zpos.y << ")" << std::endl;
    std::cout << "**********" << std::endl;
    indexed_uvs.resize(indexed_vertices.size(), glm::vec2(1.)); //List vide de UV
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// destructor
Mesh::~Mesh() = default;

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// normal computation
void Mesh::compute_smooth_vertex_normals(int weight_type)
{
    compute_smooth_vertex_normals(indexed_vertices, triangles, weight_type, indexed_normals);
}

void Mesh::compute_vertex_valences()
{
    compute_vertex_valences( indexed_vertices, triangles, valences );
    generate_valence_field();
}


void Mesh::generate_valence_field ()
{
    valence_field.resize(valences.size(), 0.0f);

    float max_valence = 0.0;
    for (unsigned int i = 0 ; i < valences.size(); ++i)
    {
        if (valences.at(i) > max_valence) max_valence = valences.at(i);
    }
    if (max_valence < 1.0) max_valence = 1.0;
    for (unsigned int i = 0 ; i < valences.size(); ++i)
    {
        valence_field.at(i) = valences.at(i) / max_valence;
    }
}


void Mesh::compute_triangle_normals (const std::vector<glm::vec3> & vertices,
                                     const std::vector<std::vector<unsigned short> > & triangles,
                                     std::vector<glm::vec3> & triangle_normals)
{
    for(auto triangle : triangles)
    {
        glm::vec3 p0 = vertices.at(triangle.at(0));
        glm::vec3 p1 = vertices.at(triangle.at(1));
        glm::vec3 p2 = vertices.at(triangle.at(2));

        glm::vec3 n = glm::normalize(glm::cross(p1-p0, p2-p0));

        triangle_normals.push_back(n);
    }
}

void Mesh::compute_smooth_vertex_normals (const std::vector<glm::vec3> & vertices,
                                          const std::vector<std::vector<unsigned short> > & triangles,
                                          unsigned int weight_type, //0 uniforme, 1 area of triangles, 2 angle of triangle
                                          std::vector<glm::vec3> & vertex_normals){

    vertex_normals.clear();
    vertex_normals.resize(vertices.size(), glm::vec3(0.0));

    std::vector<glm::vec3> triangle_normals;
    std::vector<glm::vec3> triangle_angles;
    std::vector<float> triangle_surface, point_aire_triangles, point_angles_triangles;

    compute_triangle_normals(vertices, triangles, triangle_normals);
    triangle_angles.resize(triangles.size(), glm::vec3(0.0));
    triangle_surface.resize(triangles.size(),0.0);
    point_aire_triangles.resize(vertices.size(),0.0f);
    point_angles_triangles.resize(vertices.size(), 0.0f);

    for(unsigned int i = 0; i < triangles.size(); ++i)
    {
        glm::vec3 p0 = vertices.at(triangles.at(i).at(0));
        glm::vec3 p1 = vertices.at(triangles.at(i).at(1));
        glm::vec3 p2 = vertices.at(triangles.at(i).at(2));

        switch(weight_type){
            case 0 :
                // we add normal of the current triangle to each vertex
                vertex_normals.at(triangles.at(i).at(0)) += triangle_normals.at(i);
                vertex_normals.at(triangles.at(i).at(1)) += triangle_normals.at(i);
                vertex_normals.at(triangles.at(i).at(2)) += triangle_normals.at(i);
                break;

            case 1:
                // we add area of a triangle to each vertices
                triangle_surface.at(i) = glm::dot(p1-p0, p2-p0)/2.0f;
                point_aire_triangles.at(triangles.at(i).at(0)) += triangle_surface.at(i);
                point_aire_triangles.at(triangles.at(i).at(1)) += triangle_surface.at(i);
                point_aire_triangles.at(triangles.at(i).at(2)) += triangle_surface.at(i);
                break;

            case 2:
                // we add near triangle's area with current triangle of a vertex
                triangle_angles.at(i).x = acos(glm::radians(glm::dot(p1-p0, p2-p0)/
                                                            (glm::length(p1-p0) * glm::length(p2-p0))));
                triangle_angles.at(i).y = acos(glm::radians(glm::dot(p2-p1, p0-p1)/
                                                            (glm::length(p0-p1) * glm::length(p2-p1))));
                triangle_angles.at(i).z = acos(glm::radians(glm::dot(p0-p2, p1-p2)/
                                                            (glm::length(p0-p2) * glm::length(p1-p2))));

                point_angles_triangles.at(triangles.at(i).at(0)) += triangle_angles.at(i).x;
                point_angles_triangles.at(triangles.at(i).at(1)) += triangle_angles.at(i).y;
                point_angles_triangles.at(triangles.at(i).at(2)) += triangle_angles.at(i).z;
                break;
        }
    }

    switch(weight_type){
        case 1 :
            // we divide the weight of the normal for each triangle with the area
            for(int i = 0 ; i < triangles.size() ; i++){
                vertex_normals.at(triangles.at(i).at(0)) += triangle_normals.at(i)*
                                                            (triangle_surface.at(i)/point_aire_triangles.at(triangles.at(i).at(0)));
                vertex_normals.at(triangles.at(i).at(1)) += triangle_normals.at(i)*
                                                            (triangle_surface.at(i)/point_aire_triangles.at(triangles.at(i).at(1)));
                vertex_normals.at(triangles.at(i).at(2)) += triangle_normals.at(i)*
                                                            (triangle_surface.at(i)/point_aire_triangles.at(triangles.at(i).at(2)));
            }
            break;

        case 2 :
            // we devide the weight of the normal with each vertex depending of the angle of
            // near triangles normalize with max angle
            for(int i = 0 ; i < triangles.size() ; i++){
                vertex_normals.at(triangles.at(i).at(0)) += triangle_normals.at(i)*
                                                            (triangle_angles.at(i).x/point_angles_triangles.at(triangles.at(i).at(0)));
                vertex_normals.at(triangles.at(i).at(1)) += triangle_normals.at(i)*
                                                            (triangle_angles.at(i).y/point_angles_triangles.at(triangles.at(i).at(1)));
                vertex_normals.at(triangles.at(i).at(2)) += triangle_normals.at(i)*
                                                            (triangle_angles.at(i).z/point_angles_triangles.at(triangles.at(i).at(2)));
            }
            break;
    }

    // we nomalize normals
    for(unsigned int i = 0 ; i < vertex_normals.size(); ++i)
    {
        vertex_normals.at(i) = glm::normalize(vertex_normals.at(i));
    }
}

void Mesh::collect_one_ring (const std::vector<glm::vec3> & vertices,
                             const std::vector<std::vector<unsigned short> > & triangles,
                             std::vector<std::vector<unsigned short> > & one_ring)
{
    one_ring.resize(vertices.size());

    for (unsigned int i = 0 ; i < triangles.size() ; ++i)
    {
        if (std::find(one_ring.at(triangles.at(i).at(0)).begin(),
                      one_ring.at(triangles.at(i).at(0)).end(),
                      triangles.at(i).at(1)) == one_ring.at(triangles.at(i).at(0)).end())
            one_ring.at(triangles.at(i).at(0)).push_back(triangles.at(i).at(1));

        if (std::find(one_ring.at(triangles.at(i).at(0)).begin(),
                      one_ring.at(triangles.at(i).at(0)).end(),
                      triangles.at(i).at(2)) == one_ring.at(triangles.at(i).at(0)).end())
            one_ring.at(triangles.at(i).at(0)).push_back(triangles.at(i).at(2));

        if (std::find(one_ring.at(triangles.at(i).at(1)).begin(),
                      one_ring.at(triangles.at(i).at(1)).end(),
                      triangles.at(i).at(0)) == one_ring.at(triangles.at(i).at(1)).end())
            one_ring.at(triangles.at(i).at(1)).push_back(triangles.at(i).at(0));

        if (std::find(one_ring.at(triangles.at(i).at(1)).begin(),
                      one_ring.at(triangles.at(i).at(1)).end(),
                      triangles.at(i).at(2)) == one_ring.at(triangles.at(i).at(1)).end())
            one_ring.at(triangles.at(i).at(1)).push_back(triangles.at(i).at(2));

        if (std::find(one_ring.at(triangles.at(i).at(2)).begin(),
                      one_ring.at(triangles.at(i).at(2)).end(),
                      triangles.at(i).at(0)) == one_ring.at(triangles.at(i).at(2)).end())
            one_ring.at(triangles.at(i).at(2)).push_back(triangles.at(i).at(0));

        if (std::find(one_ring.at(triangles.at(i).at(2)).begin(),
                      one_ring.at(triangles.at(i).at(2)).end(),
                      triangles.at(i).at(1)) == one_ring.at(triangles.at(i).at(2)).end())
            one_ring.at(triangles.at(i).at(2)).push_back(triangles.at(i).at(1));
    }
}


void Mesh::compute_vertex_valences (const std::vector<glm::vec3> & vertices,
                                    const std::vector<std::vector<unsigned short> > & triangles,
                                    std::vector<unsigned int> & valences)
{
    valences.resize(vertices.size());
    std::vector<std::vector<unsigned short> >  one_ring;
    collect_one_ring(vertices, triangles, one_ring);

    for (unsigned int i = 0; i < vertices.size(); ++i){
        valences.at(i) = one_ring.at(i).size();
    }
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// load off file


bool Mesh::load_OFF_file(const std::string & filename, std::vector< glm::vec3 > & vertices,
                         std::vector< glm::vec3 > & normals, std::vector< unsigned short > & indices,
                         std::vector< std::vector<unsigned short > > & triangles,
                         glm::vec2 & xpos, glm::vec2 & ypos, glm::vec2 & zpos)
{
    std::ifstream myfile; myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << "Failure to open " <<filename << " file"<< std::endl;
        return false;
    }

    std::string isOFFfile; myfile >> isOFFfile;
    if(isOFFfile != "OFF")
    {
        std::cerr << "File " << filename << " isn't an OFF format file" << std::endl;
        myfile.close();
        return false;
    }

    int numberOfVertices , numberOfFaces , numberOfEdges;
    myfile >> numberOfVertices >> numberOfFaces >> numberOfEdges;

    vertices.resize(numberOfVertices);
    normals.resize(numberOfVertices);

    std::vector< std::vector<glm::vec3> > normalsTmp;
    normalsTmp.resize(numberOfVertices);


    for( int v = 0 ; v < numberOfVertices ; ++v )
    {
        glm::vec3 vertex;
        myfile >> vertex.x >> vertex.y >> vertex.z;
        vertices[v] = vertex;

        if(v == 0)
        {
            xpos.x = vertex.x; xpos.y = vertex.x;
            ypos.x = vertex.y; ypos.y = vertex.y;
            zpos.x = vertex.z; zpos.y = vertex.z;
        }
        else
        {
            if(xpos.x > vertex.x) xpos.x = vertex.x;
            if(xpos.y < vertex.x) xpos.y = vertex.x;

            if(ypos.x > vertex.y) ypos.x = vertex.y;
            if(ypos.y < vertex.y) ypos.y = vertex.y;

            if(zpos.x > vertex.z) zpos.x = vertex.z;
            if(zpos.y < vertex.z) zpos.y = vertex.z;
        }
    }


    for( int f = 0 ; f < numberOfFaces ; ++f )
    {
        int numberOfVerticesOnFace;
        myfile >> numberOfVerticesOnFace;
        if( numberOfVerticesOnFace == 3 )
        {
            unsigned short v1 , v2 , v3;
            std::vector< unsigned short > v;
            myfile >> v1 >> v2 >> v3;

            v.push_back(v1);
            v.push_back(v2);
            v.push_back(v3);

            triangles.push_back(v);

            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);

            glm::vec3 normal;
            glm::vec3 tmp1 = vertices[v2] - vertices[v1];
            glm::vec3 tmp2 = vertices[v3] - vertices[v1];
            normal = glm::normalize(glm::cross(tmp1,tmp2));
            normalsTmp[v1].push_back(normal);
            normalsTmp[v2].push_back(normal);
            normalsTmp[v3].push_back(normal);

        }
        else
        {
            std::cerr << "Number of vertices on face must be 3" << std::endl;
            myfile.close();
            return false;
        }
    }

    for( int v = 0 ; v < numberOfVertices ; ++v )
    {
        float sizeTmp = normalsTmp[v].size();
        glm::vec3 tmp;
        for(auto n : normalsTmp[v])
        {
            tmp += n;
        }
        tmp = tmp / sizeTmp;
        normals[v] = tmp;
    }

    myfile.close();
    return true;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// simplify vertices / normals of the mesh

void Mesh::simplify (unsigned int resolution)
{
    std::vector<std::vector<unsigned int>> grid;
    std::vector<unsigned int> grid_indices;

    grid.resize(pow(resolution, 3));
    grid_indices.resize(grid.size());


    std::vector<unsigned short> repr_indices;
    std::vector<std::vector<unsigned short> > repr_triangles;
    std::vector<glm::vec3> repr_indexed_vertices, repr_indexed_normals;

    // increase bounding box of the mesh to avoid precision issues
    BOX C = bounding_box;
    C.xpos += glm::vec2(-0.1 * abs(C.xpos.x), 0.1 * abs(C.xpos.y));
    C.ypos += glm::vec2(-0.1 * abs(C.ypos.x), 0.1 * abs(C.ypos.y));
    C.zpos += glm::vec2(-0.1 * abs(C.zpos.x), 0.1 * abs(C.zpos.y));

    // calculate the size of the grid
    float dx, dy, dz;
    dx = (C.xpos.y - C.xpos.x) / (float) resolution;
    dy = (C.ypos.y - C.ypos.x) / (float) resolution;
    dz = (C.zpos.y - C.zpos.x) / (float) resolution;

    // for each vertex of a triangle, we determine the position P(ix, iy, iz)
    // and place the vertex' index in the grid at position P
    for (auto triangle : triangles) {
        for (int i = 0; i < 3; ++i) {
            glm::vec3 v = indexed_vertices.at(triangle.at(i));

            int ix = (v.x - C.xpos.x) / dx;
            int iy = (v.y - C.ypos.x) / dy;
            int iz = (v.z - C.zpos.x) / dz;

            if(std::count(grid.at(ix + iy * resolution + iz * pow(resolution, 2)).begin(), grid.at(ix + iy * resolution + iz * pow(resolution, 2)).end(), triangle.at(i)) == 0)
                grid.at(ix + iy * resolution + iz * pow(resolution, 2)).push_back(triangle.at(i));
        }
    }


    // for each position in the grid that contains at least one vertex
    // we calculate the position of the representative vertex using the
    // average of vertices at the same position in the grid
    // We do the same with normals
    for (unsigned i = 0 ; i < grid.size(); ++i) {
        if (grid.at(i).size() > 0) {
            glm::vec3 repr_pos = glm::vec3(0);
            glm::vec3 repr_norm = glm::vec3(0);

            for (unsigned int j = 0; j < grid.at(i).size(); ++j){
                repr_pos += indexed_vertices.at(grid.at(i).at(j));
                repr_norm += indexed_normals.at(grid.at(i).at(j));
            }

            repr_pos = repr_pos / (float) grid.at(i).size();
            repr_norm = repr_norm / (float) grid.at(i).size();

            grid_indices.at(i) = repr_indexed_vertices.size();
            repr_indexed_vertices.push_back(repr_pos);
            repr_indexed_normals.push_back(repr_norm);
        }
    }


    // for each triangle, we verify which index of representative vertex is
    // for each triangle's vertex. If all indices are different then we add
    // indices of the triangles else, we don't keep these vertices
    for (auto triangle: triangles) {
        short current_indices[3];
        for (short i = 0; i < 3; ++i) {
            glm::vec3 v = indexed_vertices.at(triangle.at(i));

            int ix = (v.x - C.xpos.x) / dx;
            int iy = (v.y - C.ypos.x) / dy;
            int iz = (v.z - C.zpos.x) / dz;

            current_indices[i] = grid_indices.at(ix + iy * resolution + iz * pow(resolution,2));
        }
        if( current_indices[0] != current_indices[1] &&
            current_indices[0] != current_indices[2] &&
            current_indices[1] != current_indices[2]){
            repr_indices.push_back(current_indices[0]);
            repr_indices.push_back(current_indices[1]);
            repr_indices.push_back(current_indices[2]);

            std::vector<unsigned short> tmp;
            tmp.push_back(current_indices[0]);
            tmp.push_back(current_indices[1]);
            tmp.push_back(current_indices[2]);
            repr_triangles.push_back(tmp);
        }
    }

    // we substitute old vectors with new one
    if(indexed_vertices.size() > repr_indexed_vertices.size()) {
        indices = repr_indices;
        triangles = repr_triangles;
        indexed_vertices = repr_indexed_vertices;
        indexed_normals = repr_indexed_normals;
    }
    else {std::cout << "minimum simplification" << std::endl;}
}



void Mesh::adaptiveSimplify (unsigned int numOfPerLeafVertices)
{
    std::vector<unsigned int> vertices_to_repr;
    std::vector<unsigned short> repr_indices;
    std::vector<std::vector<unsigned short> > repr_triangles;
    std::vector<glm::vec3> repr_indexed_vertices, repr_indexed_normals;
    vertices_to_repr.resize(indexed_vertices.size());

    // increase bounding box of the mesh to avoid precision issues
    BOX C = bounding_box;
    C.xpos += glm::vec2(-0.1 * abs(C.xpos.x), 0.1 * abs(C.xpos.y));
    C.ypos += glm::vec2(-0.1 * abs(C.ypos.x), 0.1 * abs(C.ypos.y));
    C.zpos += glm::vec2(-0.1 * abs(C.zpos.x), 0.1 * abs(C.zpos.y));

    // create an octree and start the recursivity
    m_octree = std::make_shared<Octree>(C.xpos.x, C.xpos.y, C.ypos.x, C.ypos.y, C.zpos.x, C.zpos.y);
    adaptiveSimplifyRec(m_octree, numOfPerLeafVertices, vertices_to_repr, repr_indexed_vertices, triangles);

    // for each triangle, if vertices of a triangle have a different representative vertex then
    // we store indices and triangle of this representative vertex as final vertex
    for(unsigned int i = 0 ; i < triangles.size(); i++){
        short current_indices[3];
        for(int j = 0 ; j < 3 ; j ++){ current_indices[j] = vertices_to_repr.at(triangles.at(i).at(j));}

        // all representatives vertices of the triangle are different
        if( current_indices[0] != current_indices[1] && current_indices[0] != current_indices[2] &&
            current_indices[1] != current_indices[2])
        {
            repr_indices.push_back(current_indices[0]);
            repr_indices.push_back(current_indices[1]);
            repr_indices.push_back(current_indices[2]);

            std::vector<unsigned short> tmp;
            tmp.push_back(current_indices[0]);
            tmp.push_back(current_indices[1]);
            tmp.push_back(current_indices[2]);
            repr_triangles.push_back(tmp);
        }
    }

    // we substitute old vectors with new one
    if(indexed_vertices.size() > repr_indexed_vertices.size()){
        indices = repr_indices;
        triangles = repr_triangles;
        indexed_vertices = repr_indexed_vertices;
        indexed_normals = repr_indexed_normals;
    }
    else{std::cout << "minimum simplification" << std::endl;}
}

void Mesh::adaptiveSimplifyRec (std::shared_ptr<Octree> octree, unsigned int numOfPerLeafVertices, std::vector<unsigned int> & vertices_to_repr,
                                std::vector<glm::vec3> & repr_indexed_vertices, std::vector<std::vector<unsigned short>> in_triangles)
{
    std::vector<std::vector<unsigned short>> out_triangles;
    std::unordered_map<int, bool> seen_vertices_map, seen_triangles_map;
    unsigned int lastJ = std::numeric_limits<unsigned int>::max();
    bool is_leaf = true;

    for (unsigned int j = 0 ; j < in_triangles.size(); ++j)
    {
        for(short i = 0 ; i < 3 ; i ++){
            if(octree->containsVertex(indexed_vertices.at(in_triangles.at(j).at(i)))){
                if(seen_vertices_map.find(in_triangles.at(j).at(i)) == seen_vertices_map.end()){
                    octree->putIndex(in_triangles.at(j).at(i));
                    seen_vertices_map[in_triangles.at(j).at(i)] = true;
                    if (seen_triangles_map.find(j) == seen_triangles_map.end()) seen_triangles_map[j] = true;
                }
                if(j!=lastJ){
                    out_triangles.push_back(in_triangles.at(j));
                    lastJ = j;
                }
            }

            if(is_leaf && octree->getIndices().size() > numOfPerLeafVertices) {
                is_leaf = false;
                octree->generateChildren();
            }
        }
    }

    if(!is_leaf){
        for(short k = 0 ; k < 8 ; k++)
            adaptiveSimplifyRec(octree->getChild(k), numOfPerLeafVertices, vertices_to_repr, repr_indexed_vertices, out_triangles);
    }

    if(is_leaf){
        glm::vec3 repr = glm::vec3(0.0f);

        // for quadric error calculation
        glm::vec4 repr_tmp = glm::vec4(0.0f);
        glm::mat4 Qp(glm::mat4(0.0f)), Qp_inv(glm::mat4(0.0f));

        for (auto kv : seen_triangles_map){
            glm::vec4 plane = equation_plane (
                    indexed_vertices.at(triangles.at(kv.first).at(0)).x, indexed_vertices.at(triangles.at(kv.first).at(0)).y, indexed_vertices.at(triangles.at(kv.first).at(0)).z,
                    indexed_vertices.at(triangles.at(kv.first).at(1)).x, indexed_vertices.at(triangles.at(kv.first).at(1)).y, indexed_vertices.at(triangles.at(kv.first).at(1)).z,
                    indexed_vertices.at(triangles.at(kv.first).at(2)).x, indexed_vertices.at(triangles.at(kv.first).at(2)).y, indexed_vertices.at(triangles.at(kv.first).at(2)).z
            ) ;
            glm::mat4 Q = glm::mat4(pow(plane.x,2) , plane.x*plane.y   , plane.x*plane.z   , plane.x*plane.w,
                                    plane.x*plane.y  , pow(plane.y,2)    , plane.y*plane.z   , plane.y*plane.w,
                                    plane.x*plane.z , plane.y*plane.z   , pow(plane.z,2)    , plane.z*plane.w,
                                    plane.x*plane.w , plane.y*plane.w   , plane.z*plane.w   , pow(plane.w,2));

            //glm::mat4 Q;
            Q[0][0] = pow(plane.x,2); Q[1][0] = plane.x*plane.y; Q[2][0] = plane.x*plane.z; Q[3][0] = plane.x*plane.w;
            Q[0][1] = plane.x*plane.y; Q[1][1] = pow(plane.y,2); Q[2][1] = plane.y*plane.z; Q[3][1] = plane.y*plane.w;
            Q[0][2] = plane.x*plane.z; Q[1][2] = plane.y*plane.z; Q[2][2] = pow(plane.z,2); Q[3][2] = plane.z*plane.w;
            Q[0][3] = plane.x*plane.w; Q[1][3] = plane.y*plane.w; Q[2][3] = plane.z*plane.w; Q[3][3] = pow(plane.w,2);
            Qp += Q;
        }
        Qp[0][3]=0; Qp[1][3]=0; Qp[2][3]=0; Qp[3][3]=1;
        bool inversible = (glm::determinant(Qp) != 0);

        if(inversible){
            Qp_inv = glm::inverse(Qp);
            repr_tmp = Qp_inv * glm::vec4(0,0,0,1);

            repr.x = repr_tmp.x;
            repr.y = repr_tmp.y;
            repr.z = repr_tmp.z;
        }
        bool outside = false;
        if(!octree->containsVertex(repr)){ outside = true; repr = glm::vec3(0.0f);};

        for(unsigned int i = 0 ; i < octree->getIndices().size() ; ++i){
            if(outside) repr += indexed_vertices.at(octree->getIndexAt(i));
            vertices_to_repr.at(octree->getIndexAt(i)) = repr_indexed_vertices.size();
        }

        if(outside) repr /= (float) octree->getIndices().size();
        repr_indexed_vertices.push_back(repr);
    } // if(is_leaf)
}


glm::vec4 Mesh::equation_plane(float x1, float y1, float z1,
                               float x2, float y2, float z2,
                               float x3, float y3, float z3)
{
    float a1 = x2 - x1;
    float b1 = y2 - y1;
    float c1 = z2 - z1;

    float a2 = x3 - x1;
    float b2 = y3 - y1;
    float c2 = z3 - z1;

    float a = b1 * c2 - b2 * c1;
    float b = a2 * c1 - a1 * c2;
    float c = a1 * b2 - b1 * a2;

    float d = (- a * x1 - b * y1 - c * z1);
    return glm::vec4(a,b,c,d);
}