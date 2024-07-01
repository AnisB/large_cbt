// Windows include
#include <Windows.h>

// Graphics API include
#include "graphics/dx12_backend.h"

// Project incldues
#include "render_pipeline/space_renderer.h"

int CALLBACK main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    // Path of the exe
    std::string exePath(__argv[0]);
    uint32_t loc = (uint32_t)exePath.find_last_of('\\');

    // Evaluate the project directory
    std::string projectDir = exePath.substr(0, loc);

    // If no project directory was specified, we take the exe directory as a project dir
    if (__argc == 2)
        projectDir = __argv[1];

    // Create the renderer
    SpaceRenderer spaceRenderer;

    // Initalize the renderer
    spaceRenderer.initialize(hInstance, projectDir.c_str());

    // Trigger the renderer loop
    spaceRenderer.render_loop();

    // Release all the resources
    spaceRenderer.release();

    // We're done
    return 0;
}