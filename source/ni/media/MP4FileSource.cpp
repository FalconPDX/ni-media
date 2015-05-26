#include "Mp4FileSource.h"

#if BOOST_OS_MACOS
#include <ni/media/core_audio/CoreAudioFileSource.h>
#elif BOOST_OS_WINDOWS
#include <ni/media/media_foundation/MfFileSource.h>
#endif

//----------------------------------------------------------------------------------------------------------------------

Mp4FileSource::Mp4FileSource(const std::string& path) { open(path); }

//----------------------------------------------------------------------------------------------------------------------

void Mp4FileSource::open(const std::string& path)
{
#if BOOST_OS_MACOS
  m_impl.reset(new CoreAudioFileSource(path));
#elif BOOST_OS_WINDOWS
  m_impl.reset(new MfFileSource(path, MfFileSource::aacOfffset));
#endif
  audioStreamInfo(m_impl->audioStreamInfo());
}

void Mp4FileSource::close()
{
  m_impl.reset();
  audioStreamInfo(AudioStreamInfo());
}

bool Mp4FileSource::is_open() const { return m_impl != nullptr; }

//----------------------------------------------------------------------------------------------------------------------

std::streampos Mp4FileSource::seek(boost::iostreams::stream_offset offset, BOOST_IOS::seekdir way)
{
  return m_impl ? m_impl->seek(offset, way) : std::streampos(0);
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize Mp4FileSource::read(char_type* dst, std::streamsize size)
{
  return m_impl ? m_impl->read(dst, size) : 0;
}
