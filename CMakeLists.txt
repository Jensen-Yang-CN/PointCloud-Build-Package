cmake_minimum_required(VERSION 3.5)

project(PointCloud LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt5 REQUIRED COMPONENTS Concurrent)

if(ANDROID)
  add_library(PointCloud SHARED
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )
else()
  add_executable(PointCloud
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    StreamRule.h
    UdpDispatcher.h
    UdpDispatcher.cpp
    PcapngReader.h
    PcapngReader.cpp
    ImuWorker.h
    ImuWorker.cpp
    BaseWorker.h
    VideoWorker.h
    VideoWorker.cpp
    RsLidarWorker.h
    RsLidarWorker.cpp
    WorkerThreadWrapper.h
    videowidget.h
    videowidget.cpp
    h265decoder.h
    h265decoder.cpp
    rtpparser.h
    rtpparser.cpp
    ffmpeg_wrapper.h
    ffmpeg_wrapper.cpp
    Decoder.h
    Decoder.cpp
    RsLidarParser.cpp
    RsLidarParser.h
    IMUParser.h
    IMUParser.cpp
    LidarPacketStruct.h
    Ls400Parser.h
    Ls400Parser.cpp
    RtpParserH264.h
    RtpParserH264.cpp
    RsLidarWidget.h
    RsLidarWidget.cpp
  )
endif()
set(FFMPEG_DIR E:/ffmpeg/ffmpeg-master-latest-win64-gpl-shared)
set(LIB_DIR    E:/ffmpeg/ffmpeg-master-latest-win64-gpl-shared)
set(NPCAP_DIR  E:/Npcap_SDK)

include_directories(${FFMPEG_DIR}/include)
include_directories(${FFMPEG_DIR}/bin)
link_directories(${LIB_DIR}/lib) #将需要的FFMPEG 的dll.a  lib文件都拷贝再此目录下

include_directories(${NPCAP_DIR}/include)

link_directories(${NPCAP_DIR}/lib)

link_directories(${CMAKE_SOURCE_DIR}/lib)


target_link_libraries(PointCloud PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Concurrent
    ${CMAKE_SOURCE_DIR}/lib/libavformat.dll.a
    ${CMAKE_SOURCE_DIR}/lib/libavcodec.dll.a
    ${CMAKE_SOURCE_DIR}/lib/libswscale.dll.a
    ${CMAKE_SOURCE_DIR}/lib/libswresample.dll.a
    ${CMAKE_SOURCE_DIR}/lib/libavutil.dll.a
    ${CMAKE_SOURCE_DIR}/lib/libwpcap.a
    ${CMAKE_SOURCE_DIR}/lib/libPacket.a
    ws2_32
 )

