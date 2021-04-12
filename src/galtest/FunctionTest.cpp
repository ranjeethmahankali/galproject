#include <gtest/gtest.h>
#include <galfunc/Functions.h>
#include <galfunc/Variable.h>
#include <galfunc/Types.h>

TEST(Function, StringConstant) {
    using namespace gal::func;
    using namespace std::string_literals;
    auto path = store::makeVariable<std::string>("test value for string"s);
    ASSERT_TRUE(path->outputRegister(0) != 0);
};
