#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <random>

#define NUMBER_OF_POINTS 250

#define WINDOW_SIZE 1000

#include "numOpt.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define SCRSHOT

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "00_random3d", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
    
  int field_size = 500;
  int number_of_points = NUMBER_OF_POINTS;
  std::mt19937 mt(0);
  std::uniform_int_distribution<int> rnd(0, field_size * 2);
  float points[NUMBER_OF_POINTS * 2];
  for (int i=0; i<NUMBER_OF_POINTS * 2; i++){
      points[i] = rnd(mt) - field_size;
  }
    
  float z[2*field_size * 2*field_size];
  calc_z_value(points, z, field_size, number_of_points);
    
  float center[2] = { 1.f, 1.f };
  float min_square_error = 1000000000;
  int step_num_i = 0;
  int lap_iteration_i = 0;
    
  while (!glfwWindowShouldClose(window))
  {
      
    lap_iteration_i++;
    if(lap_iteration_i > 10000){
        break;
    }
      
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    gRotatef(85.f, -1.f, 0.1f, 0.f);
    glRotatef(45.f, -1.f, 0.1f, 0.f);
//    glRotatef(0.f, -1.f, 0.1f, 0.f);
    glTranslatef(0.f, 0.f, -0.4f);
      
    float candidate[2] = {rnd(mt) - field_size, rnd(mt) - field_size};
    float tmp_square_error = square_error_sum(points, candidate, number_of_points);
    
      
    if (tmp_square_error < min_square_error){
        lap_iteration_i = 0;
        min_square_error = tmp_square_error;
        center[0] = candidate[0];
        center[1] = candidate[1];
        std::cout << "candidate : " << candidate[0] << ", " << candidate[1] << std::endl;
        std::cout << "temporal square error : " << tmp_square_error << std::endl;
        std::cout << "center : " << center[0] << ", " << center[1] << std::endl;
        std::cout << "minimal square error : " << min_square_error << std::endl;
        std::cout << std::endl;
        
        glColor3f(0.7f, 0.7f, 0.7f);
        glBegin(GL_TRIANGLES);
        glVertex3f(-1.f,  1.f, 0.f);
        glVertex3f(-1.f, -1.f, 0.f);
        glVertex3f( 1.f,  1.f, 0.f);
        glVertex3f(-1.f, -1.f, 0.f);
        glVertex3f( 1.f, -1.f, 0.f);
        glVertex3f( 1.f,  1.f, 0.f);
        glEnd();
          
        glLineWidth(2.0f);
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_LINES);
        glVertex3f(-1.5f,  0.f, 0.f);
        glVertex3f( 1.5f,  0.f, 0.f);
        glVertex3f( 0.f, -1.5f, 0.f);
        glVertex3f( 0.f, 1.5f, 0.f);
        glVertex3f( 0.f, 0.f, -0.5f);
        glVertex3f( 0.f, 0.f, 1.5f);
        glEnd();
        
        glPointSize(10);
        glBegin(GL_POINTS);
        for (int i=0; i<NUMBER_OF_POINTS/2; i++){
            glColor3f(1.f, 0.f, 0.f);
            glVertex2f(points[2*i+0]/field_size, points[2*i+1]/field_size);
        }
        glColor3f(0.f, 1.f, 0.f);
        glVertex2f(center[0]/field_size, center[1]/field_size);
        int center_idx = 2*field_size*(center[0] + field_size) + center[1] + field_size;
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(center[0]/field_size, center[1]/field_size, (std::log(z[center_idx])/40 - 0.3)*20);
        glEnd();
        
        glPointSize(1);
        glBegin(GL_POINTS);
        for (int i=0; i<2*field_size * 2*field_size; i++){
            int coord[2] = { i / (2*field_size) - field_size, i % (2*field_size) - field_size};
            float coordf[2] = {coord[0], coord[1]};
            // 40, 0.3, 26 : scale factor to make r value 0 ~ 1
            glColor3f((std::log(z[i])/40 - 0.3)*26, (std::log(z[i])/40 - 0.3)*10, (std::log(z[i])/40 - 0.3)*10);
            glVertex3f(coordf[0]/field_size, coordf[1]/field_size, (std::log(z[i])/40 - 0.3)*20);
        }
        glEnd();
          
#ifdef SCRSHOT
        GLubyte* pixel_data = (GLubyte*)malloc((width)*(height)*3*(sizeof(GLubyte)));
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0,
                    width, height,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    pixel_data);
        if (!pixel_data) std::cout << "error pixel data " << std::endl;
            
        stbi_flip_vertically_on_write(1);
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/00_random3d/output/step" + std::to_string(step_num_i) + ".png").c_str(),
                        width, height, 3,
                        pixel_data,
                        0);
        free(pixel_data);
#endif
        step_num_i++;
        glfwSwapBuffers(window);
        glfwPollEvents();
      }
    }
    std::cout << "final square error : " << min_square_error << std::endl;
    std::cout << "final point : " << center[0] << ", " << center[1] << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

