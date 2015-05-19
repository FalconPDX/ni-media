//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Apr/2015
//!
//! \class CAFFileSource
//!
//! \brief Alac decoding / streaming from raw (CAF) file
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------

#pragma once

#include "AudioFileSource.h"

#include <ALACAudioTypes.h>

#include <vector>
#include <memory>


class CAFFileSource : public AudioFileSource
{
public:

  CAFFileSource( const std::string& path );

  void  open(const std::string& path);
  using AudioFileSource::close;

  std::streamsize read( char_type* s, std::streamsize n );
  std::streampos  seek( boost::iostreams::stream_offset off, BOOST_IOS::seekdir way );

private:

  void   readHeader();
  size_t decodeBlock();

  using buffer_t = std::vector < uint8_t > ;

  buffer_t                          m_readBuffer;
  buffer_t                          m_writeBuffer;
  size_t                            m_writeOffset;

  AudioFormatDescription            m_inputFormat;
  AudioFormatDescription            m_outputFormat;

  std::streampos                    m_pos = {0};

  int32_t                           m_packetTablePos = {0};
  int32_t                           m_packetTableSize = {0};
  int32_t                           m_inputDataPos = {0};
  int32_t                           m_inputDataSize = {0};

  std::shared_ptr<class ALACDecoder> m_decoder;
};
