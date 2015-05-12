#include "MP4FileSource.h"

#include <boost/predef.h>
#if BOOST_OS_WINDOWS
#include "impl/MP4FileSourceWin.h"
#elif BOOST_OS_MACOS
#include "impl/MP4FileSourceMac.h"
#endif


//----------------------------------------------------------------------------------------------------------------------

MP4FileSource::MP4FileSource(const std::string& path) { open(path); }

//----------------------------------------------------------------------------------------------------------------------

void MP4FileSource::open(const std::string& path)
{
  m_impl.reset(new Impl(path));
  audioStreamInfo(m_impl->audioStreamInfo());
}

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

std::streamsize MP4FileSource::read(char_type* dst, std::streamsize numFrames)
{
  return m_impl ? m_impl->read(dst, numFrames) : 0;
}
