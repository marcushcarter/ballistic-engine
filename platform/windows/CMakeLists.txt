




function(lumen_common_target target)
    target_include_directories(${target} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated
    )
    target_link_options(${target} PRIVATE /MANIFEST:NO)
endfunction()

set(LUMEN_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/lumen_main.cpp)
set(LUMEN_WRAPPER ${CMAKE_CURRENT_SOURCE_DIR}/lumen_console_wrapper.cpp)

if(LUMEN_BUILD_EDITOR)
    add_executable(lumen_editor_app WIN32
        lumen_res.rc
        ${LUMEN_SHARED_ASSETS}
        ${LUMEN_EDITOR_ASSETS}
        ${LUMEN_MAIN}
    )
    target_link_libraries(lumen_editor_app PRIVATE lumen_core lumen_editor)
    target_compile_definitions(lumen_editor_app PRIVATE LUMEN_EDITOR LUMEN_DEV_TOOLS)
    lumen_common_target(lumen_editor_app)
endif()

if(LUMEN_BUILD_CONSOLE_WRAPPER)
    add_executable(lumen_console_wrapper
        lumen_res_console.rc
        ${LUMEN_WRAPPER}
    )
    target_compile_definitions(lumen_console_wrapper PRIVATE "LUMEN_WRAPPER_TARGET=L\"lumen_editor_app.exe\"")
    lumen_common_target(lumen_console_wrapper)
endif()

if(LUMEN_BUILD_GAME)
    add_executable(lumen_game WIN32
        lumen_res_app.rc
        ${LUMEN_SHARED_ASSETS}
        # main2.cpp
        ${LUMEN_MAIN} 
        ${CMAKE_SOURCE_DIR}/game/game_application.cpp
    )
    target_link_libraries(lumen_game PRIVATE lumen_core)
    target_compile_definitions(lumen_game PRIVATE LUMEN_GAME)
    lumen_common_target(lumen_game)
endif()