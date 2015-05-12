//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//! \date Apr/2015
//!
//! \class AiffFileSource
//!
//! \brief Class to read Aiff audio files.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------

#pragma once

#include "AudioFileSource.h"


class AiffFileSource : public AudioFileSource
{
public:
  AiffFileSource(const std::string& path);

  void  open(const std::string& path);
  using AudioFileSource::close;

  std::streampos  seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir);
  std::streamsize read(char_type*, std::streamsize);

private:
  void readHeader();

  boost::iostreams::stream_offset m_offset = 0;
};
