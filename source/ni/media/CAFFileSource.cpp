//-----------------------------------------------------------------------------
//  (c) Copyright NATIVE INSTRUMENTS, Berlin, Germany
//  ALL RIGHTS RESERVED
//-----------------------------------------------------------------------------

#include "CAFFileSource.h"
#include "CAFFileDescription.h"

#include <ALACDecoder.h>
#include <ALACBitUtilities.h>
#include <EndianPortable.h>

#include <ciso646>
#include <vector>

#include <boost/format.hpp>

#define kMaxBERSize 5

namespace
{

// Adapted from CoreAudioTypes.h
enum
{
    kTestFormatFlag_16BitSourceData    = 1,
    kTestFormatFlag_20BitSourceData    = 2,
    kTestFormatFlag_24BitSourceData    = 3,
    kTestFormatFlag_32BitSourceData    = 4
};

//----------------------------------------------------------------------------------------------------------------------

uint32_t ReadBERInteger(uint8_t * theInputBuffer, int32_t * ioNumBytes)
{
	uint32_t theAnswer = 0;
	uint8_t theData;
	int32_t size = 0;
    do
	{
		theData = theInputBuffer[size];
		theAnswer = (theAnswer << 7) | (theData & 0x7F);
		if (++size > 5)
		{
			size = 0xFFFFFFFF;
			return 0;
		}
	}
	while(((theData & 0x80) != 0) && (size <= *ioNumBytes));

    *ioNumBytes = size;

	return theAnswer;
}

//----------------------------------------------------------------------------------------------------------------------

int32_t FindCAFFPacketTableStart(AudioFileSource& source, int32_t * paktPos, int32_t * paktSize)
{
    // returns the absolute position within the file
    auto currentPosition = source.tell(); // record the current position
    uint8_t theReadBuffer[12];
    uint32_t chunkType = 0, chunkSize = 0;
    bool done = false;
    int32_t bytesRead = 8;

    source.seek(bytesRead, BOOST_IOS::beg); // start at 8!
    while (!done && bytesRead > 0) // no file size here
    {
        bytesRead = (int32_t)source.read((char*)theReadBuffer, 12);
        chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch(chunkType)
        {
            case 'pakt':
                *paktPos = (int32_t)source.tell() + kMinCAFFPacketTableHeaderSize;
                // big endian size
                *paktSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                done = true;
                break;
            default:
                chunkSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                source.seek(chunkSize, BOOST_IOS::cur);
                break;
        }
    }

    source.seek(currentPosition, BOOST_IOS::beg); // start at 0

    return 0;

}

//----------------------------------------------------------------------------------------------------------------------

uint32_t GetMagicCookieSizeFromCAFFkuki(AudioFileSource & source)
{
    // returns to the current absolute position within the file
    auto currentPosition = source.tell(); // record the current position
    uint8_t theReadBuffer[sizeof(ALACSpecificConfig)];
    uint32_t chunkType = 0, chunkSize = 0;
    bool done = false;
    std::streamsize bytesRead = sizeof(port_CAFFileHeader);
    uint32_t theCookieSize = 0;

    source.seek(bytesRead, BOOST_IOS::beg); // start at 8!
    while (!done && bytesRead > 0) // no file size here
    {
        bytesRead = source.read((char*)theReadBuffer, 12);
        chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch(chunkType)
        {
            case 'kuki':
            {
                theCookieSize = theReadBuffer[11];
                done = true;
                break;
            }
            default:
                chunkSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                source.seek(chunkSize, BOOST_IOS::cur);
                break;
        }
    }

    source.seek(currentPosition, BOOST_IOS::beg); // start at 0

    if (!done) return (uint32_t)-1;

    return theCookieSize;

}

//----------------------------------------------------------------------------------------------------------------------

int32_t GetMagicCookieFromCAFFkuki(AudioFileSource & source, uint8_t * outMagicCookie, uint32_t * ioMagicCookieSize)
{
    // returns to the current absolute position within the file
    auto currentPosition = source.tell(); // record the current position
    uint8_t theReadBuffer[12];
    uint32_t chunkType = 0, chunkSize = 0;
    bool done = false, cookieFound = false;
    int32_t bytesRead = sizeof(port_CAFFileHeader);
    uint32_t theStoredCookieSize = 0;

    source.seek(bytesRead, BOOST_IOS::beg); // start at 8!
    while (!done && bytesRead > 0) // no file size here
    {
        bytesRead = (int32_t)source.read((char*)theReadBuffer, 12);
        chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch(chunkType)
        {
            case 'kuki':
            {
                theStoredCookieSize = theReadBuffer[11];
                if (*ioMagicCookieSize >= theStoredCookieSize)
                {
                    source.read((char*)outMagicCookie, theStoredCookieSize);
                    *ioMagicCookieSize = theStoredCookieSize;
                    cookieFound = true;
                }
                else
                {
                    *ioMagicCookieSize = 0;
                }
                done = true;
                break;
            }
            default:
                chunkSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                source.seek(chunkSize, BOOST_IOS::cur);
                break;
        }
    }

    source.seek(currentPosition, BOOST_IOS::beg); // start at 0

    if (!done || !cookieFound) return -1;

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool FindCAFFDataStart(AudioFileSource & source, int32_t * dataPos, int32_t * dataSize)
{
    bool done = false;
    std::streamsize bytesRead = 8;
    uint32_t chunkType = 0, chunkSize = 0;
    uint8_t theBuffer[12];

    source.seek( bytesRead, BOOST_IOS::beg); // start at 8!
    while (!done && bytesRead > 0) // no file size here
    {
        bytesRead = source.read((char*)theBuffer, 12);
        chunkType = ((int32_t)(theBuffer[0]) << 24) + ((int32_t)(theBuffer[1]) << 16) + ((int32_t)(theBuffer[2]) << 8) + theBuffer[3];
        switch(chunkType)
        {
            case 'data':
                *dataPos = (int32_t)source.tell() + sizeof(uint32_t); // skip the edits
                // big endian size
                *dataSize = ((int32_t)(theBuffer[8]) << 24) + ((int32_t)(theBuffer[9]) << 16) + ((int32_t)(theBuffer[10]) << 8) + theBuffer[11];
                *dataSize -= 4; // the edits are included in the size
                done = true;
                break;
            default:
                chunkSize = ((int32_t)(theBuffer[8]) << 24) + ((int32_t)(theBuffer[9]) << 16) + ((int32_t)(theBuffer[10]) << 8) + theBuffer[11];
                source.seek( chunkSize, BOOST_IOS::cur);
                break;
        }
    }
    return done;
}

//----------------------------------------------------------------------------------------------------------------------

bool GetCAFFdescFormat(AudioFileSource & source, AudioFormatDescription * theInputFormat)
{
    bool done = false;
    uint32_t theChunkSize = 0, theChunkType = 0;
    uint8_t theReadBuffer[32];

    source.seek( 4, BOOST_IOS::cur); // skip 4 bytes

    while (!done)
    {
        source.read((char*)theReadBuffer, 4);
        theChunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch (theChunkType)
        {
            case 'desc':
                source.seek( 8, BOOST_IOS::cur); // skip 8 bytes
                source.read((char*)theReadBuffer, sizeof(port_CAFAudioDescription));
                theInputFormat->mFormatID = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mFormatID);
                theInputFormat->mChannelsPerFrame = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mChannelsPerFrame);
                theInputFormat->mSampleRate = SwapFloat64BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mSampleRate);
                theInputFormat->mBitsPerChannel = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mBitsPerChannel);
                theInputFormat->mFormatFlags = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mFormatFlags);
                theInputFormat->mBytesPerPacket = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mBytesPerPacket);
                if (theInputFormat->mFormatID == kALACFormatAppleLossless)
                {
                    theInputFormat->mBytesPerFrame = 0;
                }
                else
                {
					theInputFormat->mBytesPerFrame = theInputFormat->mBytesPerPacket;
					if ((theInputFormat->mFormatFlags & 0x02) == 0x02)
					{
						theInputFormat->mFormatFlags &= 0xfffffffc;
					}
					else
					{
						theInputFormat->mFormatFlags |= 0x02;
					}

                }
                theInputFormat->mFramesPerPacket = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mFramesPerPacket);
                theInputFormat->mReserved = 0;
                done = true;
                break;
            default:
                // read the size and skip
                source.read((char*)theReadBuffer, 8);
                theChunkSize = ((int32_t)(theReadBuffer[4]) << 24) + ((int32_t)(theReadBuffer[5]) << 16) + ((int32_t)(theReadBuffer[6]) << 8) + theReadBuffer[7];
                source.seek( theChunkSize, BOOST_IOS::cur);
                break;
        }
    }
    return done;
}

