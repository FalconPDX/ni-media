//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \class Mp4FileSource
//!
//! \brief Class to read Mp4 audio files.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include <boost/predef.h>

#include <ni/media/AudioFilesource.h>

#if BOOST_OS_MACOS
  class CoreAudioFileSource;
#elif BOOST_OS_WINDOWS
  class MfFileSource;
#endif

class Mp4FileSource : private AudioFileSource
{
public:
  using AudioFileSource::char_type;
  using AudioFileSource::category;
  using AudioFileSource::audioStreamInfo;

  Mp4FileSource(const std::string& path, size_t stream);

  void open(const std::string& path, size_t stream);
  void close();

  bool is_open() const;

  std::streampos  seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir = BOOST_IOS::beg);
  std::streamsize read(char_type*, std::streamsize);

private:
#if BOOST_OS_MACOS
  std::shared_ptr<CoreAudioFileSource> m_impl;
#elif BOOST_OS_WINDOWS
  std::shared_ptr<MfFileSource>        m_impl;
#endif
};
