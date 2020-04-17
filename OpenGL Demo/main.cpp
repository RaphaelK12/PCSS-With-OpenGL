// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;

#include "helper/shader.hpp"
#include "helper/objloader.hpp"
#include "helper/control.hpp"


//thanks to the tutorial from
//https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/2.2.basic_lighting_specular/basic_lighting_specular.cpp

const unsigned int WIDTH = 1300;
const unsigned int HEIGHT = 1300;

void mainDrawing();

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // this line is for MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(WIDTH, HEIGHT, "CZ - OpenGL", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glViewport(0, 0, WIDTH, HEIGHT);
    
    // grey background
    glClearColor(0.53f, 0.81f, 0.98f, 0.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // create shader for the item object
    Shader itemShader("vertexShader", "fragmentShader");
    
    //TODO: make scale automatic!
    float object_scale = 300;
    glm::vec3 object_color = glm::vec3(1, 0.53, 0);
    
    //OBJECT Buffer!
    //here to load obj/or create vertex!
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLushort> indices; // Won't be used at the moment.
    load_obj("pumpkin.obj", vertices, normals, indices, false);
    //printf("%d", vertices.size());
    
    //vertex buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    
    //normal buffer
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    
    //indices buffer
    GLuint indicesbuffer;
    glGenBuffers(1, &indicesbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
    
    float floor[] = {
        // positions            // normals
         1.0f, -0.5f,  -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -0.5f,  -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -0.5f,   1.0f,  0.0f, 1.0f, 0.0f,

         1.0f, -0.5f,  -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -0.5f,   1.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -0.5f,   1.0f,  0.0f, 1.0f, 0.0f
    };
    GLuint planeVBO;
    GLuint planeVAO;
    float floor_scale = 1;
    glm::vec3 floor_color = glm::vec3(0.75, 0.75, 0.75);
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor), floor, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindVertexArray(VertexArrayID);
    
    //https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    //Depth map - Create Framebuffer Object
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    
    //Then 2D texture as the framebuffer depth buffer
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); //What is this for?
    // Depth Texture -> FBO depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); //no need to pass in any color here~
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    
    
    do{
        //Clear the screen buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        
        itemShader.use();
        
        //Draw floor here
        glm::mat4 model = glm::mat4(1.0f);
        itemShader.setMat4("MODEL", model);
        itemShader.setVec3("OBJ_COLOR", floor_color);
        itemShader.setFloat("SCALE", floor_scale);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        //set back to vertex array of the object
        glBindVertexArray(VertexArrayID);
        
        //Model --> local to world
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        itemShader.setMat4("MODEL", model);
         
        //Deal with view position here
        //float view_X = sin(glfwGetTime()) * 3.0f;
        //float View_Z = cos(glfwGetTime()) * 3.0f;
        float view_X = sin(0) * 3.0f;
        float View_Z = cos(0) * 3.0f;
        
        glm::vec3 viewPos = glm::vec3(view_X, 1.0f, View_Z);
        itemShader.setVec3("VIEW_POS", viewPos);
        
        //View --> world to view
        glm::mat4 view = glm::lookAt(
                           viewPos,
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 3.0f, 0.0f));
        itemShader.setMat4("VIEW", view);
        
        //Projection --> view to clip
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        itemShader.setMat4("PROJECTION", projection);
        
        //std::cout<<glm::to_string(projection)<<std::endl;
        
        //Deal with light color here~
        glm::vec3 lightColor = glm::vec3(1.0f);
        itemShader.setVec3("LIGHT_COLOR", lightColor);
        
        //Deal with light position here
        glm::vec3 lightPos = glm::vec3(-2.0f, 2.0f, 1.0f);
        itemShader.setVec3("LIGHT_POS", lightPos);
        
        //Object color
        itemShader.setVec3("OBJ_COLOR", object_color);
        
        //Object scale
        itemShader.setFloat("SCALE", object_scale);
        
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
        
        // 2nd place attribute buffer: normal
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
            1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );
        
        // Draw the triangle !
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glDrawArrays(GL_TRIANGLES, 0, vertices.size() ); // 3 indices starting at 0 -> 1 triangle
        
        //process indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesbuffer);
        int size;
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
        glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(itemShader.ID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

void mainDrawing(){
    
    
    
}



