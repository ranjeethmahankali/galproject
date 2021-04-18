
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
  /*
  std::string filter = "--gtest_filter=*DragonMesh";
  char** newArgs = new char*[2];
  newArgs[0] = argv[0];
  newArgs[1] = filter.data();
  argv = newArgs;
  argc = 2;
  */

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}