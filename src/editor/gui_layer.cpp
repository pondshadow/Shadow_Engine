#include "gui_layer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> // 需要 GLFW 定义

void GuiLayer::init(void* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // 强制转换 void* 为 GLFWwindow*
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GuiLayer::begin_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GuiLayer::end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// 这里是所有 UI 控件的聚集地
void GuiLayer::render_panel(glm::vec3* clear_color, bool* is_mouse_locked,
                            DirLightParams* dir_light, PointLightParams* point_light, SpotLightParams* spot_light)
{
    // 创建一个窗口
    ImGui::Begin("BowieEngine Inspector");

    // --- 全局设置 ---
    ImGui::Text("Render Stats");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    // &clear_color->x 取出 glm::vec3 第一个分量的地址，ImGui 会自动处理后续的 y, z
    ImGui::ColorEdit3("Background", &clear_color->x);
    ImGui::Checkbox("Unlock Mouse (Left Alt)", is_mouse_locked);

    ImGui::Separator();

    // --- 定向光 ---
    if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable##Dir", &dir_light->enable); // ##Dir 是为了防止 ID 冲突
        ImGui::SliderFloat3("Direction", &dir_light->direction.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("Color##Dir", &dir_light->color.x);
    }

    // --- 点光源 ---
    if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable##Point", &point_light->enable);
        ImGui::ColorEdit3("Color##Point", &point_light->color.x);
        ImGui::SliderFloat("Linear", &point_light->linear, 0.001f, 1.0f);
        ImGui::SliderFloat("Quadratic", &point_light->quadratic, 0.0001f, 1.0f);
    }

    // --- 聚光灯 ---
    if (ImGui::CollapsingHeader("SpotLight", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable##Spot", &spot_light->enable);
        ImGui::ColorEdit3("Color##Spot", &spot_light->color.x);
        ImGui::SliderFloat("Inner Angle", &spot_light->cut_off, 1.0f, 45.0f);
        ImGui::SliderFloat("Outer Angle", &spot_light->outer_cut_off, spot_light->cut_off, 50.0f);
    }

    ImGui::End();
}

void GuiLayer::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}