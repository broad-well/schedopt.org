#include "array.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Array ADT") {
  TEST_CASE("Array Big-Three does not cause memory problems") {
    Array<int> m1(10);
    Array<int> m2(m1);
    Array<int> m3(m1);
    Array<int> m4(4);
    Array<int> m5(2);
    m5 = m4;

    CHECK_EQ(m2.Size(), 10);
    CHECK_EQ(m3.Size(), 10);
    CHECK_EQ(m1[0], m2[0]);
    CHECK_EQ(m1[0], m3[0]);
    
    m4 = m4;
    CHECK_EQ(m4.Size(), 4);
    m4 = m3;
    CHECK_EQ(m4.Size(), 10);
    CHECK_EQ(m4[4], m3[4]);
    m1 = m5;
    CHECK_EQ(m2.Size(), 10);
    CHECK_EQ(m1.Size(), 4);
    m3 = m1;
    CHECK_EQ(m3.Size(), 4);
    CHECK_EQ(m3[0], m4[0]);
  }
}