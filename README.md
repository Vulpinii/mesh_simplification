<p align="center"><h1>Mesh Simplification</h1></p>

This program allows to visualize two methods of mesh simplification:
- Simplification by Partitioning (OCS) using a regular grid;
- Adaptive simplification to regions with the Octree.

These two methods are very fast and perform flow calculations but be careful, they generate topological errors!
The results obtained are therefore not the best. Improvement possible using the Half-Edge.

Written in C++ and using OpenGL API.

## Features
- Load different 3D models
- Visualize the number of mesh' vertices
- Visualize the valence of each vertex
- Visualize the simplification of meshes
- Choose the adaptive (octree) or partitioning (grid) structure
- Render in wireframe
- Return to the original mesh

## Building
#### On Linux
**Prerequisite**: CMake

To build this program, download the source code using ``git clone https://github.com/Vulpinii/mesh_simplification`` or directly the zip archive.
Then run the `` launch.sh`` shell script.

You can do it manually by following these commands:
```shell script
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
./program
```

#### On Windows
[instructions coming soon]


## Gallery
#### YouTube Video

<a href="https://www.youtube.com/watch?v=7tElrSOR4gA" target="_blank"><img src="https://github.com/Vulpinii/mesh_simplification/blob/master/images/mesh_simplification.png" 
alt="Mesh Simplification" width="100%" height="auto" border="10" /></a>

#### Preview
<p align="center"><img src="https://github.com/Vulpinii/mesh_simplification/blob/master/images/demo.gif" alt="Animated gif" width="100%" /></p>