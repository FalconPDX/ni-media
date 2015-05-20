find_package(niboost)


add_subdirectory(ni/media/alac)

#--------------------------------------------------------------------
# sources
#--------------------------------------------------------------------

add_src_file    (FILES_media "ni/media/WAVChunks.h"                           )
add_src_file    (FILES_media "ni/media/AudioFileSource.h"                     )
add_src_file    (FILES_media "ni/media/AudioStreamInfo.h"                     )
add_src_file    (FILES_media "ni/media/CAFFileDescription.h")
add_src_file    (FILES_media "ni/media/CAFFileSource.cpp"          WITH_HEADER)
add_src_file    (FILES_media "ni/media/AiffFileSource.cpp"         WITH_HEADER)
add_src_file    (FILES_media "ni/media/MP4FileSource.cpp"          WITH_HEADER)
add_src_file    (FILES_media "ni/media/WavFileSource.cpp"          WITH_HEADER)
add_src_file_mac(FILES_media "ni/media/impl/MP4FileSourceMac.cpp"  WITH_HEADER)
add_src_file_win(FILES_media "ni/media/impl/MP4FileSourceWin.cpp"  WITH_HEADER)
add_src_group   (FILES_All media FILES_media)

add_src_file    (FILES_media_pcm "ni/media/pcm/converter.h"                           )
add_src_file    (FILES_media_pcm "ni/media/pcm/format.h"                     )
add_src_file    (FILES_media_pcm "ni/media/pcm/iterator.h"                     )
add_src_file    (FILES_media_pcm "ni/media/pcm/traits.h")
add_src_group   (FILES_All media_pcm FILES_media_pcm)

#--------------------------------------------------------------------
# linking
#--------------------------------------------------------------------

add_library                 ( ni-media EXCLUDE_FROM_ALL ${FILES_All} )
target_include_directories  ( ni-media PUBLIC . )
target_link_libraries       ( ni-media boost_header_only_interface alac)


