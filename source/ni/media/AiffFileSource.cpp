#include "AiffFileSource.h"

#include <ciso646>
#include <cmath>

#include <boost/format.hpp>


namespace {

// TODO: Replace with methods of the same name in boost::endian when moving to BOOST 1.58

void big_to_native_inplace(uint32_t& x)
{
  x = (x & 0xFF000000u) >> 24 | (x & 0x00FF0000u) >> 8 | (x & 0x0000FF00u) << 8 | x << 24;
}

void big_to_native_inplace(uint16_t& x)
{
  x = (x & 0xFF00u) >> 8 | x << 8;
}

} // namespace anonymous

namespace aiff {
namespace tags {

const size_t   size = 4;

// TODO: Replace with boost::endian buffers
const uint32_t aifc = 'AIFC';
const uint32_t aiff = 'AIFF';
const uint32_t comm = 'COMM';
const uint32_t form = 'FORM';
const uint32_t ssnd = 'SSND';

} // namespace tags

struct Tag
{
  // TODO: Test Replacing with boost::endian buffers
  uint32_t        id      = 0;
  uint32_t        length  = 0;
  uint32_t        subType = 0;
  std::streamsize start   = 0;
};

// The following structs are used in disk operations so we need to prevent the compiler from expanding them

#pragma pack(push)
#pragma pack(1)

struct CommonChunk
{
  // TODO: Test Replacing with boost::endian buffers
  uint16_t numChannels       = 0;
  uint32_t numSampleFrames   = 0;
  uint16_t sampleSize        = 0;
  uint8_t  extSampleRate[10] {};
};

struct SsndChunk
{
  // TODO: Test Replacing with boost::endian buffers
  uint32_t offset    = 0;
  uint32_t blockSize = 0;
};

#pragma pack(pop)

} // namespace aiff

namespace {

//----------------------------------------------------------------------------------------------------------------------

auto readTag(AudioFileSource& src, uint32_t id) -> boost::optional<aiff::Tag>
{
  while (true)
  {
    aiff::Tag tag;
    if  (src.read(reinterpret_cast<char*>(&tag.id),      aiff::tags::size) != aiff::tags::size
      || src.read(reinterpret_cast<char*>(&tag.length),  aiff::tags::size) != aiff::tags::size
      || src.read(reinterpret_cast<char*>(&tag.subType), aiff::tags::size) != aiff::tags::size) return boost::none;

    big_to_native_inplace(tag.id);
    big_to_native_inplace(tag.length);
    big_to_native_inplace(tag.subType);

    tag.start = src.seek(0, BOOST_IOS::cur);

    if (tag.id == id && (tag.subType == aiff::tags::aiff || tag.subType == aiff::tags::aifc))
      return boost::make_optional(tag);

    static const size_t padSize = 2;
    auto remainder = tag.length % padSize;
    auto target    = tag.length + (remainder != 0 ? padSize - remainder : 0);
    src.seek(target, BOOST_IOS::cur);
  }

  return boost::none;
}

//----------------------------------------------------------------------------------------------------------------------

auto readSubTag(AudioFileSource& src, uint32_t id, const aiff::Tag& parent) -> boost::optional<aiff::Tag>
{
  auto end = parent.start + parent.length;
  while(src.seek(0, BOOST_IOS::cur) < end)
  {
    aiff::Tag tag;
    if  (src.read(reinterpret_cast<char*>(&tag.id),     aiff::tags::size) != aiff::tags::size
      || src.read(reinterpret_cast<char*>(&tag.length), aiff::tags::size) != aiff::tags::size) return boost::none;

    big_to_native_inplace(tag.id);
    big_to_native_inplace(tag.length);

    tag.start = src.seek(0, BOOST_IOS::cur);

    if (tag.id == id) return boost::make_optional(tag);

    static const size_t padSize = 2;
    auto remainder = tag.length % padSize;
    auto target    = tag.length + (remainder != 0 ? padSize - remainder : 0);
    src.seek(target, BOOST_IOS::cur);
  }

  return boost::none;
}

//----------------------------------------------------------------------------------------------------------------------

double IEEE80_to_double(uint8_t* p)
{
  int16_t exponent = *p++;
  exponent <<= 8;
  exponent |= *p++;
  int8_t sign = (exponent & 0x8000) ? 1 : 0;
  exponent &= 0x7FFF;

  uint32_t mant1 = *p++;
  for (size_t i = 0; i < 3; ++i) { mant1 <<= 8; mant1 |= *p++; }

  uint32_t mant0 = *p++;
  for (size_t i = 0; i < 3; ++i) { mant0 <<= 8; mant0 |= *p++; }

  if (mant1 == 0 && mant0 == 0 && exponent == 0 && sign == 0) return 0.0;
  else
  {
    double val = mant0 * std::pow(2.0, -63.0);
    val += mant1 * std::pow(2.0, -31.0);
    val *= std::pow(2.0, exponent - 16383.0);
    return sign ? -val : val;
  }
}

} // anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

