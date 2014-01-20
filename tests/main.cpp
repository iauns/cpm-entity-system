
#include <cstdlib>
#include <iostream>
#include <exception>
#include <memory>
#include <ctime>

#include <gtest/gtest.h>

uint64_t gRandomSeed = 0;

int main(int argc, char** argv)
{
  gRandomSeed = std::time(NULL);

  std::cout << "Random seed for all tests: " << gRandomSeed << std::endl;

  // Add a global test environment that initializes an OpenGL batch renderer.
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();

  // Print out the random seed used for all tests. Just in case we had a failure
  // on one of the tests, and we need to reproduce the results in order to
  // debug.
  std::cout << "\nRandom seed for all tests: " << gRandomSeed << std::endl << std::endl;
  return ret;
}

