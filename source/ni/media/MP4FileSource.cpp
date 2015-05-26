#include "MP4FileSource.h"

//----------------------------------------------------------------------------------------------------------------------

#if BOOST_OS_MACOS
#include "impl/MP4FileSourceMac.h"

MP4FileSource::MP4FileSource(const std::string& path) { open(path); }

void MP4FileSource::open(const std::string& path)
{
  m_impl.reset(new Impl(path));
  audioStreamInfo(m_impl->audioStreamInfo());
}
#elif BOOST_OS_WINDOWS
#include "impl/MP4FileSourceWin.h"

MP4FileSource::MP4FileSource(const std::string& path, offset_type readOffset) { open(path, readOffset); }

void MP4FileSource::open(const std::string& path, offset_type readOffset)
{
  m_impl.reset(new Impl(path, readOffset));
  audioStreamInfo(m_impl->audioStreamInfo());
}
#endif

//----------------------------------------------------------------------------------------------------------------------

void MP4FileSource::close()
{
  m_impl.reset();
  audioStreamInfo(AudioStreamInfo());
}

bool MP4FileSource::is_open() const { return m_impl != nullptr; }

//----------------------------------------------------------------------------------------------------------------------

std::streampos MP4FileSource::seek(boost::iostreams::stream_offset offset, BOOST_IOS::seekdir way)
{
  return m_impl ? m_impl->seek(offset, way) : std::streampos(0);
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize MP4FileSource::read(char_type* dst, std::streamsize dstSize)
{
  return m_impl ? m_impl->read(dst, dstSize) : 0;
}
