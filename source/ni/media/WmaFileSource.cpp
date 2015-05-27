#include "WmaFileSource.h"

#include <ni/media/media_foundation/MfFileSource.h>

//----------------------------------------------------------------------------------------------------------------------

WmaFileSource::WmaFileSource(const std::string& path) { open(path); }

//----------------------------------------------------------------------------------------------------------------------

void WmaFileSource::open(const std::string& path)
{
  m_impl.reset(new MfFileSource(path, 0, MfFileSource::defaultOffset));
  audioStreamInfo(m_impl->audioStreamInfo());
}

void WmaFileSource::close()
{
  m_impl.reset();
  audioStreamInfo(AudioStreamInfo());
}

bool WmaFileSource::is_open() const { return m_impl != nullptr; }

//----------------------------------------------------------------------------------------------------------------------

std::streampos WmaFileSource::seek(boost::iostreams::stream_offset offset, BOOST_IOS::seekdir way)
{
  return m_impl ? m_impl->seek(offset, way) : std::streampos(0);
}

//----------------------------------------------------------------------------------------------------------------------

std::streamsize WmaFileSource::read(char_type* dst, std::streamsize size)
{
  return m_impl ? m_impl->read(dst, size) : 0;
}