//----------------------------------------------------------------------------------------------------------------------

int32_t GetInputFormat(AudioFileSource & source, AudioFormatDescription * theInputFormat, uint32_t * theFileType)
{
    // assumes the file is open
    uint8_t theReadBuffer[20];
    bool done = false;

    source.read((char*)theReadBuffer, 4);

    if (theReadBuffer[0] == 'c' && theReadBuffer[1] == 'a' && theReadBuffer[2] == 'f'  && theReadBuffer[3] == 'f')
    {
        // It's a caff file!
        *theFileType = 'caff';
        // We support pcm data for encode and alac data for decode
        done = GetCAFFdescFormat(source, theInputFormat);
    }
    else
    {
        *theFileType = 0; // clear it
        return -1;
    }

    if (!done) return -1;

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------

int32_t SetOutputFormat( AudioFormatDescription theInputFormat, AudioFormatDescription * theOutputFormat )
{

  assert( theInputFormat.mFormatID == kALACFormatAppleLossless );
  // decoding
  theOutputFormat->mFormatID = kALACFormatLinearPCM;
  theOutputFormat->mSampleRate = theInputFormat.mSampleRate;

  switch ( theInputFormat.mFormatFlags )
  {
    case kTestFormatFlag_16BitSourceData:
      theOutputFormat->mBitsPerChannel = 16;
      break;
    case kTestFormatFlag_20BitSourceData:
      theOutputFormat->mBitsPerChannel = 20;
      break;
    case kTestFormatFlag_24BitSourceData:
      theOutputFormat->mBitsPerChannel = 24;
      break;
    case kTestFormatFlag_32BitSourceData:
      theOutputFormat->mBitsPerChannel = 32;
      break;
    default:
      return -1;
      break;
  }

  theOutputFormat->mFramesPerPacket = 1;
  theOutputFormat->mChannelsPerFrame = theInputFormat.mChannelsPerFrame;
  theOutputFormat->mBytesPerPacket = theOutputFormat->mBytesPerFrame = theOutputFormat->mBitsPerChannel != 20 ? theInputFormat.mChannelsPerFrame * ((theOutputFormat->mBitsPerChannel) >> 3) : (int32_t)(theInputFormat.mChannelsPerFrame * 2.5 + .5);
  theOutputFormat->mFormatFlags = kALACFormatFlagsNativeEndian;
  theOutputFormat->mReserved = 0;

  return 0;
}

//----------------------------------------------------------------------------------------------------------------------

int32_t FindDataStart(AudioFileSource & source, int32_t * dataPos, int32_t * dataSize)
{
    // returns the absolute position within the file
    auto cur = source.tell(); // record the current position
    bool done = FindCAFFDataStart(source, dataPos, dataSize);
    source.seek(cur, BOOST_IOS::beg); // start at 0

    if (!done) return -1;

    return 0;
}

} // namespace

