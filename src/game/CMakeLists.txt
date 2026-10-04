cmake_minimum_required(VERSION 3.20)
project(burnout3_game C)

# Burnout 3 - recompiled game executable. main.c brings up the runtime and
# jumps to the XBE entry point; recomp_manual.c is where a function the lifter
# got wrong is replaced. Everything else is generated (gen/, gitignored).

file(GLOB RECOMP_GEN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/recomp/gen/recomp_*.c"
)

add_executable(burnout3
    main.c
    recomp/recomp_manual.c
    ${RECOMP_GEN_SOURCES}
)

target_link_libraries(burnout3 PRIVATE
    xbox_kernel xbox_d3d8 xbox_dsound xbox_input xbox_usb xbox_video xbox_apu
    kernel32 user32 gdi32 dbghelp
)

target_include_directories(burnout3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/recomp
    ${CMAKE_CURRENT_SOURCE_DIR}/recomp/gen
    ${XBOXRECOMP_DIR}/src
    ${XBOXRECOMP_DIR}/src/kernel
    # The template runtime headers, not a local copy: BO3's old copy had
    # drifted from the template by hundreds of lines.
    ${XBOXRECOMP_DIR}/templates/runtime
)

if(TARGET recomp_env)
    target_compile_definitions(burnout3 PRIVATE BO3_TOOLKIT_ENV_TABLE)
endif()

if(MSVC)
    target_compile_options(burnout3 PRIVATE /W4 /WX- /O2 /bigobj /MP${BO3_COMPILE_JOBS})
    set_source_files_properties(${RECOMP_GEN_SOURCES} PROPERTIES COMPILE_FLAGS "/w")
    target_compile_definitions(burnout3 PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Fixed image base so Xbox memory can be mapped at fixed addresses; no ICF
    # so every generated function keeps its own symbol for fault reports.
    target_link_options(burnout3 PRIVATE
        /DYNAMICBASE:NO /FIXED:NO /MAP /DEBUG /STACK:16777216 /OPT:NOICF /OPT:REF)
endif()

set_target_properties(burnout3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin"
)
