
#include <galcore/DebugProfile.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
#ifndef NDEBUG
  gal::debug::enableDebugging();
#endif

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
