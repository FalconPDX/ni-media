//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Oct/2013
//!
//! \class PcmIterator
//!
//! \brief Iterator for reading / writing pcm data
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------
#pragma once

#include "PcmTraits.h"
#include "PcmConverter.h"

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

    iterator_proxy( const iterator_proxy& other )
      : m_converter( other.m_converter )
      , m_iter( other.m_iter )
    {
    }

    iterator_proxy& operator = ( value_t val )
    {
      m_converter.write( val, m_iter );
      return *this;
    }

    operator value_t () const
    {
      return m_converter.read( m_iter );
    }

  private:

    friend class iterator<value_t,iterator_t,format_t>;


    iterator_proxy(); // delete

    iterator_proxy( const converter_t& converter, const iterator_t& iter )
      : m_converter( converter )
      , m_iter( iter )
    {
    }

    converter_t     m_converter;
    iterator_t      m_iter;
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

    iterator( const iterator& other )
      : m_proxy( other.m_proxy )
      , m_step( other.m_step )
    {
    }

    iterator( iterator_t iter, const ::pcm::format & fmt )
      : m_proxy( converter_t(fmt), iter )
      , m_step( fmt.getBitwidth() / 8 )
    {
    }

  private:

    void increment()
    {
      std::advance( m_proxy.m_iter, m_step );
    }

    void decrement()
    {
      std::advance( m_proxy.m_iter, -m_step );
    }

    void advance( difference_type offset )
    {
      std::advance( m_proxy.m_iter, offset * m_step );
    }

    difference_type distance_to( const iterator& other ) const
    {
      return std::distance( m_proxy.m_iter, other.m_proxy.m_iter ) / m_step;
    }

    bool equal( const iterator& other ) const
    {
      return m_proxy.m_iter == other.m_proxy.m_iter;
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



