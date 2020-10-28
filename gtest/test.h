//
// Created by Chestnut on 2020/11/24.
//

#ifndef OHMYDBMS_TEST_H
#define OHMYDBMS_TEST_H

#include <iostream>
#include <sstream>

namespace testing
{
namespace internal
{
//enum GTestColor {
//    COLOR_DEFAULT,
//    COLOR_RED,
//    COLOR_GREEN,
//    COLOR_YELLOW
//};

extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
}
}
#define PRINTF(...)  do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)

// C++ stream interface
class TestCout : public std::stringstream
{
  public:
    ~TestCout()
    {
        PRINTF("%s\n",str().c_str());
    }
};

#define TEST_COUT  TestCout()

#endif // OHMYDBMS_TEST_H