//----------------------------------------------------------------------------------------------------------------------

CAFFileSource::CAFFileSource( const std::string& path )
: AudioFileSource( path, BOOST_IOS::binary | BOOST_IOS::in )
, m_decoder(new ALACDecoder())
{
  if (not is_open()) throw std::runtime_error("Could not open the audio file.");
  readHeader();
}

//----------------------------------------------------------------------------------------------------------------------

void CAFFileSource::open( const std::string& path )
{
  AudioFileSource::open( path, BOOST_IOS::binary | BOOST_IOS::in );
  if (not is_open()) throw std::runtime_error("Could not open the audio file.");
  readHeader();
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize CAFFileSource::read( char_type* dest, std::streamsize n )
{
  size_t requested = size_t(n);
  size_t delivered = 0;
  size_t copied = std::min( m_writeBuffer.size() - m_writeOffset, requested );

  do
  {
    dest = std::copy_n( m_writeBuffer.begin() + m_writeOffset, copied, dest );
    m_writeOffset += copied;
    delivered += copied;
    if ( m_writeOffset == m_writeBuffer.size() )
    {
      decodeBlock();
      copied = std::min( m_writeBuffer.size(), requested - delivered );
    }
  }
  while (!m_writeBuffer.empty() && delivered < requested );

  m_pos += delivered;
  return delivered;
}

//----------------------------------------------------------------------------------------------------------------------

size_t CAFFileSource::decodeBlock( )
{

    AudioFileSource::seek( m_packetTablePos, BOOST_IOS::beg );
    int32_t numBytes = (int32_t)AudioFileSource::read( (char*)m_readBuffer.data(), kMaxBERSize );

    int32_t numBytesToRead = ReadBERInteger( m_readBuffer.data(), &numBytes );
    m_packetTablePos += numBytes;
    AudioFileSource::seek( m_inputDataPos, BOOST_IOS::beg );
    m_inputDataPos += numBytesToRead;

    uint32_t numFrames = 0;
    if ( (numBytesToRead > 0) && ((size_t)numBytesToRead == AudioFileSource::read( (char*)m_readBuffer.data(), numBytesToRead )) )
    {
      BitBuffer readBuffer;
      BitBufferInit(&readBuffer, m_readBuffer.data(), m_readBuffer.size());
      m_writeBuffer.resize( m_readBuffer.size() );
      m_decoder->Decode( &readBuffer, m_writeBuffer.data(), m_inputFormat.mFramesPerPacket, m_inputFormat.mChannelsPerFrame, &numFrames );
    }
    m_writeBuffer.resize( numFrames * m_outputFormat.mBytesPerFrame );
    m_writeOffset = 0;
    return m_writeBuffer.size();
}

//----------------------------------------------------------------------------------------------------------------------

std::streampos CAFFileSource::seek( boost::iostreams::stream_offset /*off*/, BOOST_IOS::seekdir /*way*/ )
{

  //std::streampos pos = (way == BOOST_IOS::cur) ? m_pos + off : boost::iostreams::stream_offset_to_streamoff( off );
  // TODO : seeking
  return m_pos;
}

//----------------------------------------------------------------------------------------------------------------------

void CAFFileSource::readHeader()
{
  AudioFileSource & source = *this;

  uint32_t inputFileType = 0; // 'caff' or 'WAVE'

  if ( GetInputFormat( source, &m_inputFormat, &inputFileType ) )
    throw std::runtime_error("Unable to determine data format");

  if ( inputFileType != 'caff' || m_inputFormat.mFormatID != kALACFormatAppleLossless )
    throw std::runtime_error("Unsupported data format");

  if ( SetOutputFormat( m_inputFormat, &m_outputFormat ) )
    throw std::runtime_error("Unsupported data format");

  if ( FindDataStart( source, &m_inputDataPos, &m_inputDataSize ) )
    throw std::runtime_error("Unsupported data format");

  if ( m_outputFormat.mChannelsPerFrame > 2 )
    throw std::runtime_error("Multichannel not supported. Maximum is two channel");

  source.seek( m_inputDataPos, BOOST_IOS::beg );

  // We need to get the cookie from the file
  auto theMagicCookieSize = GetMagicCookieSizeFromCAFFkuki( source );
  auto theMagicCookie = std::vector<uint8_t>( theMagicCookieSize );
  GetMagicCookieFromCAFFkuki( source, theMagicCookie.data(), &theMagicCookieSize );

  if ( m_decoder->Init( theMagicCookie.data(), theMagicCookieSize ) )
    throw std::runtime_error( "Could not read magic cookie" );

  m_readBuffer.resize ( m_inputFormat.mChannelsPerFrame * (m_outputFormat.mBitsPerChannel >> 3) * m_inputFormat.mFramesPerPacket + kALACMaxEscapeHeaderBytes);
  m_writeBuffer.resize( m_readBuffer.size() );
  m_writeOffset = m_writeBuffer.size();

  FindCAFFPacketTableStart( source, &m_packetTablePos, &m_packetTableSize );

  // In order to retrieve the total data size we need to decode the entire file
  size_t numDataBytes = 0;
  for ( auto numBytes = 0; numBytes = decodeBlock() ; numDataBytes += numBytes );
  // Then, seek back to beginning
  FindCAFFDataStart( source, &m_inputDataPos, &m_inputDataSize );
  FindCAFFPacketTableStart( source, &m_packetTablePos, &m_packetTableSize );
  source.seek( m_inputDataPos, BOOST_IOS::beg );

  pcm::format format;
  switch ( m_decoder->mConfig.bitDepth )
  {
    case 8:  format = pcm::format::s8();    break;
    case 16: format = pcm::format::s16le(); break;
    case 24: format = pcm::format::s24le(); break;
    case 32: format = pcm::format::s32le(); break;
    default:
      throw std::runtime_error(boost::str(boost::format("%u bit depth found. Only 8, 16, 24 and 32-bit are supported.")
        %  m_decoder->mConfig.bitDepth));
  }

  AudioStreamInfo info;

  info.numSampleFrames( numDataBytes );
  info.numChannels( m_decoder->mConfig.numChannels );
  info.sampleRate( m_decoder->mConfig.sampleRate );
  info.format( format );
  audioStreamInfo( info );
}

//----------------------------------------------------------------------------------------------------------------------
