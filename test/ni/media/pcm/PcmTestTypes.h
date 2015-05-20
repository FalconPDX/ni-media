//----------------------------------------------------------------------------------------------------------------------
//!
//!  \author Marc Boucek
//!  \date   Mar/2013
//!
//!  \file PcmTestTypes.h
//!
//!  \brief defines types for pcm conversion tests
//!
//!  (c) Copyright NATIVE INSTRUMENTS, Berlin, Germany
//!  ALL RIGHTS RESERVED
//!
//----------------------------------------------------------------------------------------------------------------------
#pragma once

#include <ni/media/pcm/format.h>
#include <ni/media/pcm/traits.h>

#include <gtest/gtest.h>

#include <limits>
#include <vector>
#include <list>

#include <cstdint>


namespace number
{
  template <typename value_t>
  struct scale
  {
    static double value()
    {
      return static_cast<double>( 1ULL << ( sizeof(value_t) * 8 - 1 ) );
    }

  };

  template<> struct scale<double>{ static double value() { return( 1.0 ); } };
  template<> struct scale<float>{ static double value() { return( 1.0 ); } };


  template <typename value_t>
  struct min
  {
    static double value()
    {
      return static_cast<double>( std::numeric_limits<value_t>::min() );
    }
  };

  template<> struct min<double>{ static double value(){ return -1.0; } };
  template<> struct min<float>{ static double value(){ return -1.0; } };

  template <typename value_t>
  struct max
  {
    static double value()
    {
      return static_cast<double>( std::numeric_limits<value_t>::max() );
    }
  };

  template<> struct max<double>{ static double value(){ return 1.0; } };
  template<> struct max<float>{ static double value(){ return 1.0; } };

  template <typename value_t>
  struct offset
  {
    static double value()
    {
      return ( min<value_t>::value() + max<value_t>::value() + 1.0 ) / 2.0 ;
    }
  };

  template<> struct offset<double>{ static double value(){ return 0.0; } };
  template<> struct offset<float>{ static double value(){ return 0.0; } };

  template<class value_t>
  struct round
  {
    static double f( double val )
    {
      return val < 0.0 ? std::ceil(val - 0.5) : std::floor(val + 0.5);
    }
  };

  template<> struct round<double>{ static double f( double val ){ return val; } };
  template<> struct round<float>{ static double f( double val ){ return val; } };


  template<class value_t>
  struct clip
  {
    static double f( double val )
    {
      return std::min( max<value_t>::value(), std::max( min<value_t>::value(), val ) );
    }
  };


  template<class value_t>
  value_t to( double val )
  {
    const double scl = scale<value_t>::value();
    const double offs = offset<value_t>::value();
    const double tmp = val * scl + offs;

    return static_cast<value_t>( round<value_t>::f ( clip<value_t>::f (tmp) ) );
  }

  template<class value_t>
  double from( value_t val )
  {
    return ( static_cast<double>( val ) - offset<value_t>::value() )  / scale<value_t>::value() ;
  }


} // namespace number


template <class target_t, class source_t>
struct ts
{
  typedef target_t target_t;
  typedef source_t source_t;
};

template <class source_t, class target_t>
struct st
{
  typedef target_t target_t;
  typedef source_t source_t;
};


typedef testing::Types<
st< uint8_t  , uint8_t  > ,
st< uint16_t , uint16_t > ,
st< uint32_t , uint32_t > ,
st< int8_t   , int8_t   > ,
st< int16_t  , int16_t  > ,
st< int32_t  , int32_t  > ,
st< float    , float    > ,
st< double   , double   >
> SelfTypes;

typedef testing::Types<
st< uint8_t , uint16_t > ,
st< uint8_t , uint32_t > ,
st< uint8_t , int8_t   > ,
st< uint8_t , int16_t  > ,
st< uint8_t , int32_t  > ,
st< uint8_t , float    > ,
st< uint8_t , double   >
> Uint8ToAllTypes;

typedef testing::Types<
st< uint16_t , uint8_t  > ,
st< uint16_t , uint32_t > ,
st< uint16_t , int8_t   > ,
st< uint16_t , int16_t  > ,
st< uint16_t , int32_t  > ,
st< uint16_t , float    > ,
st< uint16_t , double   >
> Uint16ToAllTypes;

