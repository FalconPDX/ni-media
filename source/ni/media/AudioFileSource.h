
//!
//! \author Marc Boucek
//! \date Sep/2013
//!
//! \class AudioFileSource
//!
//! \brief Base Class for all Audio Source Devices.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------

#pragma once

#include "AudioStreamInfo.h"

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>  // basic_file

class AudioFileSource : private boost::iostreams::basic_file<char>
{
public:
  typedef char char_type;
  typedef boost::iostreams::basic_file<char_type> base_type;
  struct category
    : boost::iostreams::input_seekable
    , boost::iostreams::device_tag
    , boost::iostreams::closable_tag
  { };

  virtual ~AudioFileSource() {}

  using base_type::putback;
  using base_type::is_open;
  using base_type::seek;
  using base_type::read;

  boost::iostreams::stream_offset tell()
  {
    return boost::iostreams::position_to_offset( seek( 0, std::ios_base::cur ) );
  }

  const AudioStreamInfo & audioStreamInfo() const
  {
    return m_AudioStreamInfo;
  }

protected:
  AudioFileSource()
    : base_type("", ~BOOST_IOS::out, BOOST_IOS::in )
  {}

  AudioFileSource( const std::string& path, BOOST_IOS::openmode mode = BOOST_IOS::in )
  : base_type( path, mode & ~BOOST_IOS::out, BOOST_IOS::in )
  {}

  void audioStreamInfo(const AudioStreamInfo & info)
  {
    m_AudioStreamInfo = info;
  }

  void open( const std::string& path, BOOST_IOS::openmode mode )
  {
    base_type::open(path, mode & ~BOOST_IOS::out, BOOST_IOS::in);
  }

  void close()
  {
    base_type::close();
    m_AudioStreamInfo = AudioStreamInfo();
  }

private:
    AudioStreamInfo   m_AudioStreamInfo;
};
