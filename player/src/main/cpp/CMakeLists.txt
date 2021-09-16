cmake_minimum_required(VERSION 3.10.2)

# 设置路径
set(FFMPEG ${CMAKE_SOURCE_DIR}/ffmpeg)
set(RTMP ${CMAKE_SOURCE_DIR}/rtmp)

# 导入头文件
include_directories(FFMPEG/include)

# 导入库文件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${FFMPEG}/libs/${CMAKE_ANDROID_ARCH_ABI}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${RTMP}/libs/${CMAKE_ANDROID_ARCH_ABI}")
# 批量导入
file(GLOB src_files *.cpp)

add_library(
        player
        SHARED
        ${src_files})

# 链接库
target_link_libraries(
        player
        # 批量导入，忽略顺序
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group

        rtmp
        log
        z
        android
        OpenSLES
)