typedef testing::Types<
st< uint32_t , uint8_t  > ,
st< uint32_t , uint16_t > ,
st< uint32_t , int8_t   > ,
st< uint32_t , int16_t  > ,
st< uint32_t , int32_t  > ,
st< uint32_t , float    > ,
st< uint32_t , double   >
> Uint32ToAllTypes;


typedef testing::Types<
st< int8_t , uint8_t  > ,
st< int8_t , uint16_t > ,
st< int8_t , uint32_t > ,
st< int8_t , int16_t  > ,
st< int8_t , int32_t  > ,
st< int8_t , float    > ,
st< int8_t , double   >
> Int8ToAllTypes;

typedef testing::Types<
st< int16_t , uint8_t  > ,
st< int16_t , uint16_t > ,
st< int16_t , uint32_t > ,
st< int16_t , int8_t   > ,
st< int16_t , int32_t  > ,
st< int16_t , float    > ,
st< int16_t , double   >
> Int16ToAllTypes;

typedef testing::Types<
st< int32_t , uint8_t  > ,
st< int32_t , uint16_t > ,
st< int32_t , uint32_t > ,
st< int32_t , int8_t   > ,
st< int32_t , int16_t  > ,
st< int32_t , float    > ,
st< int32_t , double   >
> Int32ToAllTypes;

typedef testing::Types<
st< float , uint8_t  > ,
st< float , uint16_t > ,
st< float , uint32_t > ,
st< float , int8_t   > ,
st< float , int16_t  > ,
st< float , int32_t  > ,
st< float , double   >
> FloatToAllTypes;

typedef testing::Types<
st< double , uint8_t  > ,
st< double , uint16_t > ,
st< double , uint32_t > ,
st< double , int8_t   > ,
st< double , int16_t  > ,
st< double , int32_t  > ,
st< double , float    >
> DoubleToAllTypes;

typedef testing::Types<
ts< uint8_t , uint16_t > ,
ts< uint8_t , uint32_t > ,
ts< uint8_t , int8_t   > ,
ts< uint8_t , int16_t  > ,
ts< uint8_t , int32_t  > ,
ts< uint8_t , float    > ,
ts< uint8_t , double   >
> AllToUint8Types;

typedef testing::Types<
ts< uint16_t , uint8_t  > ,
ts< uint16_t , uint32_t > ,
ts< uint16_t , int8_t   > ,
ts< uint16_t , int16_t  > ,
ts< uint16_t , int32_t  > ,
ts< uint16_t , float    > ,
ts< uint16_t , double   >
> AllToUint16Types;

typedef testing::Types<
ts< uint32_t , uint8_t  > ,
ts< uint32_t , uint16_t > ,
ts< uint32_t , int8_t   > ,
ts< uint32_t , int16_t  > ,
ts< uint32_t , int32_t  > ,
ts< uint32_t , float    > ,
ts< uint32_t , double   >
> AllToUint32Types;


typedef testing::Types<
ts< int8_t , uint8_t  > ,
ts< int8_t , uint16_t > ,
ts< int8_t , uint32_t > ,
ts< int8_t , int16_t  > ,
ts< int8_t , int32_t  > ,
ts< int8_t , float    > ,
ts< int8_t , double   >
> AllToInt8Types;

typedef testing::Types<
ts< int16_t , uint8_t  > ,
ts< int16_t , uint16_t > ,
ts< int16_t , uint32_t > ,
ts< int16_t , int8_t   > ,
ts< int16_t , int32_t  > ,
ts< int16_t , float    > ,
ts< int16_t , double   >
> AllToInt16Types;

typedef testing::Types<
ts< int32_t , uint8_t  > ,
ts< int32_t , uint16_t > ,
ts< int32_t , uint32_t > ,
ts< int32_t , int8_t   > ,
ts< int32_t , int16_t  > ,
ts< int32_t , float    > ,
ts< int32_t , double   >
> AllToInt32Types;

typedef testing::Types<
ts< float , uint8_t  > ,
ts< float , uint16_t > ,
ts< float , uint32_t > ,
ts< float , int8_t   > ,
ts< float , int16_t  > ,
ts< float , int32_t  > ,
ts< float , double   >
> AllToFloatTypes;

