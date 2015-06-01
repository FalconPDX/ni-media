//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \file CoreAudioFileSource
//!
//! \brief Read audio files using the CoreAudio API.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include <AudioToolbox/ExtendedAudioFile.h>

#include <ni/media/Mp4FileSource.h>


class CoreAudioFileSource
{
  using offset_type = boost::iostreams::stream_offset;

public:
  CoreAudioFileSource(const std::string& path, size_t stream);
 ~CoreAudioFileSource();

  const AudioStreamInfo& audioStreamInfo() const { return m_streamInfo; }

  std::streampos  seek(offset_type, BOOST_IOS::seekdir);
  std::streamsize read(char*, std::streamsize);

private:
  ExtAudioFileRef m_media;
  AudioStreamInfo m_streamInfo;

  offset_type     m_framePos = 0;
};
