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
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "01_gradient_descent3d", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
    
  int field_size = 500;
  int number_of_points = NUMBER_OF_POINTS;
  float points[NUMBER_OF_POINTS * 2];
  create_random_points(points, field_size, number_of_points);
    
  float z[2*field_size * 2*field_size];
  calc_z_value(points, z, field_size, number_of_points);

  float optimized_point[2] = { 500.f, -500.f };

  float dh = 0.01;
  float lr = 0.0001;
  float gradient[2];
  int step_num_i = 0; // for screen shot
    
  while (!glfwWindowShouldClose(window))
  {
    std::cout << std::endl;
    std::cout << "optimized_point : " << optimized_point[0] << ", " << optimized_point[1] << std::endl;
      
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
    glRotatef(85.f, -1.f, 0.1f, 0.f);
//    glRotatef(45.f, -1.f, 0.1f, 0.f);
//    glRotatef(0.f, -1.f, 0.1f, 0.f);
    glTranslatef(0.f, 0.f, -0.4f);
      
    calc_gradient(points, optimized_point, gradient, dh, number_of_points);
    std::cout << "gradient : (" << gradient[0] << ", " << gradient[1] << ")" << std::endl;
    if (std::sqrt(gradient[0]*gradient[0] + gradient[1]*gradient[1]) < 5000){
        break; // converged
    }

    optimized_point[0] -= gradient[0] * lr;
    optimized_point[1] -= gradient[1] * lr;
        
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
      
    // draw green points
    glPointSize(10);
    glBegin(GL_POINTS);
    for (int i=0; i<NUMBER_OF_POINTS; i++){
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(points[2*i+0]/field_size, points[2*i+1]/field_size);
    }
    glColor3f(0.f, 1.f, 0.f);
    glVertex2f(optimized_point[0]/field_size, optimized_point[1]/field_size);
    int center_idx = 2*field_size*(optimized_point[0] + field_size) + optimized_point[1] + field_size;
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(optimized_point[0]/field_size, optimized_point[1]/field_size, (std::log(z[center_idx])/40 - 0.3)*20);
    glEnd();
      
    // draw error graph
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
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/01_gradient_descent3d/output/step" + std::to_string(step_num_i) + ".png").c_str(),
                        width, height, 3,
                        pixel_data,
                        0);
        free(pixel_data);
#endif
        
        step_num_i++;
        glfwSwapBuffers(window);
        glfwPollEvents();
      }
    std::cout << "final optimized point : " << optimized_point[0] << ", " << optimized_point[1] << std::endl;
    std::cout << "final gradient : " << gradient[0] << ", " << gradient[1] << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

