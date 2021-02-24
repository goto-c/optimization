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
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "00_random", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
    
  int field_size = 1000;
  int number_of_points = NUMBER_OF_POINTS;
  std::mt19937 mt(0);
  std::uniform_int_distribution<int> rnd(0, field_size * 2);
  float points[NUMBER_OF_POINTS * 2];
  for (int i=0; i<NUMBER_OF_POINTS * 2; i++){
      points[i] = rnd(mt) - field_size;
  }
  float center[2] = { 1.f, 1.f };
  float min_square_error = 1000000000;
  int step_number = 0;
  int lap_iteration_number = 0;
    
  while (!glfwWindowShouldClose(window))
  {
      
    lap_iteration_number++;
    if(lap_iteration_number > 1000){
        break;
    }
      
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(0.f, 0.f, 0.f, 1.f);
      
    float candidate[2] = {rnd(mt) - field_size, rnd(mt) - field_size};
    float tmp_square_error = square_error_sum(points, candidate, number_of_points);
    
      
    if (tmp_square_error < min_square_error){
        lap_iteration_number = 0;
        min_square_error = tmp_square_error;
        center[0] = candidate[0];
        center[1] = candidate[1];
        std::cout << "candidate : " << candidate[0] << ", " << candidate[1] << std::endl;
        std::cout << "temporal square error : " << tmp_square_error << std::endl;
        std::cout << "center : " << center[0] << ", " << center[1] << std::endl;
        std::cout << "minimal square error : " << min_square_error << std::endl;
        std::cout << std::endl;
        
        glPointSize(10);
        glBegin(GL_POINTS);
          
        for (int i=0; i<NUMBER_OF_POINTS/2; i++){
            glColor3f(1.f, 0.f, 0.f);
            glVertex2f(points[2*i+0]/field_size, points[2*i+1]/field_size);
        }
        glColor3f(0.f, 1.f, 0.f);
        glVertex2f(center[0]/field_size, center[1]/field_size);
          
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
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/00_random/output/step" + std::to_string(step_number) + ".png").c_str(),
                        width, height, 3,
                        pixel_data,
                        0);
        free(pixel_data);
#endif
        step_number++;
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

