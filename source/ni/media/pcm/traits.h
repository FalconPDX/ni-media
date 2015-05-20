//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Oct/2013
//!
//! \class traits
//!
//! \brief Pcm traits definitions.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------

#pragma once

#include <boost/detail/endian.hpp> // BOOST_LITTLE_ENDIAN
#include <boost/type_traits/integral_constant.hpp>

namespace pcm {

  namespace detail
  {
    template < typename... Types >
    struct types {};

    template< size_t n, typename SearchType, typename... Types >
    struct index_of_impl
    {
      static_assert(sizeof...(Types) > 0, "Type not found");
    };

    template< size_t n, typename SearchType, typename Head, typename... Tail >
    struct index_of_impl<n, SearchType, Head, Tail... >
    {
      static const size_t value = index_of_impl<n + 1, SearchType, Tail...>::value;
    };

    template< size_t n, typename SearchType, typename... Tail >
    struct index_of_impl<n, SearchType, SearchType, Tail... >
    {
      static const size_t value = n;
    };

    /*
    template< size_t n , typename SearchType >
    struct index_of_impl<n,SearchType>
    {
    };
    */
    // General definition
    template <typename Type, typename ... Types>
    struct index_of
    {
      static const size_t value = index_of_impl<0, Type, Types...>::value;
    };

    template <typename Type, typename ... Types>
    struct index_of< Type, types<Types...> >
    {
      static const size_t value = index_of<Type, Types...>::value;
    };

  } // namespace detail

  enum number
  {
    signed_integer = 0,
    unsigned_integer,
    floating_point
  };

  enum endian
  {
    big_endian = 0,
    little_endian,
#if defined(BOOST_BIG_ENDIAN)
    native_endian = big_endian,
#elif defined(BOOST_LITTLE_ENDIAN)
    native_endian = little_endian,
#else
#error "unable to determine system endianness."
#endif
  };

  enum bitwidth
  {
    _8bit = 8,
    _16bit = 16,
    _24bit = 24,
    _32bit = 32,
    _64bit = 64
  };

    template<
      number n,
      bitwidth b,
      endian e>
    struct format_tag
    {
      typedef boost::integral_constant<number,n>    number_t;
      typedef boost::integral_constant<bitwidth,b>  bitwidth_t;
      typedef boost::integral_constant<endian,e>    endian_t;
    };


  using format_tags = ::pcm::detail::types <
    format_tag< signed_integer   , _8bit  , native_endian > ,
    format_tag< unsigned_integer , _8bit  , native_endian > ,
    format_tag< signed_integer   , _16bit , big_endian    > ,
    format_tag< signed_integer   , _16bit , little_endian > ,
    format_tag< unsigned_integer , _16bit , big_endian    > ,
    format_tag< unsigned_integer , _16bit , little_endian > ,
    format_tag< signed_integer   , _24bit , big_endian    > ,
    format_tag< signed_integer   , _24bit , little_endian > ,
    format_tag< unsigned_integer , _24bit , big_endian    > ,
    format_tag< unsigned_integer , _24bit , little_endian > ,
    format_tag< signed_integer   , _32bit , big_endian    > ,
    format_tag< signed_integer   , _32bit , little_endian > ,
    format_tag< unsigned_integer , _32bit , big_endian    > ,
    format_tag< unsigned_integer , _32bit , little_endian > ,
    format_tag< floating_point   , _32bit , big_endian    > ,
    format_tag< floating_point   , _32bit , little_endian > ,
    format_tag< floating_point   , _64bit , big_endian    > ,
    format_tag< floating_point   , _64bit , little_endian > >;

  template<class tag>
  struct format_id            : public boost::integral_constant< int, detail::index_of<tag, format_tags>::value >{};

  template<class tag>
  struct is_signed_integer    : public boost::integral_constant< bool, tag::number_t::value == signed_integer >{};

  template<class tag>
  struct is_unsigned_integer  : public boost::integral_constant< bool, tag::number_t::value == unsigned_integer >{};

  template<class tag>
  struct is_floating_point    : public boost::integral_constant< bool, tag::number_t::value == floating_point >{};

  template<class tag>
  struct is_big_endian        : public boost::integral_constant< bool, tag::endian_t::value == big_endian >{};

  template<class tag>
  struct is_little_endian     : public boost::integral_constant< bool, tag::endian_t::value == little_endian >{};

  template<class tag>
  struct is_native_endian     : public boost::integral_constant< bool, tag::endian_t::value == native_endian >{};


  struct machine_is_little_endian  : public boost::integral_constant< bool, little_endian == native_endian >{};

  struct machine_is_big_endian     : public boost::integral_constant< bool, big_endian == native_endian >{};

} // namesace pcm
