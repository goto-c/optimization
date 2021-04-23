#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <random>

#include "numOpt.hpp"

#include <opencv2/opencv.hpp>

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

double norm(double* r1, double* r2){
    return std::sqrt(std::pow(r1[0] - r2[0], 2) + std::pow(r1[1] - r2[1], 2));
}

void calc_jacobian(double* r, int number_of_points, double* target_point, double* jacobian, double dh){
    double target_point_plus_x[2] = {target_point[0] + dh, target_point[1]     };
    double target_point_plus_y[2] = {target_point[0]     , target_point[1] + dh};
    for (int i=0; i<number_of_points; i++){
        jacobian[i*2+0] = ( norm(&r[i*2], target_point_plus_x) - norm(&r[i*2], target_point) )/dh;
        jacobian[i*2+1] = ( norm(&r[i*2], target_point_plus_y) - norm(&r[i*2], target_point) )/dh;
    }
}

void calc_jacobian_linear(double* r, int number_of_points, double* jacobian, double dh){
    for (int i=0; i<number_of_points*2; i++){
        if(i%2==0){ // r[i] includes only x
            double r_plus_x = r[i] + dh;
            jacobian[i*2+0] = ( r_plus_x - r[i] )/dh;
            jacobian[i*2+1] = 0;
        }
        else if(i%2==1){ // r[i] includes only y
            double r_plus_y = r[i] + dh;
            jacobian[i*2+0] = 0;
            jacobian[i*2+1] = ( r_plus_y - r[i] )/dh;
        }
    }
}

void calc_r(double* points, int number_of_points, double* target_point, double* r){
    for (int i=0; i<number_of_points; i++){
        r[i*2+0] = norm(&points[i*2], target_point);
        r[i*2+1] = norm(&points[i*2], target_point);
    }
}

void calc_r_linear(double* points, int number_of_points, double* target_point, double* r){
    for (int i=0; i<number_of_points; i++){
        r[i*2+0] = points[i*2+0] - target_point[0];
        r[i*2+1] = points[i*2+1] - target_point[1];
    }
}

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "04_gauss_newton", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
    
  int field_size = 1000;
  int number_of_points = NUMBER_OF_POINTS;
  double points[NUMBER_OF_POINTS * 2];
  create_random_points(points, field_size, number_of_points);

//  double optimized_point[2] = { 640.f, 640.f }; // initial point definition
  double optimized_point[2] = { 500.f, 500.f };
  double dh = 0.01;
  double lr = 0.01;
  double r[NUMBER_OF_POINTS*2];
  double jacobian[NUMBER_OF_POINTS*4];
  int step_number = 0; // for screen shot
    
  while (!glfwWindowShouldClose(window))
  {
    std::cout << std::endl;
    std::cout << "optimized_point : " << optimized_point[0] << ", " << optimized_point[1] << std::endl;
      
    double ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (double) height;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(0.f, 0.f, 0.f, 1.f);
      
    calc_r_linear(points, number_of_points, optimized_point, r);
//            for (int i=0; i<number_of_points*2; i++){
//                std::cout << r[i] << " ";
//                std::cout << std::endl;
//            }
    calc_jacobian_linear(r, number_of_points, jacobian, dh);
//      for (int i=0; i<number_of_points*4; i++){
//          std::cout << jacobian[i] << " ";
//          std::cout << std::endl;
//      }
    cv::Mat rmat(number_of_points*2, 1, CV_64F, r);
    cv::Mat jmat(number_of_points*2, 2, CV_64F, jacobian);
//    std::cout << "rmat: " << rmat << std::endl;
//    std::cout << "jacobian: " << jmat << std::endl;
//    std::cout << "jmat: " << ((jmat.t())*jmat)/number_of_points << std::endl; // BUG : inf
    cv::Mat sdmat = -((jmat.t()*jmat).inv())*jmat.t()*rmat;
      std::cout << "sdmat: " << sdmat << std::endl;
    optimized_point[0] += sdmat.data[0] * lr;
    optimized_point[1] += sdmat.data[1] * lr;
        
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
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/04_gauss_newton/output/step" + std::to_string(step_number) + ".png").c_str(),
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
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

