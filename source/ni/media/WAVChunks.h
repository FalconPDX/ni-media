#pragma once

#include <boost/cstdint.hpp>


//----------------------------------------------------------------------------------------------------------------------
// WAV Chunks
//----------------------------------------------------------------------------------------------------------------------

struct tRiffTag
{
    boost::uint32_t nID;
    boost::uint32_t nLength;
};


//----------------------------------------------------------------------------------------------------------------------

struct tWAVSampleLoop
{
    boost::int32_t nIdentifier;
    boost::int32_t nType;
    boost::int32_t nStart;
    boost::int32_t nEnd;
    boost::int32_t nFraction;
    boost::int32_t nPlayCount;
};


//----------------------------------------------------------------------------------------------------------------------

struct tWAVSampleChunk
{
    boost::int32_t nManufacturer;
    boost::int32_t nProduct;
    boost::int32_t nSamplePeriod;
    boost::int32_t nMIDIUnityNote;
    boost::int32_t nMIDIPitchFraction;
    boost::int32_t nSMPTEFormat;
    boost::int32_t nSMPTEOffset;
    boost::int32_t nSampleLoops;
    boost::int32_t nbSamplerData;
    tWAVSampleLoop aLoops[1];
};


//----------------------------------------------------------------------------------------------------------------------

struct tWAVInstrumentChunk
{
    boost::uint8_t nRootNote;
    boost::int8_t nFineTune;
    boost::int8_t nGain;
    boost::uint8_t nLowNote;
    boost::uint8_t nHighNote;
    boost::uint8_t nLowVelocity;
    boost::uint8_t nHighVelocity;
};


//----------------------------------------------------------------------------------------------------------------------

struct tWAVFormatExtensible
{ 
	union 
	{
        boost::uint16_t nValidBitsPerSample;
        boost::uint16_t nSamplesPerBlock;   
        boost::uint16_t nReserved;
    } Samples;
    
    boost::uint32_t nChannelMask; 
    boost::uint8_t aSubFormat[16]; // a GUID in fact
    
};


//----------------------------------------------------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------------------------------------------------

enum tConst
{
    CONST_WAVE_FORMAT_PCM               = 1,
    CONST_WAVE_FORMAT_IEEE_FLOAT        = 3,
    CONST_WAVE_FORMAT_EXTENSIBLE        = 0xFFFE
};


//----------------------------------------------------------------------------------------------------------------------
