// https://kamino.hatenablog.com/entry/lm_method
// I consulted this web page to understand levengerg-marquardt method. Thank you !

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

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "02_levenberg_marquardt", NULL, NULL);
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
  double nu = 1.5;
  double lambda = 1.0;
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
    analyze_jacobian(number_of_points, jacobian);
      
    cv::Mat rmat(number_of_points*2, 1, CV_64F, r);
    cv::Mat jmat(number_of_points*2, 2, CV_64F, jacobian);
    cv::Mat opmat(2, 1, CV_64F, optimized_point);

    cv::Mat f_xk = 0.5 * rmat.t() * rmat;
      
    cv::Mat xk_devidedBYratio = opmat - ((jmat.t()*jmat + (lambda/nu)*cv::Mat::eye(2, 2, CV_64F)).inv())*jmat.t()*rmat;
      
    cv::Mat xk_notdevided = opmat - ((jmat.t()*jmat + lambda*cv::Mat::eye(2, 2, CV_64F)).inv())*jmat.t()*rmat;
    cv::Mat f_xk_devidedBYratio = 0.5 * xk_devidedBYratio.t() * xk_devidedBYratio;
    cv::Mat f_xk_notdevided = 0.5 * xk_notdevided.t() * xk_notdevided;
 
    if (f_xk_devidedBYratio.at<double>(0,0) <= f_xk.at<double>(0,0)){
        // When the value is steadily decreasing, strengthen the Gauss-Newton method component.
        lambda /= nu;
    }
    else if (f_xk.at<double>(0,0) < f_xk_notdevided.at<double>(0,0)){
        // Weaken the Gauss-Newton component of the method until the values start to decrease.
        // update λk = λk-1νw with minimum w that is φ(λk-1ν^w) <= f(xk)
        // w : integer
        int w = 0;
        cv::Mat xk_w = opmat - ((jmat.t()*jmat + (lambda*pow(nu, w))*cv::Mat::eye(2, 2, CV_64F)).inv())*jmat.t()*rmat;
        cv::Mat f_xk_w = 0.5 * xk_w.t() * xk_w;
        while (f_xk.at<double>(0,0) < f_xk_w.at<double>(0,0)){
            w++;
            xk_w = opmat - ((jmat.t()*jmat + (lambda*pow(nu, w))*cv::Mat::eye(2, 2, CV_64F)).inv())*jmat.t()*rmat;
            f_xk_w = 0.5 * xk_w.t() * xk_w;
        }
        lambda *= pow(nu, w);
    }
      
    opmat = opmat - ((jmat.t()*jmat + lambda * cv::Mat::eye(2, 2, CV_64F)).inv())*jmat.t()*rmat;
    optimized_point[0] = opmat.at<double>(0,0);
    optimized_point[1] = opmat.at<double>(0,1);
      
    cv::Mat sdmat = ((jmat.t()*jmat + lambda * cv::Mat::eye(2, 2, CV_64F)).inv())*jmat.t()*rmat;
    double sdnorm = sqrt(pow(sdmat.at<double>(0,0), 2) + pow(sdmat.at<double>(0,1), 2));
    if(sdnorm < 0.0000000001){
        break; // converged
    }
        
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
        stbi_write_png((std::string(PATH_ROOT_DIR) + "/02_levenberg_marquardt/output/step" + std::to_string(step_number) + ".png").c_str(),
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

