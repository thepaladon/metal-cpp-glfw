#include "render/renderer.hpp"

#if defined(__EMSCRIPTEN__)
#include "game/aa_game_client.hpp"

#include <emscripten/emscripten.h>

namespace
{
struct WebAppState
{
    std::unique_ptr<Renderer> renderer;
    aa::AAGameClient gameClient;
    double lastTime = 0.0;
};

void webFrameTick(void *userData)
{
    WebAppState *state = static_cast<WebAppState *>(userData);
    const double now = emscripten_get_now();
    const float deltaTime = static_cast<float>((now - state->lastTime) / 1000.0);
    state->lastTime = now;
    state->gameClient.update(deltaTime);
    state->renderer->renderFrame(deltaTime);
}
} // namespace

int main()
{
    static WebAppState appState;

    appState.renderer = createRenderer();
    if (!appState.renderer || !appState.renderer->initialize())
    {
        return 1;
    }

    appState.gameClient.initialize("ws://localhost:9001");
    appState.renderer->setGameRenderCallback([&appState]() {
        appState.gameClient.renderImGui();
    });

    appState.lastTime = emscripten_get_now();
    emscripten_set_main_loop_arg(webFrameTick, &appState, 0, true);
    return 0;
}
#else
int main()
{
    return 1;
}
#endif
