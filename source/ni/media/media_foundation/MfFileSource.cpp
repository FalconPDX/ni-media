#include "MfFileSource.h"

#include <ciso646>
#include <codecvt>
#include <functional>
#include <locale>

#include <boost/make_unique.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/format.hpp>

#ifdef WINVER
# undef WINVER
#endif

#define WINVER _WIN32_WINNT_WIN7

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <windows.h>

#include <Propvarutil.h>
#include <Wmcodecdsp.h>


namespace
{

//----------------------------------------------------------------------------------------------------------------------
// RAII wrappers for WMF interfaces
//----------------------------------------------------------------------------------------------------------------------

template <typename MfType> using MfTypePtr = MfFileSource::MfTypePtr<MfType>;

template< class C >                    struct get_MF_type_impl : get_MF_type_impl< decltype(&C::operator()) > {};
template< class C, typename MFType >   struct get_MF_type_impl< HRESULT (C::*)(MFType**) > : get_MF_type_impl< HRESULT(MFType**) > {};
template< class C, typename MFType >   struct get_MF_type_impl< HRESULT (C::*)(MFType**) const > : get_MF_type_impl< HRESULT(MFType**) > {};
template< typename MFType >            struct get_MF_type_impl< HRESULT(MFType**) > { using type = MFType; };

template< typename F >
using get_MF_type = typename get_MF_type_impl< typename std::remove_reference<F>::type >::type;

template <typename MfAllocator>
auto allocateNoThrow(MfAllocator&& allocator) -> MfTypePtr<get_MF_type<MfAllocator>>
{
  using MfType = get_MF_type<MfAllocator>;

  MfType* ptr  = nullptr;
  auto status  = allocator(&ptr);

  MfTypePtr<MfType> ret(ptr, [] (MfType* ptr) { if (ptr) ptr->Release(); });
  return SUCCEEDED(status) ? std::move(ret) : nullptr;
}

template <typename MfAllocator>
auto allocateOrThrow(MfAllocator&& allocator, std::string errorMsg) -> MfTypePtr<get_MF_type<MfAllocator>>
{
  auto ptr = allocateNoThrow(std::forward<MfAllocator>(allocator));

  if (!ptr) throw std::runtime_error(std::move(errorMsg));
  return ptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool setPosition(IMFSourceReader& reader, LONGLONG time100ns)
{
  PROPVARIANT var;
  return SUCCEEDED(InitPropVariantFromInt64(time100ns, &var))
    && SUCCEEDED(reader.SetCurrentPosition(GUID_NULL, var))
    && SUCCEEDED(PropVariantClear(&var));
}

MfFileSource::offset_type time100nsToFrames(LONGLONG time100ns, size_t sampleRate)
{
  static const LONGLONG secTo100ns = 10000000;

  // We need to be accurate to a single frame when doing this conversion, and the timestamp provided by WMF can be
  // off by a few nanoseconds, so we round it off.

  using return_type = MfFileSource::offset_type;
  return return_type(std::round(double(time100ns) * sampleRate / secTo100ns));
}

LONGLONG framesTo100ns(MfFileSource::offset_type frame, size_t sampleRate)
{
  static const LONGLONG secTo100ns = 10000000;
  return frame * secTo100ns / sampleRate;
}

} // namespace anonymous

struct MfFileSource::MfInitializer
{
 MfInitializer()
 {
   if  (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))
     || FAILED(MFStartup(MF_VERSION)))
   {
     throw std::runtime_error("Failed to initialize WMF.");
   }
 }

~MfInitializer() { if (SUCCEEDED(MFShutdown())) CoUninitialize(); }
};

struct MfFileSource::MfBlock
{
  FrameRange           range;
  MfTypePtr<IMFSample> data;
};

class MfFileSource::MfBuffer
{
  using Buffer      = MfTypePtr<IMFMediaBuffer>;
  using Sample      = MfTypePtr<IMFSample>;

public:
  MfBuffer(std::unique_ptr<MfBlock> block) :
    m_validRange(block->range)
  {
    m_buffer   = allocateNoThrow([&block] (IMFMediaBuffer** p) { return block->data->ConvertToContiguousBuffer(p); });

    DWORD size = 0;
    m_isLocked = m_buffer && SUCCEEDED(m_buffer->Lock(&m_data, nullptr, &size));
  }

 ~MfBuffer() { if (m_isLocked) m_buffer->Unlock(); }

