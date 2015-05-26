//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \class WmaFileSource
//!
//! \brief Class to read wma audio files. Works only on Windows.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include <boost/predef.h>

#include <ni/media/AudioFilesource.h>

class MfFileSource;

class WmaFileSource : private AudioFileSource
{
  using AudioFileSource::char_type;
  using AudioFileSource::category;
  using AudioFileSource::audioStreamInfo;

public:
  WmaFileSource(const std::string& path);

  void open(const std::string& path);
  void close();

  bool is_open() const;

  std::streampos  seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir = BOOST_IOS::beg);
  std::streamsize read(char_type*, std::streamsize);

private:
  std::shared_ptr<MfFileSource> m_impl;
};
