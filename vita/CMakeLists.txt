cmake_minimum_required(VERSION 3.12)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(streamflix-vita C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fno-lto")

set(VITA_APP_NAME "StreamFlix")
set(VITA_TITLEID  "STFX00001")
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

set(SRC_FILES
  src/main.c
  src/net/http_client.c
  src/net/json_helpers.c
  src/net/ss_crypto.c
  src/providers/superstream.c
  src/ui/ui_common.c
  src/ui/ui_main_menu.c
  src/ui/ui_search.c
  src/ui/ui_detail.c
  src/ui/ui_player.c
  src/ui/font_pgf.c
  src/db/database.c
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME}
  vita2d
  SceDisplay_stub
  SceCtrl_stub
  SceTouch_stub
  ScePgf_stub
  SceCommonDialog_stub
  SceGxm_stub
  SceSysmodule_stub
  SceNet_stub
  SceNetCtl_stub
  SceAvPlayer_stub
  SceAudio_stub
  SceAudiodec_stub
  SceAppMgr_stub
  SceAppUtil_stub
  ScePower_stub
  SceRegistryMgr_stub
  curl
  ssl
  crypto
  jansson
  freetype
  png
  jpeg
  z
  m
  c
  pthread
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)

# NOTE: Add these back when icon/livearea images are ready:
# FILE sce_sys/icon0.png sce_sys/icon0.png
# FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
# FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
