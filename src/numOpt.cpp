#include "numOpt.hpp"

float square_error_sum(float* points, float* target_point, int number_of_points){
    float square_error_sum = 0;
    for (int i=0; i<number_of_points; i++){
        square_error_sum += pow(points[2*i+0] - target_point[0], 2) + pow(points[2*i+1] - target_point[1], 2);
    }
    return square_error_sum;
}

// overload
float square_error_sum(float* points, int* target_point, int number_of_points){
    float square_error_sum = 0;
    for (int i=0; i<number_of_points; i++){
        square_error_sum += pow(points[2*i+0] - target_point[0], 2) + pow(points[2*i+1] - target_point[1], 2);
    }
    return square_error_sum;
}

void create_random_points(float* points, int field_size, int number_of_points){
    std::mt19937 mt(0);
    std::uniform_int_distribution<int> rnd(0, field_size * 2);
    for (int i=0; i<number_of_points * 2; i++){
        points[i] = rnd(mt) - field_size;
    }
}
void create_random_points(double* points, int field_size, int number_of_points){
    std::mt19937 mt(0);
    std::uniform_int_distribution<int> rnd(0, field_size * 2);
    for (int i=0; i<number_of_points * 2; i++){
        points[i] = rnd(mt) - field_size;
    }
}

void calc_gradient(float* points, float* target_point, float* gradient, float dh, int number_of_points){
    float target_point_plus_x[2] = {target_point[0] + dh, target_point[1]     };
    float target_point_plus_y[2] = {target_point[0]     , target_point[1] + dh};
    float dx = square_error_sum(points, target_point_plus_x, number_of_points) - square_error_sum(points, target_point, number_of_points);
    float dy = square_error_sum(points, target_point_plus_y, number_of_points) - square_error_sum(points, target_point, number_of_points);
    gradient[0] = dx / dh;
    gradient[1] = dy / dh;
}

void analyze_gradient_hessian(float* points, float* target_point, float* gradient, float* hessian, int number_of_points){
    gradient[0] = 0;
    gradient[1] = 0;
    hessian[0] = 0;
    hessian[1] = 0;
    hessian[2] = 0;
    hessian[3] = 0;
    for (int i=0; i<number_of_points; i++){
        gradient[0] += 2*(target_point[0] - points[2*i+0]);
        gradient[1] += 2*(target_point[1] - points[2*i+1]);
        hessian[0] += 2;
        hessian[1] += 0;
        hessian[2] += 0;
        hessian[3] += 2;
    }
}

void calc_hessian_factors(float* points, float* target_point, float* hessian_factors, float dh, int number_of_points){
    // hessian_factors[3]:
    // Hessian = ( hessian_factors[0] hessian_factors[1] )
    //           ( hessian_factors[1] hessian_factors[2] )
    float target_point_plus_x[2] =  {target_point[0] + dh, target_point[1]     };
    float target_point_plus_y[2] =  {target_point[0]     , target_point[1] + dh};
    float target_point_minus_x[2] = {target_point[0] - dh, target_point[1]     };
    float target_point_minus_y[2] = {target_point[0]     , target_point[1] - dh};
    float target_point_plus_xy[2] = {target_point[0] + dh, target_point[1] + dh};
    float dx2 =       square_error_sum(points, target_point_plus_x, number_of_points)
                - 2 * square_error_sum(points, target_point, number_of_points)
                    + square_error_sum(points, target_point_minus_x, number_of_points);
    float dy2 =       square_error_sum(points, target_point_plus_y, number_of_points)
                - 2 * square_error_sum(points, target_point, number_of_points)
                    + square_error_sum(points, target_point_minus_y, number_of_points);
    float dxdy =   square_error_sum(points, target_point_plus_xy, number_of_points)
                 - square_error_sum(points, target_point_plus_x, number_of_points)
                 - square_error_sum(points, target_point_plus_y, number_of_points)
                 + square_error_sum(points, target_point, number_of_points);
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
//    inverse_matrix[0] = matrix[2] / det;
//    inverse_matrix[1] = -matrix[1] / det;
//    inverse_matrix[2] = -matrix[1] / det;
//    inverse_matrix[3] = matrix[0] / det;
    inverse_matrix[0] = matrix[3] / det;
    inverse_matrix[1] = -matrix[1] / det;
    inverse_matrix[2] = -matrix[2] / det;
    inverse_matrix[3] = matrix[0] / det;
}

void mat_dot_vec(float* matrix, float* vec, float* target_vec){
    target_vec[0] = matrix[0] * vec[0] + matrix[1] * vec[1];
    target_vec[1] = matrix[2] * vec[0] + matrix[3] * vec[1];
}

void calc_z_value(float* points, float* z, int field_size, int number_of_points){
    for (int i=0; i<2*field_size * 2*field_size; i++){
        int coord[2] = { i / (2*field_size) - field_size, i % (2*field_size) - field_size};
        z[i] = square_error_sum(points, coord, number_of_points) / number_of_points;
    }
}
