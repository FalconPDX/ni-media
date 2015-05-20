//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Oct/2013
//!
//! \class converter
//!
//! \brief Helper functions for reading / writing pcm data
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------
#pragma once

#include "traits.h"
#include "format.h"

#include <boost/cstdint.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/minus.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/not.hpp>

#include <boost/mpl/greater.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/assert.hpp>


#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>

#include <iterator>
#include <algorithm>
#include <array>


namespace pcm {
  namespace detail {

    //! For float type to int type conversion.
    //! value is a number that depends on int_t
    //! . . .
    template <class float_t, size_t bits>
    struct promote_float
    {
      typedef typename
        boost::mpl::if_<
        boost::mpl::greater<
          boost::mpl::int_<bits>,
          boost::mpl::int_<23>
        >
        , double
        , float
        >::type type;
    };

    template <size_t bits>
    struct promote_float<double,bits>
    {
        typedef double type;
    };


    template<class T>
    void msb_toggle(T&, const boost::false_type&)
    {
    }

    template<class T>
    void msb_toggle(T& val, const boost::true_type&)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<T> ));
      const T mask = static_cast<T>( 1ull  << ( sizeof(T) * 8 - 1 ) );
      val ^= mask ;
    }

    template<
      typename target_t,
      typename source_t
    >
    target_t sign_cast(source_t val)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));
      BOOST_MPL_ASSERT_RELATION( sizeof(target_t), == ,sizeof(source_t) );

      typedef boost::integral_constant<
        bool,
        boost::is_signed<target_t>::value != boost::is_signed<source_t>::value
      > truth_type;

      msb_toggle(val, truth_type() );
      return static_cast<target_t>(val);
    }

    template<
      typename int_t,
      typename float_t
    >
    int_t round_cast(float_t val)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<int_t> ));
      BOOST_MPL_ASSERT(( boost::is_floating_point<float_t> ));
      return static_cast<int_t>( val > 0 ? val + static_cast<float_t>(0.5) : val - static_cast<float_t>(0.5) );
    }

    template<typename source_t>
    source_t shift_cast(source_t val)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));
      return val;
    }

    template<
      typename target_t,
      typename source_t
    >
    typename boost::enable_if<
      boost::mpl::greater<
        boost::mpl::sizeof_<source_t>,
        boost::mpl::sizeof_<target_t>
      >, target_t
    >::type shift_cast(source_t val)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));
      BOOST_MPL_ASSERT_RELATION( boost::is_signed<target_t>::value, == ,boost::is_signed<source_t>::value);
      BOOST_MPL_ASSERT_RELATION( sizeof(source_t), > ,sizeof(target_t) );

      return static_cast<target_t>( val >> ( 8 * ( sizeof(source_t) - sizeof(target_t) ) ) );
    }

    template<
      typename target_t,
      typename source_t
    >
    typename boost::enable_if<
      boost::mpl::greater<
        boost::mpl::sizeof_<target_t>,
        boost::mpl::sizeof_<source_t>
      >, target_t
    >::type shift_cast(source_t val)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));
      BOOST_MPL_ASSERT_RELATION( boost::is_signed<target_t>::value, == , boost::is_signed<source_t>::value);
      BOOST_MPL_ASSERT_RELATION( sizeof(target_t), >, sizeof(source_t) );

      return static_cast<target_t>( val ) << ( 8 * ( sizeof(target_t) - sizeof(source_t) ) );
    }

    // floating point -> floating point
    // simple cast
    template<
      typename target_t,
      typename source_t
    >
    typename boost::enable_if<
      boost::mpl::and_<
        boost::is_floating_point<target_t>,
        boost::is_floating_point<source_t>
      >, target_t
    >::type convert_to(source_t src)
    {
      BOOST_MPL_ASSERT(( boost::is_floating_point<target_t> ));
      BOOST_MPL_ASSERT(( boost::is_floating_point<source_t> ));

      return static_cast<target_t>(src);
    }

    // floating point -> fixed point
    // clip, upscale, round_cast, sign_cast
    template<
      typename target_t,
      typename source_t
    >
    typename boost::enable_if<
      boost::mpl::and_<
        boost::is_integral<target_t>,
        boost::is_floating_point<source_t>
      >, target_t
    >::type convert_to(source_t src)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<target_t> ));
      BOOST_MPL_ASSERT(( boost::is_floating_point<source_t> ));

      typedef typename boost::make_signed<target_t>::type                  signed_target_t;
      typedef typename promote_float<source_t, sizeof(target_t)* 8>::type  float_t;

      const float_t scaleValue = static_cast<float_t>( 1ull << ( sizeof( target_t ) * 8 - 1 ) );
      const float_t minValue = static_cast<float_t>( -1.0 );
      const float_t maxValue = ( scaleValue - static_cast<float_t>( 1.0 ) ) / scaleValue;

      float_t srcValue = std::max( minValue, std::min( maxValue, static_cast<float_t>( src ) ) );

      return sign_cast<target_t>( round_cast<signed_target_t>( srcValue * scaleValue ) );
    }

    // fixed point -> floating point
    // sign_cast + downscale
    template<
      typename target_t,
      typename source_t
    >
    typename boost::enable_if<
      boost::mpl::and_<
        boost::is_floating_point<target_t>,
        boost::is_integral<source_t>
      >, target_t
    >::type convert_to(source_t src)
    {
      BOOST_MPL_ASSERT(( boost::is_floating_point<target_t> ));
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));

      typedef typename boost::make_signed<source_t>::type                  signed_source_t;
      typedef typename promote_float<target_t, sizeof(source_t)* 8>::type  float_t;

      const signed_source_t srcValue = sign_cast<signed_source_t>(src);
      const float_t scaleValue = static_cast<float_t>( 1.0 ) / ( 1ull << ( sizeof( source_t ) * 8 - 1 ) );

      return static_cast<target_t>( scaleValue * static_cast<float_t>( srcValue ) );
    }

    // fixed point -> fixed point
    // sign_cast, shift_cast
    template<
      typename target_t,
      typename source_t
    >
    typename boost::enable_if<
      boost::mpl::and_<
        boost::is_integral<target_t>,
        boost::is_integral<source_t>
      >, target_t
    >::type convert_to(source_t src)
    {
      BOOST_MPL_ASSERT(( boost::is_integral<target_t> ));
      BOOST_MPL_ASSERT(( boost::is_integral<source_t> ));

      typedef typename
        boost::mpl::if_<
          boost::is_signed<target_t>,
          typename boost::make_signed<source_t>::type,
          typename boost::make_unsigned<source_t>::type
        >::type sign_adjusted_source_t;

      return shift_cast<target_t>( sign_cast<sign_adjusted_source_t>(src) );
    }


    template <
      ::pcm::number n,
      ::pcm::bitwidth b>
    struct make_intermediate
    {
    };

    template <> struct
      make_intermediate< ::pcm::floating_point   , ::pcm::_32bit > { typedef float type; };

    template <> struct
      make_intermediate< ::pcm::floating_point   , ::pcm::_64bit > { typedef double type; };

    template <> struct
      make_intermediate< ::pcm::signed_integer   , ::pcm::_8bit > { typedef boost::int8_t type; };

    template <> struct
      make_intermediate< ::pcm::signed_integer   , ::pcm::_16bit > { typedef boost::int16_t type; };

    template <> struct
      make_intermediate< ::pcm::signed_integer   , ::pcm::_24bit > { typedef boost::int32_t type; };

    template <> struct
      make_intermediate< ::pcm::signed_integer   , ::pcm::_32bit > { typedef boost::int32_t type; };

    template <> struct
      make_intermediate< ::pcm::unsigned_integer , ::pcm::_8bit > { typedef boost::uint8_t type; };

    template <> struct
      make_intermediate< ::pcm::unsigned_integer , ::pcm::_16bit > { typedef boost::uint16_t type; };

    template <> struct
      make_intermediate< ::pcm::unsigned_integer , ::pcm::_24bit > { typedef boost::uint32_t type; };

    template <> struct
      make_intermediate< ::pcm::unsigned_integer , ::pcm::_32bit > { typedef boost::uint32_t type; };


    template < class format_t >
    struct io_helper
    {
      typedef typename make_intermediate<format_t::number_t::value,format_t::bitwidth_t::value>::type   intermediate_t;

      typedef boost::mpl::size_t<sizeof(intermediate_t)>                 intermediate_size_t;
      typedef boost::mpl::size_t<format_t::bitwidth_t::value / 8>        step_size_t;

      BOOST_MPL_ASSERT_RELATION( intermediate_size_t::value, >=, step_size_t::value );

      BOOST_MPL_ASSERT(( boost::is_same<typename step_size_t::value_type, typename intermediate_size_t::value_type>  ));
      BOOST_MPL_ASSERT(( boost::is_same<typename step_size_t::value_type, size_t > ));
      BOOST_MPL_ASSERT(( boost::is_same<typename intermediate_size_t::value_type, size_t > ));

      typedef typename
        boost::mpl::if_c<
        ::pcm::native_endian == ::pcm::little_endian,
        intermediate_size_t,
        step_size_t
        >::type end_t;

      typedef typename
        boost::mpl::minus<
        end_t,
        step_size_t
        >::type begin_t;

      typedef union
      {
        intermediate_t  value;
        char byte[intermediate_size_t::value];
      } union_t;

    };
  } // namespace detail

  template<
    class format_t,
    class src_iter_t,
    class dest_value_t
  >
  dest_value_t read_value( src_iter_t iter )
    {
      typedef ::pcm::detail::io_helper<format_t> io;
      typename io::union_t tmp = { 0 };

      if ( is_native_endian<format_t>::value )
      {
        for ( size_t k = io::begin_t::value; k < io::end_t::value; ++k, ++iter )
        {
          tmp.byte[k] = *iter;
        }
      }
      else
      {
        for ( size_t k = io::end_t::value; k > io::begin_t::value; ++iter )
        {
          tmp.byte[--k] = *iter;
        }
      }
      return ::pcm::detail::convert_to<dest_value_t>( tmp.value );
    }


  template<
    class format_t,
    class dest_iter_t,
    class src_value_t
  >
  void write_value( src_value_t val, dest_iter_t iter )
    {
      typedef ::pcm::detail::io_helper<format_t> io;
      typename io::union_t tmp = { ::pcm::detail::convert_to<typename io::intermediate_t>( val ) };

      if ( is_native_endian<format_t>::value )
      {
        for ( size_t k = io::begin_t::value; k < io::end_t::value; ++k, ++iter )
        {
          *iter = tmp.byte[k];
        }
      }
      else
      {
        for ( size_t k = io::end_t::value; k > io::begin_t::value; ++iter )
        {
          *iter = tmp.byte[--k];
        }
      }
    }

  template<
    class format_t,
    class src_iter_t,
    class dest_iter_t
  >
  dest_iter_t read( src_iter_t src, src_iter_t end, dest_iter_t dest )
    {
      typedef typename boost::add_reference< src_iter_t >::type       src_iter_ref_t;
      typedef typename std::iterator_traits<dest_iter_t>::value_type  dest_value_t;

      for( ; src!=end; ++dest )
      {
        *dest = read_value<format_t,src_iter_ref_t,dest_value_t>( src );
      }
      return dest;
    }

  template<
    class format_t,
    class src_iter_t,
    class dest_iter_t
  >
  dest_iter_t write( src_iter_t src, src_iter_t end, dest_iter_t dest )
    {
      typedef typename boost::add_reference< dest_iter_t >::type       dest_iter_ref_t;
      typedef typename std::iterator_traits<src_iter_t>::value_type    src_value_t;

      for( ; src!=end; ++src )
      {
        write_value<format_t,dest_iter_ref_t,src_value_t>( *src, dest );
      }
      return dest;
    }

    template<
      class value_t,
      class iterator_t
    > using reader_signature = value_t (*) ( iterator_t ) ;

    template<
      class value_t,
      class iterator_t
    >
    using writer_signature = void (*) ( value_t, iterator_t ) ;

    template<
      typename value_t,
      typename iterator_t,
      typename Tags >
    struct dispatcher;

    template<
      typename value_t,
      typename iterator_t,
      typename... Tags >
    struct dispatcher< value_t, iterator_t, pcm::detail::types<Tags...> >
    {
      static const std::array< reader_signature<value_t,iterator_t> , sizeof...(Tags) >  readers;
      static const std::array< writer_signature<value_t,iterator_t> , sizeof...(Tags) >  writers;
    };

   template<
    class value_t,
    class iterator_t,
    class... Tags >
   const std::array< reader_signature<value_t,iterator_t>, sizeof...(Tags) >
   dispatcher< value_t, iterator_t, ::pcm::detail::types<Tags...> >::readers = { &read_value<Tags,iterator_t,value_t>... };

   template<
    class value_t,
    class iterator_t,
    class... Tags >
   const std::array< writer_signature<value_t,iterator_t>, sizeof...(Tags) >
   dispatcher< value_t, iterator_t, ::pcm::detail::types<Tags...> >::writers = { &write_value<Tags,iterator_t,value_t>... };


  template<
    class value_t,
    class iterator_t,
    class format_t
  >
  struct converter
  {
    value_t read( iterator_t iter ) const
    {
      return read_value<format_t,iterator_t,value_t>( iter );
    }

    void write( value_t val, iterator_t iter ) const
    {
      write_value<format_t,iterator_t,value_t>( val, iter );
    }

  };

  template<
    class value_t,
    class iterator_t
  >
  struct converter<value_t,iterator_t,::pcm::format>
  {
    using reader      = reader_signature<value_t, iterator_t>;
    using writer      = writer_signature<value_t, iterator_t>;
    using dispatcher  = dispatcher<value_t, iterator_t, format_tags>;

    converter() = default;

    converter( const ::pcm::format& fmt )
      : m_reader( dispatcher::readers[fmt.id()] )
      , m_writer( dispatcher::writers[fmt.id()] )
    {
    }

    value_t read( iterator_t iter ) const
    {
      return m_reader( iter );
    }

    void write( value_t val, iterator_t iter ) const
    {
      m_writer( val, iter );
    }

  private:
    reader m_reader = nullptr;
    writer m_writer = nullptr;
  };


} // namespace pcm

