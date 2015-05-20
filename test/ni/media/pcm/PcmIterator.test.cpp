//----------------------------------------------------------------------------------------------------------------------
//!
//!  \author Marc Boucek
//!  \date   Mar/2013
//!
//!  \file PcmIterator_TEST.h
//!
//!  \brief PcmIterator Unit Test using googletest (gtest) framework
//!
//!  (c) Copyright NATIVE INSTRUMENTS, Berlin, Germany
//!  ALL RIGHTS RESERVED
//!
//----------------------------------------------------------------------------------------------------------------------
#include "PcmHelper.h"

#include <gtest/gtest.h>

#include <ni/media/pcm/PcmIterator.h>

#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <algorithm>

// Then we define a test fixture class template.
template <class Traits>
class PcmIteratorTest : public testing::Test {


private:

  typedef typename Traits::ValueType                 ValueType;
  typedef typename Traits::PcmFormat                 PcmFormat;

  typedef typename Traits::SrcContainer              SrcContainer;
  typedef typename SrcContainer::iterator            SrcIterator;

  typedef typename Traits::DstContainer              DstContainer;
  typedef typename DstContainer::iterator            DstIterator;

  typedef typename Traits::RawContainer              RawContainer;
  typedef typename RawContainer::iterator            RawIterator;

  typedef pcm::iterator<ValueType, RawIterator>      PcmIterator;

  SrcContainer            m_srcContainer;
  SrcIterator             m_srcBeg;
  SrcIterator             m_srcEnd;

  DstContainer            m_dstContainer;
  DstIterator             m_dstBeg;
  DstIterator             m_dstEnd;

  RawContainer            m_rawContainer;
  RawIterator             m_rawBeg;
  RawIterator             m_rawEnd;

  PcmIterator             m_pcmBeg;
  PcmIterator             m_pcmEnd;

protected:

  PcmIteratorTest()
    : m_srcContainer(256)
    , m_srcBeg( m_srcContainer.begin() )
    , m_srcEnd( m_srcContainer.end()   )
    , m_dstContainer( m_srcContainer.size() ) 
    , m_dstBeg( m_dstContainer.begin() )
    , m_dstEnd( m_dstContainer.end()   )
    , m_rawContainer( m_srcContainer.size() * PcmFormat::bitwidth_t::value / 8 )
    , m_rawBeg( m_rawContainer.begin() )
    , m_rawEnd( m_rawContainer.end()   )
    , m_pcmBeg( m_rawBeg, PcmFormat()  )
    , m_pcmEnd( m_rawEnd, PcmFormat()  )
  {

    for (size_t i = 0; i< m_srcContainer.size(); ++i)
    {
      double x = -1.0 +  2.0 * double( i ) / m_srcContainer.size();
      m_srcContainer[i] = number::to<ValueType>( x );
    }
  };

  virtual ~PcmIteratorTest() {};


//---------------------------------------------------------------------------

  bool custom_copy_to_pcm() 
  {
    return ( m_rawEnd == pcm::write<PcmFormat>( m_srcBeg, m_srcEnd, m_rawBeg ) );
  }

  bool custom_copy_from_pcm() 
  {
    return ( m_dstEnd == pcm::read<PcmFormat>( m_rawBeg, m_rawEnd, m_dstBeg ) );
  }

  bool custom_copy_to_from_pcm() 
  {
    return custom_copy_to_pcm() && custom_copy_from_pcm() && equal();
  }


//---------------------------------------------------------------------------

  bool std_copy_to_pcm()  
  {
    return ( m_pcmEnd == std::copy( m_srcBeg, m_srcEnd, m_pcmBeg ) );
  }

  bool std_copy_from_pcm()  
  {
    return ( m_dstEnd == std::copy( m_pcmBeg, m_pcmEnd, m_dstBeg ) );
  }

  bool std_copy_to_from_pcm() 
  {
    return std_copy_to_pcm() && std_copy_from_pcm() && equal();
  }


//---------------------------------------------------------------------------

  bool std_copy_n_to_pcm()  
  {
    return ( m_pcmEnd == std::copy_n( m_srcBeg, std::distance( m_srcBeg, m_srcEnd ), m_pcmBeg ) );
  }

  bool std_copy_n_from_pcm()  
  {
   return ( m_dstEnd == std::copy_n( m_pcmBeg, std::distance( m_pcmBeg, m_pcmEnd ), m_dstBeg ) );
  }

  bool std_copy_n_to_from_pcm() 
  {
    return std_copy_n_to_pcm() && std_copy_n_from_pcm() && equal();
  }


//---------------------------------------------------------------------------

  bool boost_range_copy_to_pcm()  
  {
    typedef boost::iterator_range<SrcIterator> Range;

    return ( m_pcmEnd == copy( Range( m_srcBeg, m_srcEnd ), m_pcmBeg ) );
  }