  bool         isLocked()   const { return m_isLocked;   }
  FrameRange   frameRange() const { return m_validRange; }
  const BYTE*  bufferData() const { return m_data;       }

private:
  Buffer      m_buffer;
  bool        m_isLocked    = false;
  FrameRange  m_validRange  { 0, 0 };
  BYTE*       m_data        = nullptr;
};

//----------------------------------------------------------------------------------------------------------------------

MfFileSource::MfFileSource(const std::string& path, offset_type readOffset) :
  m_initializer(new MfInitializer),
  m_readOffset (readOffset),
  m_adjustedPos(readOffset)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  auto wpath = converter.from_bytes(path);

  m_reader = allocateOrThrow([&wpath] (IMFSourceReader** p) {
    return MFCreateSourceReaderFromURL(wpath.c_str(), nullptr, p); }, "Could not open the audio file.");

  if  (FAILED(m_reader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE))
    || FAILED(m_reader->SetStreamSelection(0, TRUE)))
  {
    throw std::runtime_error("Could not select the audio stream.");
  }

  // Retrieve the native media attributes.

  auto nativeType = allocateOrThrow([this] (IMFMediaType** p) {
    return m_reader->GetNativeMediaType(0, 0, p); }, "Could not get native media type.");

  UINT32 value = 0;
  if (FAILED(nativeType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &value)))
    throw std::runtime_error("Could not read the number of channels.");
  m_streamInfo.numChannels(value);

  if (FAILED(nativeType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &value)))
    throw std::runtime_error("Could not read the sample rate.");
  m_streamInfo.sampleRate(value);

  PROPVARIANT var; LONGLONG length100ns = 0;
  if  (FAILED(m_reader->GetPresentationAttribute(static_cast<DWORD>(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION, &var))
    || FAILED(PropVariantToInt64(var, &length100ns))
    || FAILED(PropVariantClear(&var)))
  {
    throw std::runtime_error("Could not read the track length.");
  }
  auto lengthFrames = time100nsToFrames(length100ns, m_streamInfo.sampleRate());
  m_streamInfo.numSampleFrames(lengthFrames > m_readOffset ? size_t(lengthFrames - m_readOffset) : 0);

  UINT32 bitDepth = 0;
  auto hr = nativeType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitDepth);
  if  (hr == MF_E_ATTRIBUTENOTFOUND) bitDepth = 16;
  else if (FAILED( hr ))             throw std::runtime_error("Could not read the bit depth.");

  pcm::format format;
  switch (bitDepth)
  {
  case 8:  format = pcm::format::s8();    break;
  case 16: format = pcm::format::s16le(); break;
  case 24: format = pcm::format::s24le(); break;
  case 32: format = pcm::format::s32le(); break;
  default:
    throw std::runtime_error(boost::str(boost::format("%u bit depth found. Only 8, 16, 24 and 32-bit are supported.")
      % bitDepth));
  }
  m_streamInfo.format(format);

  // Create a media type specifying uncompressed PCM audio and load it into the source reader. The reader will load
  // in turn the correct decoder.

  auto pcmType = allocateOrThrow([] (IMFMediaType** p) { return MFCreateMediaType(p); },
    "Could not create target media type.");

  UINT32 bytesPerSecond = UINT32(m_streamInfo.bytesPerSampleFrame() * m_streamInfo.sampleRate());

  if  (FAILED(pcmType->SetGUID(MF_MT_MAJOR_TYPE,                   MFMediaType_Audio))
    || FAILED(pcmType->SetGUID(MF_MT_SUBTYPE,                      MFAudioFormat_PCM))
    || FAILED(pcmType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,      UINT32(m_streamInfo.bytesPerSampleFrame())))
    || FAILED(pcmType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSecond))
    || FAILED(pcmType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT,    TRUE))
    || FAILED(m_reader->SetCurrentMediaType(0, nullptr, pcmType.get())))
  {
    throw std::runtime_error("Could not load uncompressed pcm decoder.");
  }

  if (not setPosition(*m_reader, framesTo100ns(m_readOffset, m_streamInfo.sampleRate())))
    throw std::runtime_error("Could not reposition in media buffer.");
}

//----------------------------------------------------------------------------------------------------------------------

MfFileSource::~MfFileSource() {}

//----------------------------------------------------------------------------------------------------------------------

