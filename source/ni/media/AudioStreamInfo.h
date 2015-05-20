//!---------------------------------------------------------------------------------------------------------------------
//!
//! \author Marc Boucek
//! \date Oct/2013
//!
//! \class AudioStreamInfo
//!
//! \brief Audio Stream Description.
//!
//! \copyright NATIVE INSTRUMENTS, Berlin, Germany, ALL RIGHTS RESERVED
//!
//!--------------------------------------------------------------------------------------------------------------------
#pragma once

#include <ni/media/pcm/format.h>

//----------------------------------------------------------------------------------------------------------------------
// audio stream base class
//----------------------------------------------------------------------------------------------------------------------

class AudioStreamInfo
{

public:
    using format_t = ::pcm::format;

    void format( format_t format )
    {
      m_format = format;
    }

    format_t format() const
    {
      return m_format;
    }

    void sampleRate( size_t val )
    {
        m_sampleRate = val;
    }

    size_t sampleRate() const
    {
        return m_sampleRate;
    }

    void numChannels(size_t numChannels)
    {
      m_numChannels = numChannels;
    }

    size_t numChannels() const
    {
      return m_numChannels;
    }

    void numSampleFrames( size_t numSampleFrames )
    {
      m_numSampleFrames = numSampleFrames;
    }

    size_t numSampleFrames() const
    {
      return m_numSampleFrames;
    }

    size_t numSamples() const
    {
      return numSampleFrames() * numChannels();
    }
    size_t numBytes() const
    {
      return numSamples() * bytesPerSample();
    }

    size_t bitsPerSample() const
    {
        return m_format.getBitwidth();
    }

    size_t bytesPerSample() const
    {
        return bitsPerSample() / 8;
    }

    size_t bytesPerSampleFrame() const
    {
        return bytesPerSample() * numChannels();
    }

private:

    // Members

    size_t          m_sampleRate      = { 0 }; //!< sample rate
    size_t          m_numChannels     = { 0 }; //!< number of channels
    size_t          m_numSampleFrames = { 0 }; //!< number of sampleFrames
    format_t        m_format;

};



//----------------------------------------------------------------------------------------------------------------------
