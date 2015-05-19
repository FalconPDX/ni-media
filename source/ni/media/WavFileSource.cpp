//-----------------------------------------------------------------------------
//  (c) Copyright NATIVE INSTRUMENTS, Berlin, Germany
//  ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------

#include "WavFileSource.h"


// Utilities

#include "WAVChunks.h"

#include <ciso646>

#include <boost/format.hpp>

namespace {

static const boost::uint8_t aWavFormatExtSubFormatPCM[] =
{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
static const boost::uint8_t aWavFormatExtSubFormatPCM_2[] =
{0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
static const boost::uint8_t aWavFormatExtSubFormatFloat[] =
{0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
static const boost::uint8_t aWavFormatExtSubFormatFloat_2[] =
{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};

// TODO : change this!
  uint32_t charToInt(const char a, const char b, const char c, const char d)
  {
    return  ((((int)a) << 24) | (((int)b) << 16) | (((int)c) << 8) | ((int)d));
  }

}

//----------------------------------------------------------------------------------------------------------------------

WavFileSource::WavFileSource( const std::string& path )
: AudioFileSource( path, BOOST_IOS::binary | BOOST_IOS::in )
, m_nBlockAlign(0)
{
  if (not is_open()) throw std::runtime_error("Could not open the audio file.");
  readHeader();
}


//----------------------------------------------------------------------------------------------------------------------

void WavFileSource::open( const std::string& path )
{
  AudioFileSource::open(path, BOOST_IOS::binary | BOOST_IOS::in);
  if (not is_open()) throw std::runtime_error("Could not open the audio file.");
  readHeader();
}


//----------------------------------------------------------------------------------------------------------------------

std::streamsize WavFileSource::read( char_type* s, std::streamsize n )
{
  std::streampos pos = seek( 0, BOOST_IOS::cur );
  std::streampos end = audioStreamInfo().numBytes();

  if( pos >= end )
    return 0;

  std::streamsize nMax = std::min( n, std::streamsize(end  - pos) );
  return AudioFileSource::read( s, nMax );
}


//----------------------------------------------------------------------------------------------------------------------

std::streampos WavFileSource::seek( boost::iostreams::stream_offset off, BOOST_IOS::seekdir way )
{
  std::streampos pos = (way == BOOST_IOS::cur) ?
    AudioFileSource::seek( off, way ) : AudioFileSource::seek( off + m_offs, way );
  return pos - boost::iostreams::stream_offset_to_streamoff(m_offs);
}

//----------------------------------------------------------------------------------------------------------------------

void WavFileSource::readHeader()
{
  m_offs = 0;

  AudioStreamInfo info;

  std::streamsize ncount = 0;

  AudioFileSource::seek(0, std::ios::beg);

  // read riff chunk
  tRiffTag aRiffTag;
  ncount = AudioFileSource::read(reinterpret_cast<char*>(&aRiffTag.nID), 4);
  ncount = AudioFileSource::read(reinterpret_cast<char*>(&aRiffTag.nLength), 4);
  if (aRiffTag.nID != charToInt('F','F','I','R')) throw std::runtime_error("Could not read \'RIFF\' tag.");

  // read wave chunk
  ncount = AudioFileSource::read(reinterpret_cast<char*>(&aRiffTag.nID), 4);
  if (aRiffTag.nID != charToInt('E','V','A','W')) throw std::runtime_error("Could not read \'WAVE\' tag.");

  while ( true )
  {
    ncount = AudioFileSource::read(reinterpret_cast<char*>(&aRiffTag.nID), 4);
    ncount = AudioFileSource::read(reinterpret_cast<char*>(&aRiffTag.nLength), 4);

    if (!aRiffTag.nLength) throw std::runtime_error("Could not read length of \'RIFF\' chunk." );

    // seek through wav chunks
    if (aRiffTag.nID  == charToInt(' ','t','m','f'))
    {
      // handle the fmt block
      boost::uint16_t nFormatTag;
      boost::uint16_t nNumChannels;
      boost::uint32_t nSampleRate;
      boost::uint32_t nAvgBytesPerSec;
      boost::uint16_t nBlockAlign;
      boost::uint16_t nBitsPerSample;

      ncount = AudioFileSource::read(reinterpret_cast<char*>(&nFormatTag), 2);
      ncount = AudioFileSource::read(reinterpret_cast<char*>(&nNumChannels), 2);
      ncount = AudioFileSource::read(reinterpret_cast<char*>(&nSampleRate), 4);
      ncount = AudioFileSource::read(reinterpret_cast<char*>(&nAvgBytesPerSec), 4);
      ncount = AudioFileSource::read(reinterpret_cast<char*>(&nBlockAlign), 2);
      ncount = AudioFileSource::read(reinterpret_cast<char*>(&nBitsPerSample), 2);

      info.numChannels(nNumChannels);
      info.sampleRate(nSampleRate);

      AudioStreamInfo::format_t format;

      // determine format
      if (nFormatTag == CONST_WAVE_FORMAT_EXTENSIBLE)
      {
        // read extended chunk
        tWAVFormatExtensible aFormatExtensible;
        ncount = AudioFileSource::read(reinterpret_cast<char*>(&aFormatExtensible), sizeof(tWAVFormatExtensible));

        if (memcmp(aFormatExtensible.aSubFormat, aWavFormatExtSubFormatPCM, 16) ||
          memcmp(aFormatExtensible.aSubFormat, aWavFormatExtSubFormatPCM_2, 16))
        {
          nFormatTag = CONST_WAVE_FORMAT_PCM;
        }
        else if (memcmp(aFormatExtensible.aSubFormat, aWavFormatExtSubFormatFloat, 16) ||
          memcmp(aFormatExtensible.aSubFormat, aWavFormatExtSubFormatFloat_2, 16))
        {
          nFormatTag = CONST_WAVE_FORMAT_IEEE_FLOAT;
        }

        AudioFileSource::seek(aRiffTag.nLength - 16 - sizeof(tWAVFormatExtensible), std::ios::cur);
      }
      else
      {
        AudioFileSource::seek(aRiffTag.nLength - 16, std::ios::cur);
      }

      if (nFormatTag == CONST_WAVE_FORMAT_PCM)
      {
        switch (nBitsPerSample)
        {
        case 8:     format = ::pcm::format::u8();  break;
        case 16:    format = ::pcm::format::s16le(); break;
        case 24:    format = ::pcm::format::s24le(); break;
        case 32:    format = ::pcm::format::s32le(); break;
        default:
          throw std::runtime_error(boost::str(
            boost::format("%u integer bit depth found. Only 8, 16, 24 and 32-bit are supported.") % nBitsPerSample));
        }
      }
      else if (nFormatTag == CONST_WAVE_FORMAT_IEEE_FLOAT)
      {
        switch (nBitsPerSample)
        {
        case 32:    format = ::pcm::format::f32le(); break;
        case 64:    format = ::pcm::format::f64le(); break;
        default:
          throw std::runtime_error(boost::str(
            boost::format("%u floating point bit depth found. Only 32 and 64-bit are supported.") % nBitsPerSample));
        }
      }
      else throw std::runtime_error("Unknown format.");

      info.format(format);

      if (m_nBlockAlign == 0)
      {
        m_nBlockAlign = (nBitsPerSample/8) * static_cast<unsigned>(info.numChannels());
      }

    }
    else if(aRiffTag.nID  == charToInt('a','t','a','d'))
    {
      m_offs = tell();
      info.numSampleFrames( aRiffTag.nLength / m_nBlockAlign );
      audioStreamInfo(info);
      return;
    }
    else if(aRiffTag.nID  == charToInt('l','p','m','s'))
    {
      if (aRiffTag.nLength >= sizeof(tWAVSampleChunk))
      {
        tWAVSampleChunk aSamplerChunk;
        ncount = AudioFileSource::read(reinterpret_cast<char*>(&aSamplerChunk.nManufacturer), sizeof(tWAVSampleChunk));

        const size_t nBytes = ((aRiffTag.nLength + 1) & 0xfffffe) -  sizeof(tWAVSampleChunk);
        AudioFileSource::seek(nBytes, std::ios::cur);
      }
      else
      {
        AudioFileSource::seek((aRiffTag.nLength + 1) & 0xfffffe, std::ios::cur);
      }
    }
    else if(aRiffTag.nID  == charToInt('t','s','n','i'))
    {
      if (aRiffTag.nLength >= sizeof(tWAVInstrumentChunk))
      {
        tWAVInstrumentChunk aInstChunk;
        ncount = AudioFileSource::read(reinterpret_cast<char*>(&aInstChunk.nRootNote), sizeof(tWAVInstrumentChunk));

        const size_t nBytes = (((aRiffTag.nLength + 1) & 0xfffffe)) -  sizeof(tWAVInstrumentChunk);
        AudioFileSource::seek(nBytes, std::ios::cur);
      }
      else
      {
        AudioFileSource::seek((aRiffTag.nLength + 1) & 0xfffffe, std::ios::cur);
      }
    }
    else
    {
      // make chunk size even
      AudioFileSource::seek((aRiffTag.nLength + 1) & 0xfffffe, std::ios::cur);
    }
  }

  throw std::runtime_error("Could not read \'data\' tag.");
}

//----------------------------------------------------------------------------------------------------------------------
