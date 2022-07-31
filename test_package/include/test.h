#ifndef TEST_HEADER
#define TEST_HEADER

#ifdef _WIN32
  #define cppSide_EXPORT __declspec(dllexport)
#else
  #define cppSide_EXPORT
#endif

//test_EXPORT void test();
#include "test.pb.h"

#endif //TEST_HEADER
