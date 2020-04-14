#ifndef OBJLOADER_H
#define OBJLOADER_H
#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

void load_obj(const char* filename, std::vector<glm::vec3> &vertices, std::vector<glm::vec3> &normals, std::vector<GLushort> &elements, bool swap_y_z);

#endif
