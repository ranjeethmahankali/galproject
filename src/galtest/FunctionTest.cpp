#include <gtest/gtest.h>
#include <galfunc/Functions.h>

TEST(Function, StringConstant) {
    using namespace gal::func;
    using namespace std::string_literals;
    auto path = py_constant<std::string>("test value for string"s);
};