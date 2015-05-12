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

#include "AudioFilesource.h"


class MP4FileSource : private AudioFileSource
{
public:
  class Impl;

  using AudioFileSource::char_type;
  using AudioFileSource::category;
  using AudioFileSource::audioStreamInfo;

  MP4FileSource(const std::string& path);

  void open(const std::string& path);
  void close();

  bool is_open() const;

  std::streampos  seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir = BOOST_IOS::beg);
  std::streamsize read(char_type*, std::streamsize);

private:
  std::shared_ptr<Impl> m_impl;
};
