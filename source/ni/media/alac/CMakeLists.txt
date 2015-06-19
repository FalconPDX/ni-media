##  --------------------------------------------------------------------------------------
##  Source Files
##  --------------------------------------------------------------------------------------

add_src_file  (FILES_codec "codec/aglib.h")
add_src_file  (FILES_codec "codec/dplib.h")
add_src_file  (FILES_codec "codec/matrixlib.h")

add_src_file ( FILES_codec "codec/ag_dec.c" )
add_src_file ( FILES_codec "codec/ag_enc.c" )
add_src_file ( FILES_codec "codec/dp_dec.c" )
add_src_file ( FILES_codec "codec/dp_enc.c" )
add_src_file ( FILES_codec "codec/matrix_dec.c" )
add_src_file ( FILES_codec "codec/matrix_enc.c" )

add_src_file  (FILES_codec "codec/ALACAudioTypes.h")
add_src_file  (FILES_codec "codec/EndianPortable.c" WITH_HEADER)
add_src_file  (FILES_codec "codec/ALACBitUtilities.c" WITH_HEADER)
add_src_file  (FILES_codec "codec/ALACDecoder.cpp" WITH_HEADER)
add_src_file  (FILES_codec "codec/ALACEncoder.cpp" WITH_HEADER)
add_src_group(FILES_All codec FILES_codec)

##  --------------------------------------------------------------------------------------
##   Create static library
##  --------------------------------------------------------------------------------------

add_library(alac EXCLUDE_FROM_ALL ${FILES_All})

# specify include path
target_include_directories(alac PUBLIC codec)

if(WIN32)
  target_compile_definitions( alac PRIVATE TARGET_OS_WIN32=1 )
endif()



