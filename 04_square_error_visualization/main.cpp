#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <random>

#define NUMBER_OF_POINTS 250

#define WINDOW_SIZE 500

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

float square_error_sum(float* points, int* target_point){
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

void calc_z_value(float* points, float* z){
    for (int i=0; i<2*WINDOW_SIZE * 2*WINDOW_SIZE; i++){
        int coord[2] = { i / (2*WINDOW_SIZE) - WINDOW_SIZE, i % (2*WINDOW_SIZE) - WINDOW_SIZE};
        z[i] = square_error_sum(points, coord) / NUMBER_OF_POINTS;
    }
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
    
  float z[2*WINDOW_SIZE * 2*WINDOW_SIZE];
  calc_z_value(points, z);
    
  while (!glfwWindowShouldClose(window))
  {
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(75.f, -1.f, 0.f, 0.2f);
    glTranslatef(0.f, 0.f, -0.4f);
    //glRotatef(0.f, -1.f, 0.f, 0.f);
        
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
    glEnd();
      
    glPointSize(10);
    glBegin(GL_POINTS);
      
    for (int i=0; i<NUMBER_OF_POINTS/2; i++){
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(points[2*i+0]/WINDOW_SIZE, points[2*i+1]/WINDOW_SIZE, 0.f);
    }
    glEnd();
      
    glPointSize(1);
    glBegin(GL_POINTS);
    for (int i=0; i<2*WINDOW_SIZE * 2*WINDOW_SIZE; i++){
        int coord[2] = { i / (2*WINDOW_SIZE) - WINDOW_SIZE, i % (2*WINDOW_SIZE) - WINDOW_SIZE};
        float coordf[2] = {coord[0], coord[1]};
        // 40, 0.3, 26 : scale factor to make r value 0 ~ 1
        glColor3f((std::log(z[i])/40 - 0.3)*26, (std::log(z[i])/40 - 0.3)*10, (std::log(z[i])/40 - 0.3)*10);
        glVertex3f(coordf[0]/WINDOW_SIZE, coordf[1]/WINDOW_SIZE, (std::log(z[i])/40 - 0.3)*20);
    }
    glEnd();
          
    glfwSwapBuffers(window);
    glfwPollEvents();
      
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

