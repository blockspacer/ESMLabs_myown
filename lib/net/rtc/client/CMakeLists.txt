link_directories(../../../../3rdparty/gperftools/lib)
link_directories(../../../../3rdparty/tbb/lib)
link_directories(../../../../3rdparty/webrtc/lib/${CMAKE_BUILD_TYPE})

add_definitions(-DWEBRTC_POSIX)

add_library(ESMRTCClient SHARED 
                    source/rtc_client.cpp
                    source/esm_rtc_client.cpp)

target_include_directories(ESMRTCClient PRIVATE 
                            ../../../../3rdparty/gperftools/include 
                            ../../../../3rdparty/tbb/include
                            ../../../../3rdparty/webrtc/include
                            ../../../../base/include
                            picojson
                            include)

set_target_properties(ESMRTCClient PROPERTIES
                            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/platform/linux/lib/${CMAKE_BUILD_TYPE})
    

if(CMAKE_BUILD_TYPE MATCHES "Debug")

target_link_libraries(ESMRTCClient 
                        LINK_PRIVATE pthread
                        LINK_PRIVATE tbb
                        LINK_PRIVATE webrtc_full)

else()
                        
target_link_libraries(ESMRTCClient 
                        LINK_PRIVATE pthread
                        LINK_PRIVATE tcmalloc_minimal
                        LINK_PRIVATE tbb
                        LINK_PRIVATE webrtc_full) 

endif()                        