  bool boost_range_copy_from_pcm()  
  {
    typedef boost::iterator_range<PcmIterator> Range;

    return ( m_dstEnd == copy( Range( m_pcmBeg, m_pcmEnd ), m_dstBeg ) );
  }

  bool boost_range_copy_to_from_pcm() 
  {
    return boost_range_copy_to_pcm() && boost_range_copy_from_pcm() && equal();
  }


//---------------------------------------------------------------------------

  bool equal() const
  {
    return std::equal( m_srcBeg, m_srcEnd, m_dstBeg );
  }

};

// Declare a parametrized test case. 
TYPED_TEST_CASE_P(PcmIteratorTest);

// Define parametrized test cases

// custom tests
TYPED_TEST_P(PcmIteratorTest, custom_copy_to_pcm_test) 
{
  EXPECT_TRUE( this->custom_copy_to_pcm() );
}

TYPED_TEST_P(PcmIteratorTest, custom_copy_from_pcm_test)
{
  EXPECT_TRUE( this->custom_copy_from_pcm() );
}

TYPED_TEST_P(PcmIteratorTest, custom_copy_to_from_pcm_test) 
{
  EXPECT_TRUE( this->custom_copy_to_from_pcm() );
}


// std::copy tests
TYPED_TEST_P( PcmIteratorTest, std_copy_to_pcm_test )
{
  EXPECT_TRUE( this->std_copy_to_pcm() );
}

TYPED_TEST_P( PcmIteratorTest, std_copy_from_pcm_test )
{
  EXPECT_TRUE( this->std_copy_from_pcm() );
}

TYPED_TEST_P(PcmIteratorTest, std_copy_to_from_pcm_test) 
{
  EXPECT_TRUE( this->std_copy_to_from_pcm() );
}

// std::copy_n tests
TYPED_TEST_P( PcmIteratorTest, std_copy_n_to_pcm_test )
{
  EXPECT_TRUE( this->std_copy_n_to_pcm() );
}

TYPED_TEST_P( PcmIteratorTest, std_copy_n_from_pcm_test )
{
  EXPECT_TRUE( this->std_copy_n_from_pcm() );
}

TYPED_TEST_P(PcmIteratorTest, std_copy_n_to_from_pcm_test) 
{
  EXPECT_TRUE( this->std_copy_n_to_from_pcm() );
}

// boost range tests
TYPED_TEST_P( PcmIteratorTest, boost_range_copy_to_pcm_test )
{
  EXPECT_TRUE( this->boost_range_copy_to_pcm() );
}

TYPED_TEST_P( PcmIteratorTest, boost_range_copy_from_pcm_test )
{
  EXPECT_TRUE( this->boost_range_copy_from_pcm() );
}

TYPED_TEST_P(PcmIteratorTest, boost_range_copy_to_from_pcm_test) 
{
  EXPECT_TRUE( this->boost_range_copy_to_from_pcm() );
}

// Type-parameterized tests involve one extra step: you have to
// enumerate the tests you defined:
REGISTER_TYPED_TEST_CASE_P( PcmIteratorTest, // The first argument is the test case name.
                           // The rest of the arguments are the test names.
                            custom_copy_to_pcm_test,
                            custom_copy_from_pcm_test,
                            custom_copy_to_from_pcm_test,
                            std_copy_to_pcm_test,
                            std_copy_from_pcm_test,
                            std_copy_to_from_pcm_test,
                            std_copy_n_to_pcm_test,
                            std_copy_n_from_pcm_test,
                            std_copy_n_to_from_pcm_test,
                            boost_range_copy_to_pcm_test,
                            boost_range_copy_from_pcm_test,
                            boost_range_copy_to_from_pcm_test
                          );

INSTANTIATE_TYPED_TEST_CASE_P( Uint8ToAll  , PcmIteratorTest, Uint8ToAllPcmTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( Uint16ToAll , PcmIteratorTest, Uint16ToAllPcmTypes );
INSTANTIATE_TYPED_TEST_CASE_P( Uint32ToAll , PcmIteratorTest, Uint32ToAllPcmTypes );
INSTANTIATE_TYPED_TEST_CASE_P( Int8ToAll   , PcmIteratorTest, Int8ToAllPcmTypes   );
INSTANTIATE_TYPED_TEST_CASE_P( Int16ToAll  , PcmIteratorTest, Int16ToAllPcmTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( Int32ToAll  , PcmIteratorTest, Int32ToAllPcmTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( FloatToAll  , PcmIteratorTest, FloatToAllPcmTypes  );
INSTANTIATE_TYPED_TEST_CASE_P( DoubleToAll , PcmIteratorTest, DoubleToAllPcmTypes );

                                                                            