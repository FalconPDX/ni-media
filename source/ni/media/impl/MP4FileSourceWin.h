//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Andrea Savio
//!
//! \date Mar/2015
//!
//! \file MP4FileSourceWin
//!
//! \brief Win implementation to read MP4 audio files.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!---------------------------------------------------------------------------------------------------------------------
#pragma once

#include "MP4FileSource.h"

#include <boost/icl/right_open_interval.hpp>
#include <boost/icl/interval_set.hpp>


struct IMFSourceReader;

class MP4FileSource::Impl
{
  struct MfInitializer;
  struct MfBlock;
  class  MfBuffer;

public:

//----------------------------------------------------------------------------------------------------------------------

  using offset_t      = boost::iostreams::stream_offset;

  using FrameRange    = boost::icl::right_open_interval<offset_t>;
  using FrameRangeSet = boost::icl::interval_set<FrameRange::domain_type, std::less, FrameRange>;

  template <typename MfType> using MfTypePtr = std::unique_ptr<MfType, std::function<void(MfType*)>>;

//----------------------------------------------------------------------------------------------------------------------

  Impl(const std::string& path);

  const AudioStreamInfo& audioStreamInfo() const { return m_streamInfo; }

  std::streampos  seek(offset_t, BOOST_IOS::seekdir);
  std::streamsize read(char*, std::streamsize);

private:

  static const offset_t s_defaultBlockSize = 1024;
  static const offset_t s_aacReadOffset    = 2112;

  std::unique_ptr<MfInitializer> m_initializer;
  std::unique_ptr<MfBuffer>      m_buffer;
  MfTypePtr<IMFSourceReader>     m_reader;

  AudioStreamInfo                m_streamInfo;

  offset_t                       m_nominalPos     = 0;
  offset_t                       m_adjustedPos    = s_aacReadOffset;

  bool                           m_fadein         = false;

  bool seekInternal(offset_t);

  auto readFromBuffer(const FrameRange&, char*) -> FrameRangeSet;
  auto readFromFile  (const FrameRange&, char*) -> FrameRangeSet;

  auto searchAndRetrieveBlock(offset_t)                   -> std::unique_ptr<MfBlock>;
  auto searchForwardAndRetrieveBlock(offset_t)            -> std::unique_ptr<MfBlock>;
  auto searchBackwardAndRetrieveBlock(offset_t, offset_t) -> std::unique_ptr<MfBlock>;

  auto consumeBlock() -> std::unique_ptr<MfBlock>;
  bool updateBuffer(std::unique_ptr<MfBlock>);
  bool discardFadeinBlock();
};
