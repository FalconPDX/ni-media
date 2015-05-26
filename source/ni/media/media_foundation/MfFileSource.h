//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \file MfFileSource
//!
//! \brief Read audio files using the Windows Media Foundation API.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include <boost/icl/right_open_interval.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/iostreams/stream.hpp>

#include <ni/media/AudioStreamInfo.h>


struct IMFSourceReader;

class MfFileSource
{
  struct MfInitializer;
  struct MfBlock;
  class  MfBuffer;

public:
//----------------------------------------------------------------------------------------------------------------------

  using offset_type   = boost::iostreams::stream_offset;

  using FrameRange    = boost::icl::right_open_interval<offset_type>;
  using FrameRangeSet = boost::icl::interval_set<FrameRange::domain_type, std::less, FrameRange>;

  template <typename MfType> using MfTypePtr = std::unique_ptr<MfType, std::function<void(MfType*)>>;

//----------------------------------------------------------------------------------------------------------------------

  static const offset_type defaultOffset = 0;
  static const offset_type aacOfffset    = 2112;

//----------------------------------------------------------------------------------------------------------------------

  MfFileSource(const std::string& path, offset_type readOffset = defaultOffset);
 ~MfFileSource();

  const AudioStreamInfo& audioStreamInfo() const { return m_streamInfo; }

  std::streampos  seek(offset_type, BOOST_IOS::seekdir);
  std::streamsize read(char*, std::streamsize);

private:
  static const offset_type s_defaultBlockSize = 1024;

  std::unique_ptr<MfInitializer> m_initializer;
  std::unique_ptr<MfBuffer>      m_buffer;
  MfTypePtr<IMFSourceReader>     m_reader;

  AudioStreamInfo                m_streamInfo;

  offset_type                    m_readOffset     = 0;
  offset_type                    m_nominalPos     = 0;
  offset_type                    m_adjustedPos    = 0;

  bool                           m_fadein         = false;

  bool seekInternal(offset_type);

  auto readFromBuffer(const FrameRange&, char*) -> FrameRangeSet;
  auto readFromFile  (const FrameRange&, char*) -> FrameRangeSet;

  auto searchAndRetrieveBlock(offset_type)                   -> std::unique_ptr<MfBlock>;
  auto searchForwardAndRetrieveBlock(offset_type)            -> std::unique_ptr<MfBlock>;
  auto searchBackwardAndRetrieveBlock(offset_type, offset_type) -> std::unique_ptr<MfBlock>;

  auto consumeBlock() -> std::unique_ptr<MfBlock>;
  bool updateBuffer(std::unique_ptr<MfBlock>);
  bool discardFadeinBlock();
};
