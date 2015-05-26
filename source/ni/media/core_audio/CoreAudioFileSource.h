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


class CoreAudioFileSource
{
  using offset_type = boost::iostreams::stream_offset;

public:
  CoreAudioFileSource(const std::string& path);
 ~CoreAudioFileSource();

  const AudioStreamInfo& audioStreamInfo() const { return m_streamInfo; }

  std::streampos  seek(offset_type, BOOST_IOS::seekdir);
  std::streamsize read(char*, std::streamsize);

private:
  ExtAudioFileRef m_media;
  AudioStreamInfo m_streamInfo;

  offset_type     m_framePos = 0;
};
