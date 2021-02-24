#pragma once

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <random>

#define NUMBER_OF_POINTS 250

float square_error_sum(float* points, float* target_point);
float square_error_sum(float* points, int* target_point);

void create_random_points(float* points, int point_limit);

void calc_gradient(float* points, float* target_point, float* gradient, float dh);

void calc_hessian_factors(float* points, float* target_point, float* hessian_factors, float dh);

void normalize_mat(float* matrix);

void normalize_vec(float* vec);

void calc_inverse_matrix(float* matrix, float* inverse_matrix);

void mat_dot_vec(float* matrix, float* vec, float* target_vec);

void calc_z_value(float* points, float* z, int point_limit);
