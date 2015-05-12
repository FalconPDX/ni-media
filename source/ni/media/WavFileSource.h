//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Sep/2013
//!
//! \class WavFileSource
//!
//! \brief Implements Wav File Source Device.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------

#pragma once

#include "AudioFileSource.h"

class WavFileSource : public AudioFileSource
{
public:

  WavFileSource( const std::string& path );

  void  open( const std::string& path );
  using AudioFileSource::close;

  std::streamsize   read( char_type* s, std::streamsize n );
  std::streampos    seek( boost::iostreams::stream_offset off, BOOST_IOS::seekdir way );

private:

  bool  readHeader();

  boost::iostreams::stream_offset   m_offs;
  unsigned                          m_nBlockAlign;
};

//#include "WavFileSource.cpp"
