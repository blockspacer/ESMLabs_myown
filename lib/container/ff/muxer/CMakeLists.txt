link_directories(../../../../../3rdparty/gperftools/lib)
link_directories(../../../../../3rdparty/tbb/lib)
link_directories(../../../../../3rdparty/boost/lib)
link_directories(../../../../../3rdparty/ffmpeg/lib)

add_library(MGNTFFMuxer SHARED 
                    source/ff_muxer.cpp
                    source/esm_ff_muxer.cpp)

target_include_directories(MGNTFFMuxer PRIVATE 
                            ../../../../../3rdparty/gperftools/include
                            ../../../../../3rdparty/tbb/include 
                            ../../../../../3rdparty/boost/include
                            ../../../../../3rdparty/ffmpeg/include
                            ../../../../../base/include
                            include)

set_target_properties(MGNTFFMuxer PROPERTIES
                            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/platform/linux/lib/${CMAKE_BUILD_TYPE})

if(CMAKE_BUILD_TYPE MATCHES "Debug")

target_link_libraries(MGNTFFMuxer 
                        LINK_PRIVATE pthread
                        LINK_PRIVATE tbb
                        LINK_PRIVATE avcodec
                        LINK_PRIVATE avdevice
                        LINK_PRIVATE avfilter
                        LINK_PRIVATE avformat
                        LINK_PRIVATE avutil
                        LINK_PRIVATE swresample
                        LINK_PRIVATE swscale) 

else()

target_link_libraries(MGNTFFMuxer 
                        LINK_PRIVATE pthread
                        LINK_PRIVATE tcmalloc_minimal                        
                        LINK_PRIVATE tbb
                        LINK_PRIVATE avcodec
                        LINK_PRIVATE avdevice
                        LINK_PRIVATE avfilter
                        LINK_PRIVATE avformat
                        LINK_PRIVATE avutil
                        LINK_PRIVATE swresample
                        LINK_PRIVATE swscale) 


endif()
