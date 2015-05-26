#include "MP4FileSourceMac.h"

#include <boost/algorithm/clamp.hpp>


namespace
{

//----------------------------------------------------------------------------------------------------------------------

AudioStreamInfo buildOutStreamInfo(ExtAudioFileRef& media)
{
  AudioStreamInfo info;

  AudioStreamBasicDescription descriptor;
  UInt32 size = sizeof(descriptor);
  if (ExtAudioFileGetProperty(media, kExtAudioFileProperty_FileDataFormat, &size, &descriptor) != noErr)
  {
    throw std::runtime_error("Could not retrieve the audio format.");
  }

  info.format     (pcm::format::f32le());
  info.sampleRate (descriptor.mSampleRate);
  info.numChannels(descriptor.mChannelsPerFrame);

  SInt64 numFrames = 0;
  size = sizeof(numFrames);
  if (ExtAudioFileGetProperty(media, kExtAudioFileProperty_FileLengthFrames, &size, &numFrames) != noErr)
  {
    throw std::runtime_error("Could not read the track length.");
  }

  info.numSampleFrames(numFrames);
  return info;
}

} // namespace anonymous

//----------------------------------------------------------------------------------------------------------------------

MP4FileSource::Impl::Impl(const std::string& path)
{
  auto url = CFURLCreateFromFileSystemRepresentation(
    nullptr, reinterpret_cast<const UInt8*>(path.c_str()), path.size(), false);
  if (ExtAudioFileOpenURL(url, &m_media) != noErr) throw std::runtime_error("Could not open the audio file.");

  AudioFileID fileId;
  UInt32 size = sizeof(fileId);
  if (ExtAudioFileGetProperty(m_media, kExtAudioFileProperty_AudioFile, &size, &fileId) != noErr)
  {
    throw std::runtime_error("Could not retrieve the file id.");
  }

  size_t index = 0;
  if (AudioFileSetProperty(fileId, 'uatk', sizeof(index), &index) != noErr)
  {
    throw std::runtime_error("Could not select the audio stream.");
  }

  auto outStreamInfo = buildOutStreamInfo(m_media);

  AudioStreamBasicDescription description
  {
    .mFormatID         = kAudioFormatLinearPCM,
    .mChannelsPerFrame = UInt32 (outStreamInfo.numChannels()),
    .mSampleRate       = Float64(outStreamInfo.sampleRate()),
    .mFormatFlags      = kAudioFormatFlagIsFloat,
    .mFramesPerPacket  = 1,
    .mBitsPerChannel   = UInt32(outStreamInfo.bitsPerSample()),
    .mBytesPerPacket   = UInt32(outStreamInfo.bytesPerSampleFrame()),
    .mBytesPerFrame    = UInt32(outStreamInfo.bytesPerSampleFrame())
  };

  if (ExtAudioFileSetProperty(
    m_media, kExtAudioFileProperty_ClientDataFormat, sizeof(description), &description) != noErr)
  {
    throw std::runtime_error("Could not set the output format.");
  }

  m_streamInfo = outStreamInfo;
}

//----------------------------------------------------------------------------------------------------------------------

MP4FileSource::Impl::~Impl() { ExtAudioFileDispose(m_media); }

//----------------------------------------------------------------------------------------------------------------------

std::streampos MP4FileSource::Impl::seek(offset_t offset, BOOST_IOS::seekdir way)
{
  size_t frameSize  = m_streamInfo.bytesPerSampleFrame();
  offset_t framePos = offset / frameSize, endPos = m_streamInfo.numSampleFrames() - 1;

  switch (way)
  {
  case BOOST_IOS::beg: break;
  case BOOST_IOS::end: framePos  = endPos - framePos; break;
  default:             framePos += m_framePos;
  }

   framePos = boost::algorithm::clamp(framePos, 0, endPos);

  if (m_framePos != offset && ExtAudioFileSeek(m_media, framePos) != noErr) m_framePos = framePos;
  return boost::iostreams::stream_offset_to_streamoff(m_framePos * frameSize);
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize MP4FileSource::Impl::read(char* dst, std::streamsize size)
{
  UInt32 numFramesRead = 0;
  offset_t endPos      = m_streamInfo.numSampleFrames() - 1;

  auto frameSize = m_streamInfo.bytesPerSampleFrame();
  auto maxFrames = size / frameSize;

  if (size > 0 && m_framePos < endPos)
  {
    numFramesRead = std::min(UInt32(maxFrames), UInt32(endPos - m_framePos));

    AudioBufferList buffer
    {
      .mNumberBuffers = 1,
      .mBuffers[0]    =
      {
        .mNumberChannels  = UInt32(m_streamInfo.numChannels()),
        .mDataByteSize    = UInt32(m_streamInfo.bytesPerSampleFrame() * numFramesRead),
        .mData            = static_cast<void*>(dst)
      }
    };

    if (ExtAudioFileRead(m_media, &numFramesRead, &buffer) == noErr) m_framePos += numFramesRead;
    else numFramesRead = 0;
  }

  // Fill the rest with silence
  size_t numCharsRead = numFramesRead * frameSize;
  std::fill(dst + numCharsRead, dst + size, char(0));

  return numCharsRead;
}
