
add_library(munit "${CMAKE_CURRENT_SOURCE_DIR}/munit/munit.c")
target_include_directories(munit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/munit")

# MinGW's tmpfile() fails in some Windows/MSYS environments. MUnit uses it
# only to buffer test stderr, so disable that optional capture on Windows.
if(WIN32)
	target_compile_definitions(munit PRIVATE MUNIT_NO_BUFFER)
endif()

set(CHIAKI_UNIT_SOURCES
				main.c
				http.c
				rpcrypt.c
				gkcrypt.c
				takion.c
				seqnum.c
				keystate.c
				feedback.c
				reorderqueue.c
				fec.c
				test_log.c
				test_log.h
				bitstream.c
				regist.c)

if(NOT CHIAKI_LIB_ENABLE_MBEDTLS)
	list(APPEND CHIAKI_UNIT_SOURCES aia.c)
endif()

if(CHIAKI_ENABLE_FFMPEG_DECODER)
	list(APPEND CHIAKI_UNIT_SOURCES ffmpegdecoder.c)
endif()

add_executable(chiaki-unit ${CHIAKI_UNIT_SOURCES})
add_test(NAME unit COMMAND chiaki-unit)
add_executable(kbm-input-test kbmengine.cpp)
target_include_directories(kbm-input-test PRIVATE ../gui/include)
add_test(NAME native-input COMMAND kbm-input-test)
if(WIN32 AND CHIAKI_ENABLE_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Network Quick Qml)
    add_executable(kbm-secrets-test kbmsecrets.cpp)
    target_include_directories(kbm-secrets-test PRIVATE ../gui/include)
    target_link_libraries(kbm-secrets-test Qt6::Core crypt32)
    add_test(NAME windows-credentials COMMAND kbm-secrets-test)
    add_executable(kbm-profiles-test kbmprofiles.cpp)
    target_include_directories(kbm-profiles-test PRIVATE ../gui/include)
    target_link_libraries(kbm-profiles-test Qt6::Core)
    add_test(NAME input-profiles COMMAND kbm-profiles-test)
    add_executable(kbm-qml-test kbmqml.cpp)
    target_link_libraries(kbm-qml-test Qt6::Quick Qt6::Qml)
    target_compile_definitions(kbm-qml-test PRIVATE KBM_QML_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../gui/src/qml")
    add_test(NAME responsive-dialogs COMMAND kbm-qml-test)
    set_tests_properties(responsive-dialogs PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_executable(kbm-network-test kbmnetwork.cpp ../gui/src/jsonrequester.cpp ../gui/include/jsonrequester.h)
    set_target_properties(kbm-network-test PROPERTIES AUTOMOC ON)
    target_include_directories(kbm-network-test PRIVATE ../gui/include)
    target_link_libraries(kbm-network-test Qt6::Core Qt6::Network)
    add_test(NAME network-error-redaction COMMAND kbm-network-test)
    add_executable(kbm-av-test kbmav.cpp)
    target_link_libraries(kbm-av-test chiaki-lib SDL2::SDL2 FFMPEG::avcodec FFMPEG::avutil)
    target_include_directories(kbm-av-test PRIVATE ${Opus_INCLUDE_DIRS})
    add_test(NAME native-av-initialization COMMAND kbm-av-test)
endif()

target_link_libraries(chiaki-unit chiaki-lib munit)

if(CHIAKI_ENABLE_FFMPEG_DECODER)
	target_link_libraries(chiaki-unit FFMPEG::avcodec FFMPEG::avutil)
endif()
