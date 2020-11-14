#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <glm.hpp>

#define OC_LeftBottomBack      0
#define OC_RightBottomBack     1
#define OC_LeftTopBack         2
#define OC_RightTopBack        3
#define OC_LeftBottomFront     4
#define OC_RightBottomFront    5
#define OC_LeftTopFront        6
#define OC_RightTopFront       7

class Octree {
public:
    Octree (float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
        : m_xmin(xmin), m_xmax(xmax), m_ymin(ymin), m_ymax(ymax), m_zmin(zmin), m_zmax(zmax) {
        m_children.resize(8, nullptr);
    }

    Octree ()
            : m_xmin(0), m_xmax(0), m_ymin(0), m_ymax(0), m_zmin(0), m_zmax(0) {
        m_children.resize(8, nullptr);
    }
    virtual ~Octree() = default;

    std::vector<int>& getIndices(){ return m_indices;}

    int getIndexAt(int i) { return m_indices.at(i);}

    void putIndex(int index){ m_indices.push_back(index); }

    std::shared_ptr<Octree> getChild(short index){
        return m_children.at(index);
    }

    [[nodiscard]] bool containsVertex(glm::vec3 position) const{
        return position.x >= m_xmin &&  position.x <= m_xmax &&
               position.y >= m_ymin &&  position.y <= m_ymax &&
               position.z >= m_zmin &&  position.z <= m_zmax;
    }

    void generateChildren() {
        float x_middle = m_xmin + (m_xmax - m_xmin)/2.0f;
        float y_middle = m_ymin + (m_ymax - m_ymin)/2.0f;
        float z_middle = m_zmin + (m_zmax - m_zmin)/2.0f;

        std::vector<std::vector<double>> extremum;
        extremum.resize(8);

        extremum[OC_LeftBottomBack]     = {m_xmin, x_middle, m_ymin, y_middle, z_middle, m_zmax};
        extremum[OC_RightBottomBack]    = {x_middle, m_xmax, m_ymin, y_middle, z_middle, m_zmax};
        extremum[OC_LeftTopBack]        = {m_xmin, x_middle, y_middle, m_ymax, z_middle, m_zmax};
        extremum[OC_RightTopBack]       = {x_middle, m_xmax, y_middle, m_ymax, z_middle, m_zmax};
        extremum[OC_LeftBottomFront]    = {m_xmin, x_middle, m_ymin, y_middle, m_zmin, z_middle};
        extremum[OC_RightBottomFront]   = {x_middle, m_xmax, m_ymin, y_middle, m_zmin, z_middle};
        extremum[OC_LeftTopFront]       = {m_xmin, x_middle, y_middle, m_ymax, m_zmin, z_middle};
        extremum[OC_RightTopFront]      = {x_middle, m_xmax, y_middle, m_ymax, m_zmin, z_middle};

        for (int i = 0 ; i < 8; ++i) // for each m_children in octree
        {
            m_children[i] = std::make_shared<Octree>();
            m_children[i]->m_xmin = extremum[i].at(0);
            m_children[i]->m_xmax = extremum[i].at(1);
            m_children[i]->m_ymin = extremum[i].at(2);
            m_children[i]->m_ymax = extremum[i].at(3);
            m_children[i]->m_zmin = extremum[i].at(4);
            m_children[i]->m_zmax = extremum[i].at(5);
        }
    }

private:
    std::vector<int> m_indices;

    float m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax;
    std::vector<std::shared_ptr<Octree>> m_children;
};
#endif //OCTREE_HPP