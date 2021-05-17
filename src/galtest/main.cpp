
#include <gtest/gtest.h>

void applyFilter(int& argc, char**& argv)
{
  //   static std::string filter = "--gtest_filter=MeshFunction.Centroid";
  static std::string filter = "--gtest_filter=Circle2d.MinBoundingCircle";
  if (argc == 1) {
    char** newArgs = new char*[2];
    newArgs[0]     = argv[0];
    newArgs[1]     = filter.data();
    argv           = newArgs;
    argc           = 2;
  }
}

int main(int argc, char** argv)
{
  /*
  applyFilter(argc, argv);
   */

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}