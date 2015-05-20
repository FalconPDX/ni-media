//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Oct/2013
//!
//! \class iterator
//!
//! \brief Iterator for reading / writing pcm data
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------
#pragma once

#include "traits.h"
#include "converter.h"

#include <boost/iterator/iterator_facade.hpp>

#include <iterator>     // std::iterator_traits

namespace pcm
{

  template<
    class value_t,
    class iterator_t,
    class format_t
  >
  class iterator;

  template<
    class value_t,
    class iterator_t,
    class format_t
  >
  struct iterator_proxy
  {
    typedef typename::pcm::converter<value_t,iterator_t, format_t> converter_t;

    iterator_proxy() = default;
    iterator_proxy( const iterator_proxy& other ) = default;

    iterator_proxy& operator = ( value_t val )
    {
      converter.write( val, iter );
      return *this;
    }

    operator value_t () const
    {
      return converter.read( iter );
    }

  private:

    friend class iterator<value_t,iterator_t,format_t>;

    iterator_proxy( const converter_t& c, const iterator_t& i ) : converter( c ) , iter( i ){}

    converter_t     converter;
    iterator_t      iter;
  };


  template<
    typename value_t,
    typename iterator_t,
    typename format_t = ::pcm::format
  >
  class iterator
    : public boost::iterator_facade<
    iterator<value_t,iterator_t,format_t>,
    value_t,
    typename std::iterator_traits<iterator_t>::iterator_category,
    iterator_proxy<value_t,iterator_t,format_t>
    >
  {
    typedef typename
      boost::iterator_facade<
        iterator<value_t,iterator_t,format_t>,
        value_t,
        typename boost::iterator_traversal<iterator_t>::type,
        iterator_proxy<value_t,iterator_t,format_t>
      >
      base_t;

    typedef iterator<value_t,iterator_t,format_t>                       this_t;
    typedef iterator_proxy<value_t,iterator_t,format_t>                 proxy_t;
    typedef typename proxy_t::converter_t                               converter_t;
    typedef typename std::iterator_traits<iterator_t>::difference_type  step_t;


  public:

    typedef typename base_t::value_type         value_type;
    typedef typename base_t::difference_type    difference_type;
    typedef typename base_t::reference          reference;

    iterator() = default;
    iterator( const iterator& other ) = default;

    iterator( iterator_t iter, const ::pcm::format & fmt )
      : m_proxy(converter_t( fmt ), iter)
      , m_step( fmt.getBitwidth() / 8 )
    {
    }

  private:

    void increment()
    {
      std::advance( m_proxy.iter, m_step );
    }

    void decrement()
    {
      std::advance( m_proxy.iter, -m_step );
    }

    void advance( difference_type offset )
    {
      std::advance( m_proxy.iter, offset * m_step );
    }

    difference_type distance_to( const iterator& other ) const
    {
      return std::distance( m_proxy.iter, other.m_proxy.iter ) / m_step;
    }

    bool equal( const iterator& other ) const
    {
      return m_proxy.iter == other.m_proxy.iter;
    }

    reference dereference() const
    {
      return m_proxy;
    }

    friend class ::boost::iterator_core_access;

    proxy_t m_proxy;
    step_t  m_step;
  };

} // namespace pcm



