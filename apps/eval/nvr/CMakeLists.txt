link_directories(../../../3rdparty/gperftools/lib)
link_directories(../../../3rdparty/tbb/lib)
link_directories(../../../3rdparty/boost/lib)
link_directories(../../../3rdparty/nanogui/lib)
link_directories(../../../platform/linux/lib/${CMAKE_BUILD_TYPE})

add_executable(ESMRecorder recorder.cpp)

target_include_directories(ESMRecorder PRIVATE 
                            ../../../3rdparty/gperftools/include
                            ../../../3rdparty/tbb/include 
                            ../../../3rdparty/boost/include
                            ../../../3rdparty/nanogui/include
                            ../../../base/include
                            ../../../lib/net/rtsp/client/include
                            ../../../lib/container/ff/muxer/include)

set_target_properties(ESMRecorder PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/platform/linux/apps/eval/nvr/${CMAKE_BUILD_TYPE})

add_custom_command(TARGET ESMRecorder
                    POST_BUILD 
                    COMMAND cp -r "${PROJECT_SOURCE_DIR}/3rdparty/tbb/lib/*.so*" 
                    "${PROJECT_SOURCE_DIR}/3rdparty/gperftools/lib/*.so*" 
                    "${PROJECT_SOURCE_DIR}/3rdparty/openssl/lib/*.so*"
                    "${PROJECT_SOURCE_DIR}/3rdparty/ffmpeg/lib/*.so*"
                    "${PROJECT_SOURCE_DIR}/3rdparty/nanogui/lib/*.so*"
                    "${PROJECT_SOURCE_DIR}/platform/linux/lib/${CMAKE_BUILD_TYPE}/libESMRTSPClient.so" 
                    "${PROJECT_SOURCE_DIR}/platform/linux/lib/${CMAKE_BUILD_TYPE}/libESMFFMuxer.so" 
                    ${PROJECT_SOURCE_DIR}/platform/linux/apps/eval/nvr/${CMAKE_BUILD_TYPE})                        
    

if(CMAKE_BUILD_TYPE MATCHES "Debug")

target_link_libraries(ESMRecorder PUBLIC 
                        pthread
                        tbb
                        nanogui
                        MGNTRTSPClient
                        MGNTFFMuxer
                        ${CMAKE_DL_LIBS}) 

else()

target_link_libraries(ESMRecorder PUBLIC
                        pthread
                        tcmalloc_minimal                        
                        tbb
                        nanogui
                        MGNTRTSPClient
                        MGNTFFMuxer                        
                        ${CMAKE_DL_LIBS}) 


endif()
