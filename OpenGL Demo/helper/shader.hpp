#ifndef shader_hpp
#define shader_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader{
    
public:
    
    GLuint ID;
    
    Shader(const char * vertex_file_path, const char * fragment_file_path);
    
    void use();
    
    void setFloat(const std::string &name, float value);
    
    void setInt(const std::string &name, int value);

    void setVec2(const std::string &name, const glm::vec2 &value);

    void setVec3(const std::string &name, const glm::vec3 &value);

    void setMat3(const std::string &name, const glm::mat3 &mat);
    
    void setMat4(const std::string &name, const glm::mat4 &mat);
    
    
private:
    
    
    
};



//GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

#endif
