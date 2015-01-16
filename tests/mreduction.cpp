#include <cmath>
#include "common.hpp"
#include "atidlas/array.h"

namespace ad = atidlas;

template<typename T>
void test_row_wise_reduction(T epsilon, simple_vector_base<T> & cy, simple_matrix_base<T> const & cA, simple_vector_base<T> & cx,
                                        ad::array & y, ad::array const & A, ad::array & x)
{
  int failure_count = 0;

  ad::int_t M = A.shape()._1;
  ad::int_t N = A.shape()._2;

  simple_vector<T> buffer(M);

  T yi = 0, xi = 0;
#define TEST_OPERATION(NAME, SIZE1, SIZE2, REDUCTION, ASSIGNMENT, GPU_REDUCTION)\
  std::cout << NAME "..." << std::flush;\
  for(int i = 0 ; i < SIZE1 ; ++i)\
  {\
    yi = 0;\
    xi = 0;\
    for(int j = 0 ; j < SIZE2 ; ++j)\
      REDUCTION;\
    ASSIGNMENT;\
  }\
  GPU_REDUCTION;\
  ad::copy(y, buffer.data());\
  if(failure_vector(cy, buffer, epsilon))\
  {\
    failure_count++;\
    std::cout << " [Failure!]" << std::endl;\
  }\
  else\
    std::cout << std::endl;

  TEST_OPERATION("y = A.x", M, N, yi+=cA(i,j)*cx[j], cy[i] = yi, y = dot(A,x));
  TEST_OPERATION("x = A'.y", N, M, xi+=cA(j, i)*cy[j], cx[i] = xi, x = dot(trans(A),y));

  if(failure_count>0)
    exit(EXIT_FAILURE);
}

template<typename T>
void test_impl(T epsilon)
{
  int_t M = 1324;
  int_t N = 1143;
  int_t SUBM = 184;
  int_t SUBN = 145;

  INIT_MATRIX(M, SUBM, 5, 3, N, SUBN, 7, 2, cA, A);
  INIT_VECTOR(M, SUBM, 6, 2, cy, y);
  INIT_VECTOR(N, SUBN, 4, 3, cx, x);

  test_row_wise_reduction(epsilon, cy_vector, cA_matrix, cx_vector, y_vector, A_matrix, x_vector);
}

int main()
{
  std::cout << ">>> float" << std::endl;
  test_impl<float>(1e-4);
  std::cout << ">>> double" << std::endl;
  test_impl<double>(1e-9);
  std::cout << "---" << std::endl;
  std::cout << "Passed" << std::endl;

  return EXIT_SUCCESS;
}