#include <gtest/gtest.h>

int main( int argNum , char** args )
{
  ::testing::InitGoogleTest( &argNum, args );
  return RUN_ALL_TESTS();
}
