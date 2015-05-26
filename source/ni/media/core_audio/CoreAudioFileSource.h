//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \file MP4FileSourceMac
//!
//! \brief Mac implementation to stream MP4 audio files.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include <AudioToolbox/ExtendedAudioFile.h>

#include <ni/media/MP4FileSource.h>


class MP4FileSource::Impl
{
  using offset_t = boost::iostreams::stream_offset;

public:
  Impl(const std::string& path);
 ~Impl();

  const AudioStreamInfo& audioStreamInfo() const { return m_streamInfo; }

  std::streampos  seek(offset_t, BOOST_IOS::seekdir);
  std::streamsize read(char*, std::streamsize);

private:
  ExtAudioFileRef m_media;
  AudioStreamInfo m_streamInfo;

  offset_t        m_framePos = 0;
};