typedef testing::Types<
ts< double , uint8_t  > ,
ts< double , uint16_t > ,
ts< double , uint32_t > ,
ts< double , int8_t   > ,
ts< double , int16_t  > ,
ts< double , int32_t  > ,
ts< double , float    >
> AllToDoubleTypes;


template
< class value_t
, class format_t
, class src_container_t = std::vector<value_t>
, class dst_container_t = std::vector<value_t>
, class raw_container_t = std::list<uint8_t>
>
struct vf
{
  typedef value_t         ValueType;
  typedef format_t        PcmFormat;
  typedef src_container_t SrcContainer;
  typedef dst_container_t DstContainer;
  typedef raw_container_t RawContainer;
};


typedef testing::Types<
vf< uint8_t , pcm::format::f32be > ,
vf< uint8_t , pcm::format::f32le > ,
vf< uint8_t , pcm::format::f64be > ,
vf< uint8_t , pcm::format::f64le > ,
vf< uint8_t , pcm::format::s8 > ,
vf< uint8_t , pcm::format::s16be > ,
vf< uint8_t , pcm::format::s16le > ,
vf< uint8_t , pcm::format::s24be > ,
vf< uint8_t , pcm::format::s24le > ,
vf< uint8_t , pcm::format::s32be > ,
vf< uint8_t , pcm::format::s32le > ,
vf< uint8_t , pcm::format::u8 > ,
vf< uint8_t , pcm::format::u16be > ,
vf< uint8_t , pcm::format::u16le > ,
vf< uint8_t , pcm::format::u24be > ,
vf< uint8_t , pcm::format::u24le > ,
vf< uint8_t , pcm::format::u32be > ,
vf< uint8_t , pcm::format::u32le >
> Uint8ToAllPcmTypes;

typedef testing::Types<
vf< uint16_t , pcm::format::f32be > ,
vf< uint16_t , pcm::format::f32le > ,
vf< uint16_t , pcm::format::f64be > ,
vf< uint16_t , pcm::format::f64le > ,
vf< uint16_t , pcm::format::s8 > ,
vf< uint16_t , pcm::format::s16be > ,
vf< uint16_t , pcm::format::s16le > ,
vf< uint16_t , pcm::format::s24be > ,
vf< uint16_t , pcm::format::s24le > ,
vf< uint16_t , pcm::format::s32be > ,
vf< uint16_t , pcm::format::s32le > ,
vf< uint16_t , pcm::format::u8 > ,
vf< uint16_t , pcm::format::u16be > ,
vf< uint16_t , pcm::format::u16le > ,
vf< uint16_t , pcm::format::u24be > ,
vf< uint16_t , pcm::format::u24le > ,
vf< uint16_t , pcm::format::u32be > ,
vf< uint16_t , pcm::format::u32le >
> Uint16ToAllPcmTypes;

typedef testing::Types<
vf< uint32_t , pcm::format::f32be > ,
vf< uint32_t , pcm::format::f32le > ,
vf< uint32_t , pcm::format::f64be > ,
vf< uint32_t , pcm::format::f64le > ,
vf< uint32_t , pcm::format::s8 > ,
vf< uint32_t , pcm::format::s16be > ,
vf< uint32_t , pcm::format::s16le > ,
vf< uint32_t , pcm::format::s24be > ,
vf< uint32_t , pcm::format::s24le > ,
vf< uint32_t , pcm::format::s32be > ,
vf< uint32_t , pcm::format::s32le > ,
vf< uint32_t , pcm::format::u8 > ,
vf< uint32_t , pcm::format::u16be > ,
vf< uint32_t , pcm::format::u16le > ,
vf< uint32_t , pcm::format::u24be > ,
vf< uint32_t , pcm::format::u24le > ,
vf< uint32_t , pcm::format::u32be > ,
vf< uint32_t , pcm::format::u32le >
> Uint32ToAllPcmTypes;

