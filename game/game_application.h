
#pragma once
#include <core/application/application.h>
#include <core/rendering/render_path/game_render_path.h>

namespace lumen {

struct GameApplication : Application
{
    Error on_init() override;
    void on_update(float p_dt) override;
    void on_shutdown() override;

    bool wants_docking() const override { return false; }
    RenderPath* GameApplication::create_render_path() { return new GameRenderPath(); }
};

}