#include <game/game_application.h>
#include <core/io/path.h>
#include <imgui.h>
#include <windows.h>
#include <shellapi.h>

namespace lumen {

Error GameApplication::on_init()
{
    using enum Error;

    // Error err = project_load(Paths::executable_dir());
    Error err = project_load("D:/TestLumen");
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    
    win32.window_set_title(project.name);
    win32.window_set_size(project.settings.width, project.settings.height);
    
    return Ok;
}

void GameApplication::on_update(float p_dt)
{
    (void)p_dt;
    renderer.request_size(win32.window.width, win32.window.height);
}

void GameApplication::on_shutdown() {}

}