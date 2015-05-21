//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Mai/2015
//! \file converted
//!
//! \brief A pcm range adaptor to convert between pcm formats
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------

#pragma once

#include <ni/media/pcm/iterator.h>
#include <boost/range/iterator_range.hpp>
#include <boost/range/iterator.hpp>


namespace pcm{
namespace converted_detail{

  template< class Rng, class Value>
  struct converted_range
    : boost::iterator_range
      < ::pcm::iterator
        < Value,
          typename boost::range_iterator<Rng>::type,
          ::pcm::format
        >
      >
  {
  private:
    using pcm_iterator = ::pcm::iterator
      < Value,
        typename boost::range_iterator<Rng>::type,
        ::pcm::format
      > ;

    using base_range = boost::iterator_range < pcm_iterator > ;

  public:
    converted_range( Rng& r, const ::pcm::format&  fmt )
      : base_range( pcm_iterator( r.begin(), fmt ), pcm_iterator( r.end(), fmt ) )
    {
    }
  };

  template< class Value>
  struct converter_holder
  {
    ::pcm::format fmt;
    converter_holder( const ::pcm::format& f ) : fmt( f )
    {
    }
  };

  template< template<class> class Holder >
  struct converter_forwarder
  {
    template< class Value >
    Holder<Value> operator()( const ::pcm::format& from, const Value& /*to*/ ) const
    {
      return Holder<Value>( from );
    }
  };

  template< class Rng, class Value >
  inline converted_range<Rng, Value>
  operator|(Rng& r, const converter_holder<Value>& h)
  {
    return converted_range<Rng, Value >( r, h.fmt );
  }

  template< class Rng, class Value>
  inline converted_range<const Rng, Value>
  operator|(const Rng& r, const converter_holder<Value>& h)
  {
    return converted_range<const Rng, Value>( r, h.fmt );
  }

} // namespace converted_detail

using converted_detail::converted_range;

namespace
{
  const converted_detail::converter_forwarder<converted_detail::converter_holder>
  converted = converted_detail::converter_forwarder<converted_detail::converter_holder>();
}

} // namespace pcm

