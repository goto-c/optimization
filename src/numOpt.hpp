#pragma once

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <random>

float square_error_sum(float* points, float* target_point, int number_of_points);
float square_error_sum(float* points, int* target_point, int number_of_points);

void create_random_points(float* points, int field_size, int number_of_points);
void create_random_points(double* points, int field_size, int number_of_points);

void calc_gradient(float* points, float* target_point, float* gradient, float dh, int number_of_points);

void calc_hessian_factors(float* points, float* target_point, float* hessian_factors, float dh, int number_of_points);

void normalize_mat(float* matrix);

void normalize_vec(float* vec);

void calc_inverse_matrix(float* matrix, float* inverse_matrix);

void mat_dot_vec(float* matrix, float* vec, float* target_vec);

void calc_z_value(float* points, float* z, int field_size, int number_of_points);
