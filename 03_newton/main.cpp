#include <iostream>
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

float square_error_sum(float* points, float* target_point){
    float square_error_sum = 0;
    for (int i=0; i<NUMBER_OF_POINTS; i++){
        square_error_sum += pow(points[2*i+0] - target_point[0], 2) + pow(points[2*i+1] - target_point[1], 2);
    }
    return square_error_sum;
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
    float dx = square_error_sum(points, target_point_plus_x) - square_error_sum(points, target_point);
    float dy = square_error_sum(points, target_point_plus_y) - square_error_sum(points, target_point);
    gradient[0] = dx / dh;
    gradient[1] = dy / dh;
}

void calc_hessian_factors(float* points, float* target_point, float* hessian_factors, float dh){
    // hessian_factors[3]:
    // Hessian = ( hessian_factors[0] hessian_factors[1] )
    //           ( hessian_factors[1] hessian_factors[2] )
    float target_point_plus_x[2] =  {target_point[0] + dh, target_point[1]     };
    float target_point_plus_y[2] =  {target_point[0]     , target_point[1] + dh};
    float target_point_minus_x[2] = {target_point[0] - dh, target_point[1]     };
    float target_point_minus_y[2] = {target_point[0]     , target_point[1] - dh};
    float target_point_plus_xy[2] = {target_point[0] + dh, target_point[1] + dh};
    float dx2 =       square_error_sum(points, target_point_plus_x)
                - 2 * square_error_sum(points, target_point)
                    + square_error_sum(points, target_point_minus_x);
    float dy2 =       square_error_sum(points, target_point_plus_y)
                - 2 * square_error_sum(points, target_point)
                    + square_error_sum(points, target_point_minus_y);
    float dxdy =   square_error_sum(points, target_point_plus_xy)
                 - square_error_sum(points, target_point_plus_x)
                 - square_error_sum(points, target_point_plus_y)
                 + square_error_sum(points, target_point);
    hessian_factors[0] = dx2 /(dh*dh);
    hessian_factors[1] = dxdy/(dh*dh);
    hessian_factors[2] = dy2 /(dh*dh);
}

void normalize_mat(float* matrix){
    float det = matrix[0] * matrix[3] - matrix[2] * matrix[1];
    for (int i=0; i<4; i++){
        matrix[i] /= det;
    }
}

void normalize_vec(float* vec){
    float norm = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
    vec[0] /= norm;
    vec[1] /= norm;
}

void calc_inverse_matrix(float* matrix, float* inverse_matrix){
    // inverse_matrix[4]:
    // H^(-1) = ( inverse_matrix[0] inverse_matrix[1] )
    //          ( inverse_matrix[2] inverse_matrix[3] )
    float det = matrix[0] * matrix[3] - matrix[2] * matrix[1];
    inverse_matrix[0] = matrix[2] / det;
    inverse_matrix[1] = -matrix[1] / det;
    inverse_matrix[2] = -matrix[1] / det;
    inverse_matrix[3] = matrix[0] / det;
}

void mat_dot_vec(float* matrix, float* vec, float* target_vec){
    target_vec[0] = matrix[0] * vec[0] + matrix[1] * vec[1];
    target_vec[1] = matrix[2] * vec[0] + matrix[3] * vec[1];
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

