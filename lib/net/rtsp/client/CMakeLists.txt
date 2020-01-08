link_directories(../../../../3rdparty/gperftools/lib)
link_directories(../../../../3rdparty/openssl/lib)
link_directories(../../../../3rdparty/live/lib)

add_library(MGNTRTSPClient SHARED 
                    source/aac_buffer_sink.cpp
                    source/buffer_sink.cpp
                    source/h2645_buffer_sink.cpp
                    source/h264_buffer_sink.cpp 
                    source/h265_buffer_sink.cpp  
                    source/rtsp_client.cpp
                    source/esm_rtsp_client.cpp)

target_include_directories(MGNTRTSPClient PRIVATE 
                            ../../../../3rdparty/gperftools/include 
                            ../../../../3rdparty/live/include
                            ../../../../base/include
                            include)

set_target_properties(MGNTRTSPClient PROPERTIES
                            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/platform/linux/lib/${CMAKE_BUILD_TYPE})
    

if(CMAKE_BUILD_TYPE MATCHES "Debug")

target_link_libraries(MGNTRTSPClient 
                        LINK_PRIVATE pthread
                        LINK_PRIVATE crypto
                        LINK_PRIVATE ssl
                        LINK_PRIVATE liveMedia_debug
                        LINK_PRIVATE groupsock_debug
                        LINK_PRIVATE BasicUsageEnvironment_debug
                        LINK_PRIVATE UsageEnvironment_debug)
else()
                        
target_link_libraries(MGNTRTSPClient 
                        LINK_PRIVATE pthread
                        LINK_PRIVATE tcmalloc_minimal
                        LINK_PRIVATE crypto
                        LINK_PRIVATE ssl
                        LINK_PRIVATE liveMedia
                        LINK_PRIVATE groupsock
                        LINK_PRIVATE BasicUsageEnvironment
                        LINK_PRIVATE UsageEnvironment) 

endif()                        
