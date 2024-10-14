#include <stdio.h>
#include "cimgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

void test_imgui() {
    ImGuiIO* io = igGetIO();
    printf("ImGui version: %s\n", IMGUI_VERSION);
    printf("Display size: %.2f x %.2f\n", io->DisplaySize.x, io->DisplaySize.y);
}