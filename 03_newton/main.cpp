#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <random>

#include "numOpt.hpp"

#define NUMBER_OF_POINTS 250

#define WINDOW_SIZE 1000

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
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "03_newton", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
    
  int field_size = 1000;
  float points[NUMBER_OF_POINTS * 2];
  create_random_points(points, field_size);

  //float optimized_point[2] = { 640.f, 640.f }; // initial point definition
  float optimized_point[2] = { 100.f, 100.f };

  float dh = 0.01;
  float lr = 10;
  float gradient[2];
  float hessian_factors[3];
  float hessian[4];
  float hessian_inverse[4];
  float search_direction[2];
  int step_number = 0; // for screen shot
    
  while (!glfwWindowShouldClose(window))
  {
    std::cout << std::endl;
    std::cout << "optimized_point : " << optimized_point[0] << ", " << optimized_point[1] << std::endl;
      
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
      
    calc_gradient(points, optimized_point, gradient, dh);
    if (std::sqrt(gradient[0]*gradient[0] + gradient[1]*gradient[1]) < 5000){
        break; // converged
    }
    std::cout << "gradient : " << gradient[0] << ", " << gradient[1] << std::endl;
    calc_hessian_factors(points, optimized_point, hessian_factors, dh);
    hessian[0] = hessian_factors[0];
    hessian[1] = hessian_factors[1];
    hessian[2] = hessian_factors[1];
    hessian[3] = hessian_factors[2];
    std::cout << "hessian : " << hessian[0] << ", " << hessian[1] << ", " << hessian[2] << ", " << hessian[3] << std:: endl;
//    normalize_mat(hessian);
//    std::cout << "normalized hessian : " << hessian[0] << ", " << hessian[1] << ", " << hessian[2] << ", " << hessian[3] << std:: endl;
    calc_inverse_matrix(hessian, hessian_inverse);
    std::cout << "inverse : " << hessian_inverse[0] << ", " << hessian_inverse[1] << ", " << hessian_inverse[2] << ", " << hessian_inverse[3] << std::endl;
    mat_dot_vec(hessian_inverse, gradient, search_direction);
    std::cout << "search_direction : " << search_direction[0] << ", " << search_direction[1] << std::endl;
    normalize_vec(search_direction);
    std::cout << "normalized search_direction : " << search_direction[0] << ", " << search_direction[1] << std::endl;
    optimized_point[0] -= search_direction[0] * lr;
    optimized_point[1] -= search_direction[1] * lr;
        
    glPointSize(10);
    glBegin(GL_POINTS);
          
    for (int i=0; i<NUMBER_OF_POINTS; i++){
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(points[2*i+0]/field_size, points[2*i+1]/field_size);
    }
    glColor3f(0.f, 1.f, 0.f);
    glVertex2f(optimized_point[0]/field_size, optimized_point[1]/field_size);
          
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
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/03_newton/output/step" + std::to_string(step_number) + ".png").c_str(),
                        width, height, 3,
                        pixel_data,
                        0);
        free(pixel_data);
#endif
        
        step_number++;
        glfwSwapBuffers(window);
        glfwPollEvents();
      }
    std::cout << "final optimized point : " << optimized_point[0] << ", " << optimized_point[1] << std::endl;
    std::cout << "final gradient : " << gradient[0] << ", " << gradient[1] << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

