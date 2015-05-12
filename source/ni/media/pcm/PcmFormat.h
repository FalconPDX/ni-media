//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Oct/2013
//!
//! \class PcmFormat
//!
//! \brief Pcm Format Description.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------
#pragma once

#include "PcmTraits.h"

namespace pcm {

  struct format
  {
    typedef ::pcm::format_tag< signed_integer   , _8bit  , native_endian >   s8;
    typedef format_tag< unsigned_integer , _8bit  , native_endian >   u8;

    typedef format_tag< signed_integer   , _16bit , big_endian    >   s16be;
    typedef format_tag< signed_integer   , _16bit , little_endian >   s16le;
    typedef format_tag< signed_integer   , _16bit , native_endian >   s16ne;

    typedef format_tag< unsigned_integer , _16bit , big_endian    >   u16be;
    typedef format_tag< unsigned_integer , _16bit , little_endian >   u16le;
    typedef format_tag< unsigned_integer , _16bit , native_endian >   u16ne;

    typedef format_tag< signed_integer   , _24bit , big_endian    >   s24be;
    typedef format_tag< signed_integer   , _24bit , little_endian >   s24le;
    typedef format_tag< signed_integer   , _24bit , native_endian >   s24ne;

    typedef format_tag< unsigned_integer , _24bit , big_endian    >   u24be;
    typedef format_tag< unsigned_integer , _24bit , little_endian >   u24le;
    typedef format_tag< unsigned_integer , _24bit , native_endian >   u24ne;

    typedef format_tag< signed_integer   , _32bit , big_endian    >   s32be;
    typedef format_tag< signed_integer   , _32bit , little_endian >   s32le;
    typedef format_tag< signed_integer   , _32bit , native_endian >   s32ne;

    typedef format_tag< unsigned_integer , _32bit , big_endian    >   u32be;
    typedef format_tag< unsigned_integer , _32bit , little_endian >   u32le;
    typedef format_tag< unsigned_integer , _32bit , native_endian >   u32ne;

    typedef format_tag< floating_point   , _32bit , big_endian    >   f32be;
    typedef format_tag< floating_point   , _32bit , little_endian >   f32le;
    typedef format_tag< floating_point   , _32bit , native_endian >   f32ne;

    typedef format_tag< floating_point   , _64bit , big_endian    >   f64be;
    typedef format_tag< floating_point   , _64bit , little_endian >   f64le;
    typedef format_tag< floating_point   , _64bit , native_endian >   f64ne;

    format()
      : m_id( -1 )
    {
    }

    template < class tag >
    format( const tag & )
      : m_id        ( format_id<tag>::value )
      , m_number    ( tag::number_t::value )
      , m_bitwidth  ( tag::bitwidth_t::value )
      , m_endian    ( tag::endian_t::value )
    {

    }

    bool operator == ( const format& other ) const
    {
      return ( m_id == other.m_id );
    }
    
    bool operator != ( const format& other ) const
    {
      return ( m_id != other.m_id );
    }

    int id() const
    {
      return m_id;
    }

    bool valid() const
    {
      return m_id != -1;
    }

    bitwidth getBitwidth() const
    {
      return m_bitwidth;
    }

    number getNumber() const
    {
      return m_number;
    }

    endian getEndian() const
    {
      return m_endian;
    }

    bool isBigEndian()        const { return m_endian == big_endian; }
    bool isLittleEndian()     const { return m_endian == little_endian; }
    bool isNativeEndian()     const { return m_endian == native_endian; }

    bool isSignedInteger()    const { return m_number == signed_integer; }
    bool isUnsignedInteger()  const { return m_number == unsigned_integer; }
    bool isFloatingPoint()    const { return m_number == floating_point; }

  private:

    int       m_id;
    number    m_number;
    endian    m_endian;
    bitwidth  m_bitwidth;
  };

} // namespace pcm
