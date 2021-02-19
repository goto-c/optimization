#include <iostream>
#include <GLFW/glfw3.h>

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <random>

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

float square_error(float* points, float* target_point){
    float square_error = 0;
    for (int i=0; i<NUMBER_OF_POINTS; i++){
        square_error += pow(points[2*i+0] - target_point[0], 2) + pow(points[2*i+1] - target_point[1], 2);
    }
    return square_error;
}

void create_random_points(float* points){
    std::mt19937 mt(0);
    std::uniform_int_distribution<int> rnd(0, WINDOW_SIZE * 2);
    for (int i=0; i<NUMBER_OF_POINTS * 2; i++){
        points[i] = rnd(mt) - WINDOW_SIZE;
    }
}

void calc_gradient(float* points, float* target_point, float* gradient, float dh){
    float target_point_plus_x[2] = {target_point[0] + dh, target_point[1]     };
    float target_point_plus_y[2] = {target_point[0]     , target_point[1] + dh};
    float dx = square_error(points, target_point_plus_x);
    float dy = square_error(points, target_point_plus_y);
    dx -= square_error(points, target_point);
    dy -= square_error(points, target_point);
    gradient[0] = dx / dh;
    gradient[1] = dy / dh;
}

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "Simple example", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
    
  float points[NUMBER_OF_POINTS * 2];
  create_random_points(points);

  float optimized_point[2] = { 640.f, 640.f };

  float dh = 0.01;
  float lr = 0.0001;
  float gradient[2];
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
    std::cout << "grad(E) : (" << gradient[0] << ", " << gradient[1] << ")" << std::endl;
    if (abs(gradient[0]) < 5000 && abs(gradient[1]) < 5000){
        break; // converged
    }
    optimized_point[0] -= gradient[0] * lr;
    optimized_point[1] -= gradient[1] * lr;
        
    glPointSize(10);
    glBegin(GL_POINTS);
          
    for (int i=0; i<NUMBER_OF_POINTS; i++){
        glColor3f(1.f, 0.f, 0.f);
        glVertex2f(points[2*i+0]/WINDOW_SIZE, points[2*i+1]/WINDOW_SIZE);
    }
    glColor3f(0.f, 1.f, 0.f);
    glVertex2f(optimized_point[0]/WINDOW_SIZE, optimized_point[1]/WINDOW_SIZE);
          
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
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/01_gradient_descent/output/step" + std::to_string(step_number) + ".png").c_str(),
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

