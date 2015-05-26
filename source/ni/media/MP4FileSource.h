//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \class MP4FileSource
//!
//! \brief Class to read MP4 audio files.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include <boost/predef.h>

#include "AudioFilesource.h"

class MP4FileSource : private AudioFileSource
{
public:
  class Impl;

  using AudioFileSource::char_type;
  using AudioFileSource::category;
  using AudioFileSource::audioStreamInfo;

#if BOOST_OS_MACOS
  MP4FileSource(const std::string& path);
  void open(const std::string& path);
#elif BOOST_OS_WINDOWS
  using offset_type = boost::iostreams::stream_offset;
  static const offset_type defaultOffset = 0;
  static const offset_type aacOfffset    = 2112;

  MP4FileSource(const std::string& path, offset_type readOffset = defaultOffset);
  void open(const std::string& path, offset_type readOffset);
#endif

  void close();

  bool is_open() const;

  std::streampos  seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir = BOOST_IOS::beg);
  std::streamsize read(char_type*, std::streamsize);

private:
  std::shared_ptr<Impl> m_impl;
};
