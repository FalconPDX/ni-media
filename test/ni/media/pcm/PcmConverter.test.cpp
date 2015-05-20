//----------------------------------------------------------------------------------------------------------------------
//!
//!  \author Marc Boucek
//!  \date   Mar/2013
//!
//!  \file PcmConvert_TEST.h
//!
//!  \brief PcmConvert Unit Test using googletest (gtest) framework
//!
//!  (c) Copyright NATIVE INSTRUMENTS, Berlin, Germany
//!  ALL RIGHTS RESERVED
//!
//----------------------------------------------------------------------------------------------------------------------
#pragma once

#include <gtest/gtest.h>

#include <ni/media/pcm/PcmConverter.h>

#include "PcmHelper.h"


// We define a test fixture class template.
template <class conversion_t>
class PcmConverterTest : public testing::Test {

  typedef typename conversion_t::source_t source_t;
  typedef typename conversion_t::target_t target_t;
protected:


  PcmConverterTest()
    : m_size(256)
  {
  };

  virtual ~PcmConverterTest() {};

  bool testConversion() const
  {
    for (size_t i = 0; i< m_size; ++i)
    {
      double x = -1.0 +  2.0 * double( i ) / m_size;
      source_t src = number::to<source_t>( x );
      target_t trg = number::to<target_t>( x );
      target_t tst = pcm::detail::convert_to<target_t>(src);

      if ( trg!=tst )
      {
        target_t tst2 = pcm::detail::convert_to<target_t>(src);
        return false;
      }
    }

    return true;
  }
  const size_t m_size;
};


// Then, declare the test case.  The argument is the name of the test
// fixture, and also the name of the test case (as usual).  The _P
// suffix is for "parameterized" or "pattern".
TYPED_TEST_CASE_P(PcmConverterTest);

// Next, use TYPED_TEST_P(TestCaseName, TestName) to define a test,
// similar to what you do with TEST_F.

TYPED_TEST_P(PcmConverterTest, ConversionEqualty) {
  EXPECT_TRUE( this->testConversion());

}


// Type-parameterized tests involve one extra step: you have to
// enumerate the tests you defined:
REGISTER_TYPED_TEST_CASE_P(
                           PcmConverterTest,  // The first argument is the test case name.
                           // The rest of the arguments are the test names.
                           ConversionEqualty);

// At this point the test pattern is done.  However, you don't have
// any real test yet as you haven't said which types you want to run
// the tests with.

// To turn the abstract test pattern into real tests, you instantiate
// it with a list of types.  Usually the test pattern will be defined
// in a .h file, and anyone can #include and instantiate it.  You can
// even instantiate it more than once in the same program.  To tell
// different instances apart, you give each of them a name, which will
// become part of the test case name and can be used in test filters.

// The list of types we want to test.  Note that it doesn't have to be
// defined at the time we write the TYPED_TEST_P()s.



INSTANTIATE_TYPED_TEST_CASE_P( SelfTest    , PcmConverterTest, SelfTypes        );
INSTANTIATE_TYPED_TEST_CASE_P( Uint8ToAll  , PcmConverterTest, Uint8ToAllTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( Uint16ToAll , PcmConverterTest, Uint16ToAllTypes );
INSTANTIATE_TYPED_TEST_CASE_P( Uint32ToAll , PcmConverterTest, Uint32ToAllTypes );
INSTANTIATE_TYPED_TEST_CASE_P( Int8ToAll   , PcmConverterTest, Int8ToAllTypes   );
INSTANTIATE_TYPED_TEST_CASE_P( Int16ToAll  , PcmConverterTest, Int16ToAllTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( Int32ToAll  , PcmConverterTest, Int32ToAllTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( FloatToAll  , PcmConverterTest, FloatToAllTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( DoubleToAll , PcmConverterTest, DoubleToAllTypes );
INSTANTIATE_TYPED_TEST_CASE_P( AllToUint8  , PcmConverterTest, AllToUint8Types  );
INSTANTIATE_TYPED_TEST_CASE_P( AllToUint16 , PcmConverterTest, AllToUint16Types );
INSTANTIATE_TYPED_TEST_CASE_P( AllToUint32 , PcmConverterTest, AllToUint32Types );
INSTANTIATE_TYPED_TEST_CASE_P( AllToInt8   , PcmConverterTest, AllToInt8Types   );
INSTANTIATE_TYPED_TEST_CASE_P( AllToInt16  , PcmConverterTest, AllToInt16Types  );
INSTANTIATE_TYPED_TEST_CASE_P( AllToInt32  , PcmConverterTest, AllToInt32Types  );
INSTANTIATE_TYPED_TEST_CASE_P( AllToFloat  , PcmConverterTest, AllToFloatTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( AllToDouble , PcmConverterTest, AllToDoubleTypes );
                                                                              