typedef testing::Types<
vf< int8_t , pcm::format::f32be > ,
vf< int8_t , pcm::format::f32le > ,
vf< int8_t , pcm::format::f64be > ,
vf< int8_t , pcm::format::f64le > ,
vf< int8_t , pcm::format::s8 > ,
vf< int8_t , pcm::format::s16be > ,
vf< int8_t , pcm::format::s16le > ,
vf< int8_t , pcm::format::s24be > ,
vf< int8_t , pcm::format::s24le > ,
vf< int8_t , pcm::format::s32be > ,
vf< int8_t , pcm::format::s32le > ,
vf< int8_t , pcm::format::u8 > ,
vf< int8_t , pcm::format::u16be > ,
vf< int8_t , pcm::format::u16le > ,
vf< int8_t , pcm::format::u24be > ,
vf< int8_t , pcm::format::u24le > ,
vf< int8_t , pcm::format::u32be > ,
vf< int8_t , pcm::format::u32le >
> Int8ToAllPcmTypes;

typedef testing::Types<
vf< int16_t , pcm::format::f32be > ,
vf< int16_t , pcm::format::f32le > ,
vf< int16_t , pcm::format::f64be > ,
vf< int16_t , pcm::format::f64le > ,
vf< int16_t , pcm::format::s8 > ,
vf< int16_t , pcm::format::s16be > ,
vf< int16_t , pcm::format::s16le > ,
vf< int16_t , pcm::format::s24be > ,
vf< int16_t , pcm::format::s24le > ,
vf< int16_t , pcm::format::s32be > ,
vf< int16_t , pcm::format::s32le > ,
vf< int16_t , pcm::format::u8 > ,
vf< int16_t , pcm::format::u16be > ,
vf< int16_t , pcm::format::u16le > ,
vf< int16_t , pcm::format::u24be > ,
vf< int16_t , pcm::format::u24le > ,
vf< int16_t , pcm::format::u32be > ,
vf< int16_t , pcm::format::u32le >
> Int16ToAllPcmTypes;

typedef testing::Types<
vf< int32_t , pcm::format::f32be > ,
vf< int32_t , pcm::format::f32le > ,
vf< int32_t , pcm::format::f64be > ,
vf< int32_t , pcm::format::f64le > ,
vf< int32_t , pcm::format::s8 > ,
vf< int32_t , pcm::format::s16be > ,
vf< int32_t , pcm::format::s16le > ,
vf< int32_t , pcm::format::s24be > ,
vf< int32_t , pcm::format::s24le > ,
vf< int32_t , pcm::format::s32be > ,
vf< int32_t , pcm::format::s32le > ,
vf< int32_t , pcm::format::u8 > ,
vf< int32_t , pcm::format::u16be > ,
vf< int32_t , pcm::format::u16le > ,
vf< int32_t , pcm::format::u24be > ,
vf< int32_t , pcm::format::u24le > ,
vf< int32_t , pcm::format::u32be > ,
vf< int32_t , pcm::format::u32le >
> Int32ToAllPcmTypes;


typedef testing::Types<
vf< float , pcm::format::f32be > ,
vf< float , pcm::format::f32le > ,
vf< float , pcm::format::f64be > ,
vf< float , pcm::format::f64le > ,
vf< float , pcm::format::s8 > ,
vf< float , pcm::format::s16be > ,
vf< float , pcm::format::s16le > ,
vf< float , pcm::format::s24be > ,
vf< float , pcm::format::s24le > ,
vf< float , pcm::format::s32be > ,
vf< float , pcm::format::s32le > ,
vf< float , pcm::format::u8 > ,
vf< float , pcm::format::u16be > ,
vf< float , pcm::format::u16le > ,
vf< float , pcm::format::u24be > ,
vf< float , pcm::format::u24le > ,
vf< float , pcm::format::u32be > ,
vf< float , pcm::format::u32le >
> FloatToAllPcmTypes;

typedef testing::Types<
vf< double , pcm::format::f32be > ,
vf< double , pcm::format::f32le > ,
vf< double , pcm::format::f64be > ,
vf< double , pcm::format::f64le > ,
vf< double , pcm::format::s8 > ,
vf< double , pcm::format::s16be > ,
vf< double , pcm::format::s16le > ,
vf< double , pcm::format::s24be > ,
vf< double , pcm::format::s24le > ,
vf< double , pcm::format::s32be > ,
vf< double , pcm::format::s32le > ,
vf< double , pcm::format::u8 > ,
vf< double , pcm::format::u16be > ,
vf< double , pcm::format::u16le > ,
vf< double , pcm::format::u24be > ,
vf< double , pcm::format::u24le > ,
vf< double , pcm::format::u32be > ,
vf< double , pcm::format::u32le >
> DoubleToAllPcmTypes;