AiffFileSource::AiffFileSource(const std::string& path) :
  AudioFileSource(path, BOOST_IOS::binary | BOOST_IOS::in)
{
  if (not is_open()) throw std::runtime_error("Could not open the audio file.");
  readHeader();
}

//----------------------------------------------------------------------------------------------------------------------

void AiffFileSource::open(const std::string& path)
{
  AudioFileSource::open(path, BOOST_IOS::binary | BOOST_IOS::in);
  if (not is_open()) throw std::runtime_error("Could not open the audio file.");
  readHeader();
}

//----------------------------------------------------------------------------------------------------------------------

std::streampos AiffFileSource::seek(boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
{
  std::streampos pos = way == BOOST_IOS::beg
    ? AudioFileSource::seek(off + m_offset, way) : AudioFileSource::seek(off, way);
  return pos - boost::iostreams::stream_offset_to_streamoff(m_offset);
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize AiffFileSource::read(char_type* s, std::streamsize n)
{
  std::streampos pos = seek(0, BOOST_IOS::cur);
  std::streampos end = audioStreamInfo().numBytes();

  if (pos >= end) return 0;

  auto nMax = std::min(n, std::streamsize(end  - pos));
  return AudioFileSource::read(s, nMax);
}

//----------------------------------------------------------------------------------------------------------------------

void AiffFileSource::readHeader()
{
  m_offset = 0;
  AudioFileSource::seek(0, BOOST_IOS::beg);

  // Read the 'FORM'chunk

  auto formTag = readTag(*this, aiff::tags::form);
  if (not formTag) throw std::runtime_error("Could not read \'FORM\' tag.");

  // Retrieve the 'COMM' subchunk, which contains the audio format information

  static const size_t commChunkSize = sizeof(aiff::CommonChunk);

  auto commTag = readSubTag(*this, aiff::tags::comm, formTag.get());
  if (not commTag) throw std::runtime_error("Could not read \'COMM\' tag.");

  bool littleEndian = false;

  if (formTag->subType == aiff::tags::aifc)
  {
    uint32_t compression = 0;
    if (AudioFileSource::read(reinterpret_cast<char*>(&compression), aiff::tags::size) != aiff::tags::size)
      throw std::runtime_error("Could not retrieve the compression id.");

    if (commTag->length > commChunkSize + 4)
    {
      // Read the Pascal-style string containing the name.

      uint8_t compressor[256];
      uint8_t compressorNameLength;

      if  (AudioFileSource::read(reinterpret_cast<char*>(&compressorNameLength), 1) != 1
        || AudioFileSource::read(reinterpret_cast<char*>(compressor), compressorNameLength) != compressorNameLength)
      {
        throw std::runtime_error("Could not retrieve the compressor id.");
      }
    }

    switch (compression)
    {
    case 'NONE': break;
    case 'sowt': littleEndian = true; break;
    default:
      throw std::runtime_error("Unsupported compressor.");
    }
  }

  aiff::CommonChunk commChunk;
  if  (commTag->length < commChunkSize
    || AudioFileSource::read(reinterpret_cast<char*>(&commChunk) , commChunkSize) != commChunkSize)
  {
    throw std::runtime_error("Could not read \'COMM\' chunk.");
  }

  big_to_native_inplace(commChunk.numChannels);
  big_to_native_inplace(commChunk.numSampleFrames);
  big_to_native_inplace(commChunk.sampleSize);

  using format_t = AudioStreamInfo::format_t; format_t format;

  switch (commChunk.sampleSize)
  {
  case 8:  format = format_t::u8(); break;
  case 16: format = littleEndian ? format_t(format_t::s16le()) : format_t(format_t::s16be()); break;
  case 24: format = littleEndian ? format_t(format_t::s24le()) : format_t(format_t::s24be()); break;
  case 32: format = littleEndian ? format_t(format_t::s32le()) : format_t(format_t::s32be()); break;
  default:
    throw std::runtime_error(boost::str(boost::format("%u bit depth found. Only 8, 16, 24 and 32-bit are supported.")
      % commChunk.sampleSize));
  }

  // The SSND chunk is the last in the header

  static const size_t ssndChunkSize = sizeof(aiff::SsndChunk);
  aiff::SsndChunk ssndChunck;

  auto ssndTag = readSubTag(*this, aiff::tags::ssnd, formTag.get());
  if (not ssndTag || AudioFileSource::read(reinterpret_cast<char*>(&ssndChunck), ssndChunkSize) != ssndChunkSize)
  {
    throw std::runtime_error("Could not read \'SSND\' chunk.") ;
  }

  big_to_native_inplace(ssndChunck.offset);
  big_to_native_inplace(ssndChunck.blockSize);

  AudioFileSource::seek(ssndChunck.offset, BOOST_IOS::cur);
  m_offset = boost::iostreams::position_to_offset(AudioFileSource::seek(0, BOOST_IOS::cur));

  AudioStreamInfo info;
  info.format         (format);
  info.numChannels    (commChunk.numChannels);
  info.numSampleFrames(commChunk.numSampleFrames);
  info.sampleRate     (size_t(IEEE80_to_double(commChunk.extSampleRate)));
  audioStreamInfo(info);
}
