include(BuildGTest)

add_src_file       (FILES_main "main.cpp")
add_src_group(FILES_test_all test_main FILES_main)

add_src_file       (FILES_test_ni_media_pcm "ni/media/pcm/converter.test.cpp")
add_src_file       (FILES_test_ni_media_pcm "ni/media/pcm/iterator.test.cpp")
add_src_file       (FILES_test_ni_media_pcm "ni/media/pcm/PcmTestTypes.h")

add_src_group(FILES_test_all test_ni_media_pcm FILES_test_ni_media_pcm)


##################
# iTraktorLibTests

add_executable             ( ni-media_test  ${FILES_test_all} )
target_link_libraries      ( ni-media_test  gtest ni-media  )

add_test( ni-media_test_run  ni-media_test )

