cmake_minimum_required(VERSION 3.24)
project(Lowline VERSION 0.5.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(FetchContent)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt")
 set(FETCHCONTENT_SOURCE_DIR_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE")
 set(FETCHCONTENT_SOURCE_DIR_NAM "${CMAKE_CURRENT_SOURCE_DIR}/third_party/NAM")
endif()
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_Declare(nam GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git GIT_TAG 0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842 GIT_SUBMODULES Dependencies/eigen SOURCE_SUBDIR _lowline_no_cmake)
FetchContent_MakeAvailable(JUCE nam)
file(GLOB NAM_SOURCES CONFIGURE_DEPENDS "${nam_SOURCE_DIR}/NAM/*.cpp" "${nam_SOURCE_DIR}/NAM/wavenet/*.cpp")
add_library(nam_core STATIC ${NAM_SOURCES})
target_include_directories(nam_core SYSTEM PUBLIC "${nam_SOURCE_DIR}" "${nam_SOURCE_DIR}/Dependencies/eigen" "${nam_SOURCE_DIR}/Dependencies/nlohmann")
target_compile_definitions(nam_core PUBLIC NAM_SAMPLE_FLOAT NAM_ENABLE_A2_FAST)
if(MSVC)
 target_compile_definitions(nam_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
 target_compile_options(nam_core PRIVATE /bigobj)
endif()
set(LOWLINE_FORMATS Standalone VST3)
set(LOWLINE_PLATFORM_OPTIONS)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
 # iPad / iPhone: the app and an AUv3 (AUM, GarageBand, Loopy Pro, Cubasis...), landscape.
 # Configure with: cmake -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 (see scripts/ios-testflight.sh)
 set(LOWLINE_FORMATS Standalone AUv3)
 set(LOWLINE_TESTS OFF CACHE BOOL "Build audio regression tests" FORCE)
 set(LOWLINE_BUILD_NUMBER "1" CACHE STRING "CFBundleVersion: must increase with every TestFlight upload")
 set(LOWLINE_TEAM_ID "" CACHE STRING "Apple Developer Team ID used for automatic signing")
 if(LOWLINE_TEAM_ID)
  set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${LOWLINE_TEAM_ID}")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic")
 endif()
 # Archive only the app (JUCE marks it SKIP_INSTALL=NO); helper static libraries stay out, or
 # Xcode would produce a "generic archive" that cannot be uploaded.
 set(CMAKE_XCODE_ATTRIBUTE_SKIP_INSTALL "YES")
 list(APPEND LOWLINE_PLATFORM_OPTIONS
  BUILD_VERSION "${LOWLINE_BUILD_NUMBER}"
  ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon-1024.png"
  IPHONE_SCREEN_ORIENTATIONS UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
  IPAD_SCREEN_ORIENTATIONS UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight
  REQUIRES_FULL_SCREEN TRUE
  STATUS_BAR_HIDDEN TRUE
  BACKGROUND_AUDIO_ENABLED TRUE
  FILE_SHARING_ENABLED TRUE
  DOCUMENT_BROWSER_ENABLED TRUE
  BLUETOOTH_PERMISSION_ENABLED TRUE
  BLUETOOTH_PERMISSION_TEXT "Radice connects to Bluetooth MIDI pedals and controllers."
  PLIST_TO_MERGE "<plist><dict><key>ITSAppUsesNonExemptEncryption</key><false/></dict></plist>")
elseif(APPLE)
 list(APPEND LOWLINE_FORMATS AU)
 list(APPEND LOWLINE_PLATFORM_OPTIONS ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon-1024.png")
endif()
# The product is called Radice (Lowline until 0.4). The CMake target, bundle ID and the
# manufacturer/plugin codes keep their original values on purpose: DAWs identify the plugin by
# those codes, so existing sessions and presets keep loading after the rename.
juce_add_plugin(Lowline COMPANY_NAME "Massimo Danieli" BUNDLE_ID "com.massimodanieli.lowline" PLUGIN_MANUFACTURER_CODE MsDn PLUGIN_CODE LwLn FORMATS ${LOWLINE_FORMATS} PRODUCT_NAME "Radice" IS_SYNTH FALSE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE MICROPHONE_PERMISSION_ENABLED TRUE MICROPHONE_PERMISSION_TEXT "Radice uses your audio interface input to process your instrument." VST3_CATEGORIES Fx Distortion ${LOWLINE_PLATFORM_OPTIONS})
target_sources(Lowline PRIVATE Source/Processor.cpp Source/Editor.cpp Source/Workbench.cpp)
juce_add_binary_data(LowlineHelp HEADER_NAME LowlineHelpData.h NAMESPACE LowlineHelpData
 SOURCES docs/HELP_IT.md docs/HELP_EN.md)
target_compile_definitions(Lowline PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(Lowline PRIVATE LowlineHelp "$<LINK_LIBRARY:WHOLE_ARCHIVE,nam_core>" juce::juce_audio_utils juce::juce_dsp juce::juce_cryptography PUBLIC juce::juce_recommended_config_flags)
option(LOWLINE_TESTS "Build audio regression tests" ON)
if(LOWLINE_TESTS)
 enable_testing()
 juce_add_console_app(LowlineTests PRODUCT_NAME "LowlineTests")
 target_sources(LowlineTests PRIVATE tests/Tests.cpp)
 target_link_libraries(LowlineTests PRIVATE Lowline LowlineHelp "$<LINK_LIBRARY:WHOLE_ARCHIVE,nam_core>" juce::juce_audio_utils juce::juce_dsp juce::juce_cryptography)
 target_compile_definitions(LowlineTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
 add_test(NAME lowline_audio COMMAND LowlineTests)
endif()