std::streampos MfFileSource::seek(offset_type offset, BOOST_IOS::seekdir way)
{
  auto frameSize         = m_streamInfo.bytesPerSampleFrame();
  offset_type nominalPos = offset / frameSize, endPos = m_streamInfo.numSampleFrames() - 1;

  switch (way)
  {
  case BOOST_IOS::beg: break;
  case BOOST_IOS::end: nominalPos  = endPos - nominalPos; break;
  default:             nominalPos += m_nominalPos;
  }

  nominalPos = boost::algorithm::clamp(nominalPos, 0, endPos);

  // The allowed range for the nominal offset  is [0, lengthInFrames[
  // The allowed range for the adjusted offset is [aacReadOffset, lengthInFrames + aacReadOffset[

  auto adjustedPos = nominalPos + m_readOffset;

  if (m_nominalPos != nominalPos && seekInternal(adjustedPos))
  {
    m_nominalPos  = nominalPos;
    m_adjustedPos = adjustedPos;
  }

  return boost::iostreams::stream_offset_to_streamoff(m_nominalPos * frameSize);
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize MfFileSource::read(char* dst, std::streamsize size)
{
  auto frameSize = m_streamInfo.bytesPerSampleFrame();
  auto endPos    = boost::algorithm::clamp<offset_type>(
    m_nominalPos + size / frameSize, 0, m_streamInfo.numSampleFrames() - 1);
  auto numFrames = endPos - m_nominalPos;

  if (numFrames > 0)
  {
    FrameRange toFill(m_adjustedPos, m_adjustedPos + numFrames);
    auto remainingFirstPass = readFromBuffer(toFill, dst);

    FrameRangeSet remainingSecondPass;
    for (const auto& range : remainingFirstPass)
    {
      auto offset = (range.lower() - toFill.lower()) * m_streamInfo.bytesPerSampleFrame();
      remainingSecondPass += readFromFile(range, dst + offset);
    }

    for (const auto& range : remainingSecondPass)
    {
      auto offset = (range.lower() - toFill.lower()) * frameSize;
      std::fill_n(dst + offset, boost::icl::size(range) * frameSize, char(0));
    }
  }

  m_nominalPos  += numFrames;
  m_adjustedPos += numFrames;
  return numFrames * frameSize;
}

//----------------------------------------------------------------------------------------------------------------------

bool MfFileSource::seekInternal(offset_type adjustedPos)
{
  // WMF applies a fade-in to the audio data fetched after a seek, which produces crackling. To avoid this, when
  // doing a seek we jump one block back of the target, then discard it in the following fetch.

  static const auto minValue = s_defaultBlockSize + m_readOffset;
  bool removeFadein = adjustedPos >= minValue;

  adjustedPos = removeFadein ? adjustedPos - s_defaultBlockSize : m_readOffset;
  if (not setPosition(*m_reader, framesTo100ns(adjustedPos, m_streamInfo.sampleRate()))) return false;

  m_fadein = removeFadein;
  return true;
}

//----------------------------------------------------------------------------------------------------------------------

auto MfFileSource::readFromBuffer(const FrameRange& range, char* dst) -> FrameRangeSet
{
  FrameRangeSet remaining; remaining += range;

  if (not m_buffer) return remaining;

  auto bufferedRange = m_buffer->frameRange();
  auto intersection  = range & bufferedRange;

  using namespace boost::icl;

  if (not is_empty(intersection))
  {
    auto src      = m_buffer->bufferData()
      + (intersection.lower() - bufferedRange.lower()) * m_streamInfo.bytesPerSampleFrame();
    auto dstBytes = reinterpret_cast<BYTE*>(dst)
      + (intersection.lower() - range.lower()) * m_streamInfo.bytesPerSampleFrame();

    std::copy_n(src, size(intersection) * m_streamInfo.bytesPerSampleFrame(), dstBytes);
    remaining -= intersection;
  }

  return remaining;
}

//----------------------------------------------------------------------------------------------------------------------

auto MfFileSource::readFromFile(const FrameRange& range, char* dst) -> FrameRangeSet
{
  FrameRangeSet remaining; remaining += range;

  // We start by retrieving the leftmost data block with valid data.

  auto block = searchAndRetrieveBlock(range.lower());
  if (not block || not updateBuffer(std::move(block))) return remaining;

  // We then fill out the remaining bits of the range by consuming the next data blocks. The output of
  // readFromBuffer() is a set which in this case contains only one element.

  remaining = readFromBuffer(range, dst);

  while (not boost::icl::contains(m_buffer->frameRange(), range.upper() - 1))
  {
    block = consumeBlock();
    if (not block || not updateBuffer(std::move(block))) return remaining;

    auto offset = (remaining.begin()->lower() - range.lower()) * m_streamInfo.bytesPerSampleFrame();
    remaining   = readFromBuffer(*remaining.begin(), dst + offset);
  }

  return remaining;
}

//----------------------------------------------------------------------------------------------------------------------

auto MfFileSource::searchAndRetrieveBlock(offset_type target) -> std::unique_ptr<MfBlock>
{
  // We start by discarding the first data block if it contains a fade-in.

  if (not discardFadeinBlock()) return nullptr;

  // We then fetch the next block and check its timestamp to determine where we are in the audio stream, then move to
  // the target position.

  auto block = consumeBlock();
  if (not block) return nullptr;

  const auto& range = block->range;
  if (boost::icl::contains(range, target)) return block;
  else if (target >= range.upper())        return searchForwardAndRetrieveBlock (target);
  else                                     return searchBackwardAndRetrieveBlock(target, 0);
}

//----------------------------------------------------------------------------------------------------------------------

auto MfFileSource::searchForwardAndRetrieveBlock(offset_type target) -> std::unique_ptr<MfBlock>
{
  // If the target frame is ahead of the current position in the audio stream, we keep consuming data blocks until
  // we reach it. According to the WMF documentation, we are not going to overshoot.

  std::unique_ptr<MfBlock> block;
  do
  {
    block = consumeBlock();
  } while (block && not boost::icl::contains(block->range, target));
  return block;
}

//----------------------------------------------------------------------------------------------------------------------

auto MfFileSource::searchBackwardAndRetrieveBlock(offset_type target, offset_type backstep) -> std::unique_ptr<MfBlock>
{
  // If the target frame is behind the current position in the audio stream, we seek backwards recursively with an
  // increasing step until we reach it. If we overshoot, we simply move forward by consuming the excess blocks.

  auto correctedTarget = target >= m_readOffset + backstep ? target - backstep : m_readOffset;
  if (seekInternal(correctedTarget))
  {
    if (not discardFadeinBlock()) return nullptr;

    auto block = consumeBlock();
    if (not block) return nullptr;

    const auto& range = block->range;
    if (boost::icl::contains(range, target)) return block;
    else if (target >= range.upper())        return searchForwardAndRetrieveBlock(target);
    else
    {
      return correctedTarget > m_readOffset
        ? searchBackwardAndRetrieveBlock(target, backstep + 2 * s_defaultBlockSize) : nullptr;
    }
  }

  return nullptr;
}

namespace
{

//----------------------------------------------------------------------------------------------------------------------

bool checkErrors(const IMFSample* mfSample, DWORD flags)
{
  return
    (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) == 0
    && (flags & MF_SOURCE_READERF_ENDOFSTREAM) == 0
    && (flags & MF_SOURCE_READERF_STREAMTICK) == 0
    && mfSample;
}

} // namespace anonymous

//----------------------------------------------------------------------------------------------------------------------

auto MfFileSource::consumeBlock() -> std::unique_ptr<MfBlock>
{
  DWORD flags = 0; LONGLONG timestamp = 0, duration = 0;
  auto mfSample = allocateNoThrow([this, &flags, &timestamp] ( IMFSample** p )
  {
    return m_reader->ReadSample(0, 0, nullptr, &flags, &timestamp, p);
  });

  if (not checkErrors(mfSample.get(), flags) || FAILED(mfSample->GetSampleDuration(&duration))) return nullptr;

  auto beg = time100nsToFrames(timestamp, m_streamInfo.sampleRate());
  auto end = beg + time100nsToFrames(duration, m_streamInfo.sampleRate());

  std::unique_ptr<MfBlock> block(new MfBlock{ { beg, end }, std::move(mfSample) });
  return block;
}

//----------------------------------------------------------------------------------------------------------------------

bool MfFileSource::updateBuffer(std::unique_ptr<MfBlock> block)
{
  auto buffer = boost::make_unique<MfBuffer>(std::move(block));
  if (not buffer->isLocked()) return false;

  m_buffer.swap(buffer);
  return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool MfFileSource::discardFadeinBlock()
{
  // If this is the first fetch after a seek we need to discard the first frame, as WMF applies a fade-in
  // effect to the first few samples that produces crackling when played back by Traktor.

  if (m_fadein)
  {
    auto frame = consumeBlock();
    if (not frame) return false;
    m_fadein = false;
  }

  return true;
}
