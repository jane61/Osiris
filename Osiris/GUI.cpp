#include <fstream>
#include <functional>
#include <string>
#include <ShlObj.h>
#include <Windows.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_stdlib.h"

#include "imguiCustom.h"

#include "GUI.h"
#include "Config.h"
#include "Hacks/Misc.h"
#include "Hacks/Reportbot.h"
#include "Hacks/SkinChanger.h"
#include "Hacks/Visuals.h"
#include "Hooks.h"
#include "SDK/InputSystem.h"

constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

GUI::GUI() noexcept
{
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(FindWindowW(L"Valve001", NULL));

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 9.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);

        static constexpr ImWchar ranges[]{ 0x0020, 0xFFFF, 0 };
        fonts.tahoma = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, nullptr, ranges);
        fonts.segoeui = io.Fonts->AddFontFromFileTTF((path / "segoeui.ttf").string().c_str(), 15.0f, nullptr, ranges);
    }
}

void GUI::render() noexcept
{
    if (!config->style.menuStyle) {
        renderMenuBar();
        renderAimbotWindow();
        renderAntiAimWindow();
        renderTriggerbotWindow();
        renderBacktrackWindow();
        renderGlowWindow();
        renderChamsWindow();
        renderEspWindow();
        renderVisualsWindow();
        renderSkinChangerWindow();
        renderSoundWindow();
        renderStyleWindow();
        renderMiscWindow();
        renderReportbotWindow();
        renderConfigWindow();
    } else {
        renderGuiStyle2();
    }
}

void GUI::updateColors() const noexcept
{
    switch (config->style.menuColors) {
    case 0: ImGui::StyleColorsDark(); break;
    case 1: ImGui::StyleColorsLight(); break;
    case 2: ImGui::StyleColorsClassic(); break;
    }
}

void GUI::hotkey(int& key) noexcept
{
    key ? ImGui::Text("[ %s ]", interfaces->inputSystem->virtualKeyToString(key)) : ImGui::TextUnformatted("[ key ]");

    if (!ImGui::IsItemHovered())
        return;

    ImGui::SetTooltip("Press any key to change keybind");
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
        if (ImGui::IsKeyPressed(i) && i != config->misc.menuKey)
            key = i != VK_ESCAPE ? i : 0;

    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
        if (ImGui::IsMouseDown(i) && i + (i > 1 ? 2 : 1) != config->misc.menuKey)
            key = i + (i > 1 ? 2 : 1);
}

void GUI::renderMenuBar() noexcept
{
    if (ImGui::BeginMainMenuBar()) {
        ImGui::MenuItem("Aimbot", nullptr, &window.aimbot);
        ImGui::MenuItem("Anti aim", nullptr, &window.antiAim);
        ImGui::MenuItem("Triggerbot", nullptr, &window.triggerbot);
        ImGui::MenuItem("Backtrack", nullptr, &window.backtrack);
        ImGui::MenuItem("Glow", nullptr, &window.glow);
        ImGui::MenuItem("Chams", nullptr, &window.chams);
        ImGui::MenuItem("Esp", nullptr, &window.esp);
        ImGui::MenuItem("Visuals", nullptr, &window.visuals);
        ImGui::MenuItem("Skin changer", nullptr, &window.skinChanger);
        ImGui::MenuItem("Sound", nullptr, &window.sound);
        ImGui::MenuItem("Style", nullptr, &window.style);
        ImGui::MenuItem("Misc", nullptr, &window.misc);
        ImGui::MenuItem("Reportbot", nullptr, &window.reportbot);
        ImGui::MenuItem("Config", nullptr, &window.config);
        ImGui::EndMainMenuBar();
    }
}

void GUI::renderAimbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.aimbot)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("Aimbot", &window.aimbot, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->aimbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 220.0f);
    ImGui::Checkbox("On key", &config->aimbot[currentWeapon].onKey);
    ImGui::SameLine();
    hotkey(config->aimbot[currentWeapon].key);
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushItemWidth(70.0f);
    ImGui::Combo("", &config->aimbot[currentWeapon].keyMode, "Hold\0Toggle\0");
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::Checkbox("Aimlock", &config->aimbot[currentWeapon].aimlock);
    ImGui::Checkbox("Silent", &config->aimbot[currentWeapon].silent);
    ImGui::Checkbox("Friendly fire", &config->aimbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Visible only", &config->aimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->aimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->aimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->aimbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Auto shot", &config->aimbot[currentWeapon].autoShot);
    ImGui::Checkbox("Auto scope", &config->aimbot[currentWeapon].autoScope);
    ImGui::Combo("Bone", &config->aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(240.0f);
    ImGui::SliderFloat("Fov", &config->aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", 2.5f);
    ImGui::SliderFloat("Smooth", &config->aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Max aim inaccuracy", &config->aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", 2.0f);
    ImGui::SliderFloat("Max shot inaccuracy", &config->aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", 2.0f);
    ImGui::InputInt("Min damage", &config->aimbot[currentWeapon].minDamage);
    config->aimbot[currentWeapon].minDamage = std::clamp(config->aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Between shots", &config->aimbot[currentWeapon].betweenShots);
    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderAntiAimWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.antiAim)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Anti aim", &window.antiAim, windowFlags);
    }
    ImGui::Checkbox("Enabled", &config->antiAim.enabled);
    ImGui::Checkbox("##pitch", &config->antiAim.pitch);
    ImGui::SameLine();
    ImGui::SliderFloat("Pitch", &config->antiAim.pitchAngle, -89.0f, 89.0f, "%.2f");
    ImGui::Checkbox("Yaw", &config->antiAim.yaw);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderTriggerbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.triggerbot)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Triggerbot", &window.triggerbot, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0Zeus x27\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 5:
        currentWeapon = 39;
        ImGui::NewLine();
        break;

    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->triggerbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::Checkbox("On key", &config->triggerbot[currentWeapon].onKey);
    ImGui::SameLine();
    hotkey(config->triggerbot[currentWeapon].key);
    ImGui::Checkbox("Friendly fire", &config->triggerbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Scoped only", &config->triggerbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->triggerbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->triggerbot[currentWeapon].ignoreSmoke);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::Combo("Hitgroup", &config->triggerbot[currentWeapon].hitgroup, "All\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Shot delay", &config->triggerbot[currentWeapon].shotDelay, 0, 250, "%d ms");
    ImGui::InputInt("Min damage", &config->triggerbot[currentWeapon].minDamage);
    config->triggerbot[currentWeapon].minDamage = std::clamp(config->triggerbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->triggerbot[currentWeapon].killshot);
    ImGui::SliderFloat("Burst Time", &config->triggerbot[currentWeapon].burstTime, 0.0f, 0.5f, "%.3f s");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderBacktrackWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.backtrack)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Backtrack", &window.backtrack, windowFlags);
    }
    ImGui::Checkbox("Enabled", &config->backtrack.enabled);
    ImGui::Checkbox("Ignore smoke", &config->backtrack.ignoreSmoke);
    ImGui::Checkbox("Recoil based fov", &config->backtrack.recoilBasedFov);
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Time limit", &config->backtrack.timeLimit, 1, 200, "%d ms");
    ImGui::PopItemWidth();
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderGlowWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.glow)
            return;
        ImGui::SetNextWindowSize({ 450.0f, 0.0f });
        ImGui::Begin("Glow", &window.glow, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Weapons\0C4\0Planted C4\0Chickens\0Defuse kits\0Projectiles\0Hostages\0Ragdolls\0");
    ImGui::PopID();
    static int currentItem{ 0 };
    if (currentCategory <= 3) {
        ImGui::SameLine();
        static int currentType{ 0 };
        ImGui::PushID(1);
        ImGui::Combo("", &currentType, "All\0Visible\0Occluded\0");
        ImGui::PopID();
        currentItem = currentCategory * 3 + currentType;
    } else {
        currentItem = currentCategory + 8;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->glow[currentItem].enabled);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 150.0f);
    ImGui::Checkbox("Health based", &config->glow[currentItem].healthBased);

    ImGuiCustom::colorPicker("Color", config->glow[currentItem].color.color, nullptr, &config->glow[currentItem].color.rainbow, &config->glow[currentItem].color.rainbowSpeed);

    ImGui::NextColumn();
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderFloat("Thickness", &config->glow[currentItem].thickness, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Alpha", &config->glow[currentItem].alpha, 0.0f, 1.0f, "%.2f");
    ImGui::SliderInt("Style", &config->glow[currentItem].style, 0, 3);
    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderChamsWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.chams)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Chams", &window.chams, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Weapons\0Hands\0Backtrack\0Sleeves\0");
    ImGui::PopID();
    static int currentItem{ 0 };

    if (currentCategory <= 3) {
        ImGui::SameLine();
        static int currentType{ 0 };
        ImGui::PushID(1);
        ImGui::Combo("", &currentType, "All\0Visible\0Occluded\0");
        ImGui::PopID();
        currentItem = currentCategory * 3 + currentType;
    } else {
        currentItem = currentCategory + 8;
    }

    ImGui::SameLine();
    static int material = 1;

    if (ImGui::ArrowButton("##left", ImGuiDir_Left) && material > 1)
        --material;
    ImGui::SameLine();
    ImGui::Text("%d", material);
    ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right) && material < int(config->chams[0].materials.size()))
        ++material;

    ImGui::SameLine();
    auto& chams{ config->chams[currentItem].materials[material - 1] };

    ImGui::Checkbox("Enabled", &chams.enabled);
    ImGui::Separator();
    ImGui::Checkbox("Health based", &chams.healthBased);
    ImGui::Checkbox("Blinking", &chams.blinking);
    ImGui::Combo("Material", &chams.material, "Normal\0Flat\0Animated\0Platinum\0Glass\0Chrome\0Crystal\0Silver\0Gold\0Plastic\0Glow\0");
    ImGui::Checkbox("Wireframe", &chams.wireframe);
    ImGuiCustom::colorPicker("Color", chams.color.color, nullptr, &chams.color.rainbow, &chams.color.rainbowSpeed);
    ImGui::SetNextItemWidth(220.0f);
    ImGui::SliderFloat("Alpha", &chams.alpha, 0.0f, 1.0f, "%.2f");

    if (!contentOnly) {
        ImGui::End();
    }
}

void GUI::renderEspWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.esp)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Esp", &window.esp, windowFlags);
    }

    static int currentCategory = 0;
    static int currentItem = 0;

    if (ImGui::ListBoxHeader("##", { 125.0f, 300.0f })) {
        static constexpr const char* players[]{ "All", "Visible", "Occluded" };

        ImGui::Text("Allies");
        ImGui::Indent();
        ImGui::PushID("Allies");
        ImGui::PushFont(fonts.segoeui);

        for (int i = 0; i < IM_ARRAYSIZE(players); i++) {
            bool isSelected = currentCategory == 0 && currentItem == i;

            if ((i == 0 || !config->esp.players[0].enabled) && ImGui::Selectable(players[i], isSelected)) {
                currentItem = i;
                currentCategory = 0;
            }
        }

        ImGui::PopFont();
        ImGui::PopID();
        ImGui::Unindent();
        ImGui::Text("Enemies");
        ImGui::Indent();
        ImGui::PushID("Enemies");
        ImGui::PushFont(fonts.segoeui);

        for (int i = 0; i < IM_ARRAYSIZE(players); i++) {
            bool isSelected = currentCategory == 1 && currentItem == i;

            if ((i == 0 || !config->esp.players[3].enabled) && ImGui::Selectable(players[i], isSelected)) {
                currentItem = i;
                currentCategory = 1;
            }
        }

        ImGui::PopFont();
        ImGui::PopID();
        ImGui::Unindent();
        if (bool isSelected = currentCategory == 2; ImGui::Selectable("Weapons", isSelected))
            currentCategory = 2;

        ImGui::Text("Projectiles");
        ImGui::Indent();
        ImGui::PushID("Projectiles");
        ImGui::PushFont(fonts.segoeui);
        static constexpr const char* projectiles[]{ "Flashbang", "HE Grenade", "Breach Charge", "Bump Mine", "Decoy Grenade", "Molotov", "TA Grenade", "Smoke Grenade", "Snowball" };

        for (int i = 0; i < IM_ARRAYSIZE(projectiles); i++) {
            bool isSelected = currentCategory == 3 && currentItem == i;

            if (ImGui::Selectable(projectiles[i], isSelected)) {
                currentItem = i;
                currentCategory = 3;
            }
        }

        ImGui::PopFont();
        ImGui::PopID();
        ImGui::Unindent();

        ImGui::Text("Danger Zone");
        ImGui::Indent();
        ImGui::PushID("Danger Zone");
        ImGui::PushFont(fonts.segoeui);
        static constexpr const char* dangerZone[]{ "Sentries", "Drones", "Cash", "Cash Dufflebag", "Pistol Case", "Light Case", "Heavy Case", "Explosive Case", "Tools Case", "Full Armor", "Armor", "Helmet", "Parachute", "Briefcase", "Tablet Upgrade", "ExoJump", "Ammobox", "Radar Jammer" };

        for (int i = 0; i < IM_ARRAYSIZE(dangerZone); i++) {
            bool isSelected = currentCategory == 4 && currentItem == i;

            if (ImGui::Selectable(dangerZone[i], isSelected)) {
                currentItem = i;
                currentCategory = 4;
            }
        }

        ImGui::PopFont();
        ImGui::PopID();
        ImGui::ListBoxFooter();
    }
    ImGui::SameLine();
    if (ImGui::BeginChild("##child", { 400.0f, 0.0f })) {
        switch (currentCategory) {
        case 0:
        case 1: {
            int selected = currentCategory * 3 + currentItem;
            ImGui::Checkbox("Enabled", &config->esp.players[selected].enabled);
            ImGui::SameLine(0.0f, 50.0f);
            ImGui::SetNextItemWidth(85.0f);
            ImGui::InputInt("Font", &config->esp.players[selected].font, 1, 294);
            config->esp.players[selected].font = std::clamp(config->esp.players[selected].font, 1, 294);

            ImGui::Separator();

            constexpr auto spacing{ 200.0f };
            ImGuiCustom::colorPicker("Snaplines", config->esp.players[selected].snaplines);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Box", config->esp.players[selected].box);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("", &config->esp.players[selected].boxType, "2D\0""2D corners\0""3D\0""3D corners\0");
            ImGuiCustom::colorPicker("Eye traces", config->esp.players[selected].eyeTraces);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Health", config->esp.players[selected].health);
            ImGuiCustom::colorPicker("Head dot", config->esp.players[selected].headDot);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Health bar", config->esp.players[selected].healthBar);
            ImGuiCustom::colorPicker("Name", config->esp.players[selected].name);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Armor", config->esp.players[selected].armor);
            ImGuiCustom::colorPicker("Money", config->esp.players[selected].money);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Armor bar", config->esp.players[selected].armorBar);
            ImGuiCustom::colorPicker("Outline", config->esp.players[selected].outline);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Distance", config->esp.players[selected].distance);
            ImGuiCustom::colorPicker("Active Weapon", config->esp.players[selected].activeWeapon);
            ImGui::SameLine(spacing);
            ImGui::Checkbox("Dead ESP", &config->esp.players[selected].deadesp);
            ImGui::SliderFloat("Max distance", &config->esp.players[selected].maxDistance, 0.0f, 200.0f, "%.2fm");
            break;
        }
        case 2: {
            ImGui::Checkbox("Enabled", &config->esp.weapon.enabled);
            ImGui::SameLine(0.0f, 50.0f);
            ImGui::SetNextItemWidth(85.0f);
            ImGui::InputInt("Font", &config->esp.weapon.font, 1, 294);
            config->esp.weapon.font = std::clamp(config->esp.weapon.font, 1, 294);

            ImGui::Separator();

            constexpr auto spacing{ 200.0f };
            ImGuiCustom::colorPicker("Snaplines", config->esp.weapon.snaplines);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Box", config->esp.weapon.box);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("", &config->esp.weapon.boxType, "2D\0""2D corners\0""3D\0""3D corners\0");
            ImGuiCustom::colorPicker("Name", config->esp.weapon.name);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Outline", config->esp.weapon.outline);
            ImGuiCustom::colorPicker("Distance", config->esp.weapon.distance);
            ImGui::SliderFloat("Max distance", &config->esp.weapon.maxDistance, 0.0f, 200.0f, "%.2fm");
            break;
        }
        case 3: {
            ImGui::Checkbox("Enabled", &config->esp.projectiles[currentItem].enabled);
            ImGui::SameLine(0.0f, 50.0f);
            ImGui::SetNextItemWidth(85.0f);
            ImGui::InputInt("Font", &config->esp.projectiles[currentItem].font, 1, 294);
            config->esp.projectiles[currentItem].font = std::clamp(config->esp.projectiles[currentItem].font, 1, 294);

            ImGui::Separator();

            constexpr auto spacing{ 200.0f };
            ImGuiCustom::colorPicker("Snaplines", config->esp.projectiles[currentItem].snaplines);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Box", config->esp.projectiles[currentItem].box);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("", &config->esp.projectiles[currentItem].boxType, "2D\0""2D corners\0""3D\0""3D corners\0");
            ImGuiCustom::colorPicker("Name", config->esp.projectiles[currentItem].name);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Outline", config->esp.projectiles[currentItem].outline);
            ImGuiCustom::colorPicker("Distance", config->esp.projectiles[currentItem].distance);
            ImGui::SliderFloat("Max distance", &config->esp.projectiles[currentItem].maxDistance, 0.0f, 200.0f, "%.2fm");
            break;
        }
        case 4: {
            int selected = currentItem;
            ImGui::Checkbox("Enabled", &config->esp.dangerZone[selected].enabled);
            ImGui::SameLine(0.0f, 50.0f);
            ImGui::SetNextItemWidth(85.0f);
            ImGui::InputInt("Font", &config->esp.dangerZone[selected].font, 1, 294);
            config->esp.dangerZone[selected].font = std::clamp(config->esp.dangerZone[selected].font, 1, 294);

            ImGui::Separator();

            constexpr auto spacing{ 200.0f };
            ImGuiCustom::colorPicker("Snaplines", config->esp.dangerZone[selected].snaplines);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Box", config->esp.dangerZone[selected].box);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("", &config->esp.dangerZone[selected].boxType, "2D\0""2D corners\0""3D\0""3D corners\0");
            ImGuiCustom::colorPicker("Name", config->esp.dangerZone[selected].name);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Outline", config->esp.dangerZone[selected].outline);
            ImGuiCustom::colorPicker("Distance", config->esp.dangerZone[selected].distance);
            ImGui::SliderFloat("Max distance", &config->esp.dangerZone[selected].maxDistance, 0.0f, 200.0f, "%.2fm");
            break;
        }
        }

        ImGui::EndChild();
    }

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderVisualsWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.visuals)
            return;
        ImGui::SetNextWindowSize({ 680.0f, 0.0f });
        ImGui::Begin("Visuals", &window.visuals, windowFlags);
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 280.0f);
    ImGui::Combo("T Player Model", &config->visuals.playerModelT, "Default\0Special Agent Ava | FBI\0Operator | FBI SWAT\0Markus Delrow | FBI HRT\0Michael Syfers | FBI Sniper\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0Third Commando Company | KSK\0'Two Times' McCoy | USAF TACP\0Dragomir | Sabre\0Rezan The Ready | Sabre\0'The Doctor' Romanov | Sabre\0Maximus | Sabre\0Blackwolf | Sabre\0The Elite Mr. Muhlik | Elite Crew\0Ground Rebel | Elite Crew\0Osiris | Elite Crew\0Prof. Shahmat | Elite Crew\0Enforcer | Phoenix\0Slingshot | Phoenix\0Soldier | Phoenix\0");
    ImGui::Combo("CT Player Model", &config->visuals.playerModelCT, "Default\0Special Agent Ava | FBI\0Operator | FBI SWAT\0Markus Delrow | FBI HRT\0Michael Syfers | FBI Sniper\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0Third Commando Company | KSK\0'Two Times' McCoy | USAF TACP\0Dragomir | Sabre\0Rezan The Ready | Sabre\0'The Doctor' Romanov | Sabre\0Maximus | Sabre\0Blackwolf | Sabre\0The Elite Mr. Muhlik | Elite Crew\0Ground Rebel | Elite Crew\0Osiris | Elite Crew\0Prof. Shahmat | Elite Crew\0Enforcer | Phoenix\0Slingshot | Phoenix\0Soldier | Phoenix\0");
    ImGui::Checkbox("Disable post-processing", &config->visuals.disablePostProcessing);
    ImGui::Checkbox("Inverse ragdoll gravity", &config->visuals.inverseRagdollGravity);
    ImGui::Checkbox("No fog", &config->visuals.noFog);
    ImGui::Checkbox("No 3d sky", &config->visuals.no3dSky);
    ImGui::Checkbox("No aim punch", &config->visuals.noAimPunch);
    ImGui::Checkbox("No view punch", &config->visuals.noViewPunch);
    ImGui::Checkbox("No hands", &config->visuals.noHands);
    ImGui::Checkbox("No sleeves", &config->visuals.noSleeves);
    ImGui::Checkbox("No weapons", &config->visuals.noWeapons);
    ImGui::Checkbox("No smoke", &config->visuals.noSmoke);
    ImGui::Checkbox("No blur", &config->visuals.noBlur);
    ImGui::Checkbox("No scope overlay", &config->visuals.noScopeOverlay);
    ImGui::Checkbox("No grass", &config->visuals.noGrass);
    ImGui::Checkbox("No shadows", &config->visuals.noShadows);
    ImGui::Checkbox("Wireframe smoke", &config->visuals.wireframeSmoke);
    ImGui::NextColumn();
    ImGui::Checkbox("Zoom", &config->visuals.zoom);
    ImGui::SameLine();
    hotkey(config->visuals.zoomKey);
    ImGui::Checkbox("Thirdperson", &config->visuals.thirdperson);
    ImGui::SameLine();
    hotkey(config->visuals.thirdpersonKey);
    ImGui::PushItemWidth(290.0f);
    ImGui::PushID(0);
    ImGui::SliderInt("", &config->visuals.thirdpersonDistance, 0, 1000, "Thirdperson distance: %d");
    ImGui::PopID();
    ImGui::PushID(1);
    ImGui::SliderInt("", &config->visuals.viewmodelFov, -60, 60, "Viewmodel FOV: %d");
    ImGui::PopID();
    ImGui::PushID(2);
    ImGui::SliderInt("", &config->visuals.fov, -60, 60, "FOV: %d");
    ImGui::PopID();
    ImGui::PushID(3);
    ImGui::SliderInt("", &config->visuals.farZ, 0, 2000, "Far Z: %d");
    ImGui::PopID();
    ImGui::PushID(4);
    ImGui::SliderInt("", &config->visuals.flashReduction, 0, 100, "Flash reduction: %d%%");
    ImGui::PopID();
    ImGui::PushID(5);
    ImGui::SliderFloat("", &config->visuals.brightness, 0.0f, 1.0f, "Brightness: %.2f");
    ImGui::PopID();
    ImGui::PopItemWidth();
    ImGui::Combo("Skybox", &config->visuals.skybox, "Default\0cs_baggage_skybox_\0cs_tibet\0embassy\0italy\0jungle\0nukeblank\0office\0sky_cs15_daylight01_hdr\0sky_cs15_daylight02_hdr\0sky_cs15_daylight03_hdr\0sky_cs15_daylight04_hdr\0sky_csgo_cloudy01\0sky_csgo_night_flat\0sky_csgo_night02\0sky_day02_05_hdr\0sky_day02_05\0sky_dust\0sky_l4d_rural02_ldr\0sky_venice\0vertigo_hdr\0vertigo\0vertigoblue_hdr\0vietnam\0");
    ImGuiCustom::colorPicker("World color", config->visuals.world);
    ImGuiCustom::colorPicker("Sky color", config->visuals.sky);
    ImGui::Checkbox("Deagle spinner", &config->visuals.deagleSpinner);
    ImGui::Combo("Screen effect", &config->visuals.screenEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::Combo("Hit effect", &config->visuals.hitEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::SliderFloat("Hit effect time", &config->visuals.hitEffectTime, 0.1f, 1.5f, "%.2fs");
    ImGui::Combo("Hit marker", &config->visuals.hitMarker, "None\0Default (Cross)\0");
    ImGui::SliderFloat("Hit marker time", &config->visuals.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
    ImGui::Checkbox("Color correction", &config->visuals.colorCorrection.enabled);
    ImGui::SameLine();
    bool ccPopup = ImGui::Button("Edit");

    if (ccPopup)
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup")) {
        ImGui::VSliderFloat("##1", { 40.0f, 160.0f }, &config->visuals.colorCorrection.blue, 0.0f, 1.0f, "Blue\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##2", { 40.0f, 160.0f }, &config->visuals.colorCorrection.red, 0.0f, 1.0f, "Red\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##3", { 40.0f, 160.0f }, &config->visuals.colorCorrection.mono, 0.0f, 1.0f, "Mono\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##4", { 40.0f, 160.0f }, &config->visuals.colorCorrection.saturation, 0.0f, 1.0f, "Sat\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##5", { 40.0f, 160.0f }, &config->visuals.colorCorrection.ghost, 0.0f, 1.0f, "Ghost\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##6", { 40.0f, 160.0f }, &config->visuals.colorCorrection.green, 0.0f, 1.0f, "Green\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##7", { 40.0f, 160.0f }, &config->visuals.colorCorrection.yellow, 0.0f, 1.0f, "Yellow\n%.3f"); ImGui::SameLine();
        ImGui::EndPopup();
    }
    ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderSkinChangerWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.skinChanger)
            return;
        ImGui::SetNextWindowSize({ 700.0f, 0.0f });
        ImGui::Begin("nSkinz", &window.skinChanger, windowFlags);
    }

    static auto itemIndex = 0;

    ImGui::PushItemWidth(110.0f);
    ImGui::Combo("##1", &itemIndex, [](void* data, int idx, const char** out_text) {
        *out_text = game_data::weapon_names[idx].name;
        return true;
        }, nullptr, IM_ARRAYSIZE(game_data::weapon_names), 5);
    ImGui::PopItemWidth();

    auto& selected_entry = config->skinChanger[itemIndex];
    selected_entry.itemIdIndex = itemIndex;

    {
        ImGui::SameLine();
        ImGui::Checkbox("Enabled", &selected_entry.enabled);
        ImGui::Separator();
        ImGui::Columns(2, nullptr, false);
        ImGui::InputInt("Seed", &selected_entry.seed);
        ImGui::InputInt("StatTrak", &selected_entry.stat_trak);
        ImGui::SliderFloat("Wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);

        ImGui::Combo("Paint Kit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text) {
            *out_text = (itemIndex == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits)[idx].name.c_str();
            return true;
            }, nullptr, (itemIndex == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits).size(), 10);

        ImGui::Combo("Quality", &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text) {
            *out_text = game_data::quality_names[idx].name;
            return true;
            }, nullptr, IM_ARRAYSIZE(game_data::quality_names), 5);

        if (itemIndex == 0) {
            ImGui::Combo("Knife", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
                *out_text = game_data::knife_names[idx].name;
                return true;
                }, nullptr, IM_ARRAYSIZE(game_data::knife_names), 5);
        } else if (itemIndex == 1) {
            ImGui::Combo("Glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
                *out_text = game_data::glove_names[idx].name;
                return true;
                }, nullptr, IM_ARRAYSIZE(game_data::glove_names), 5);
        } else {
            static auto unused_value = 0;
            selected_entry.definition_override_vector_index = 0;
            ImGui::Combo("Unavailable", &unused_value, "For knives or gloves\0");
        }

        ImGui::InputText("Name Tag", selected_entry.custom_name, 32);
    }

    ImGui::NextColumn();

    {
        ImGui::PushID("sticker");

        static auto selectedStickerSlot = 0;

        ImGui::PushItemWidth(-1);

        if (ImGui::ListBoxHeader("", 5)) {
            for (int i = 0; i < 5; ++i) {
                ImGui::PushID(i);

                const auto kit_vector_index = config->skinChanger[itemIndex].stickers[i].kit_vector_index;
                const std::string text = '#' + std::to_string(i + 1) + "  " + SkinChanger::stickerKits[kit_vector_index].name;

                if (ImGui::Selectable(text.c_str(), i == selectedStickerSlot))
                    selectedStickerSlot = i;

                ImGui::PopID();
            }
            ImGui::ListBoxFooter();
        }

        ImGui::PopItemWidth();

        auto& selected_sticker = selected_entry.stickers[selectedStickerSlot];

        ImGui::Combo("Sticker Kit", &selected_sticker.kit_vector_index, [](void* data, int idx, const char** out_text) {
            *out_text = SkinChanger::stickerKits[idx].name.c_str();
            return true;
            }, nullptr, SkinChanger::stickerKits.size(), 10);

        ImGui::SliderFloat("Wear", &selected_sticker.wear, FLT_MIN, 1.0f, "%.10f", 5.0f);
        ImGui::SliderFloat("Scale", &selected_sticker.scale, 0.1f, 5.0f);
        ImGui::SliderFloat("Rotation", &selected_sticker.rotation, 0.0f, 360.0f);

        ImGui::PopID();
    }
    selected_entry.update();

    ImGui::Columns(1);

    ImGui::Separator();

    if (ImGui::Button("Update", { 130.0f, 30.0f }))
        SkinChanger::scheduleHudUpdate();

    ImGui::TextUnformatted("nSkinz by namazso");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderSoundWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.sound)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Sound", &window.sound, windowFlags);
    }
    ImGui::SliderInt("Chicken volume", &config->sound.chickenVolume, 0, 200, "%d%%");

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::Combo("", &currentCategory, "Local player\0Allies\0Enemies\0");
    ImGui::PopItemWidth();
    ImGui::SliderInt("Master volume", &config->sound.players[currentCategory].masterVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Headshot volume", &config->sound.players[currentCategory].headshotVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Weapon volume", &config->sound.players[currentCategory].weaponVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Footstep volume", &config->sound.players[currentCategory].footstepVolume, 0, 200, "%d%%");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderStyleWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.style)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Style", &window.style, windowFlags);
    }

    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu style", &config->style.menuStyle, "Classic\0One window\0"))
        window = { };
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark\0Light\0Classic\0Custom\0"))
        updateColors();
    ImGui::PopItemWidth();

    if (config->style.menuColors == 3) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i]);
        }
    }

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderMiscWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.misc)
            return;
        ImGui::SetNextWindowSize({ 580.0f, 0.0f });
        ImGui::Begin("Misc", &window.misc, windowFlags);
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 230.0f);
    ImGui::TextUnformatted("Menu key");
    ImGui::SameLine();
    hotkey(config->misc.menuKey);

    ImGui::Checkbox("Anti AFK kick", &config->misc.antiAfkKick);
    ImGui::Checkbox("Auto strafe", &config->misc.autoStrafe);
    ImGui::Checkbox("Bunny hop", &config->misc.bunnyHop);
    ImGui::Checkbox("Fast duck", &config->misc.fastDuck);
    ImGui::Checkbox("Moonwalk", &config->misc.moonwalk);
    ImGui::Checkbox("Edge Jump", &config->misc.edgejump);
    ImGui::SameLine();
    hotkey(config->misc.edgejumpkey);
    ImGui::Checkbox("Slowwalk", &config->misc.slowwalk);
    ImGui::SameLine();
    hotkey(config->misc.slowwalkKey);
    ImGui::Checkbox("Sniper crosshair", &config->misc.sniperCrosshair);
    ImGui::Checkbox("Recoil crosshair", &config->misc.recoilCrosshair);
    ImGui::Checkbox("Auto pistol", &config->misc.autoPistol);
    ImGui::Checkbox("Auto reload", &config->misc.autoReload);
    ImGui::Checkbox("Auto accept", &config->misc.autoAccept);
    ImGui::Checkbox("Radar hack", &config->misc.radarHack);
    ImGui::Checkbox("Reveal ranks", &config->misc.revealRanks);
    ImGui::Checkbox("Reveal money", &config->misc.revealMoney);
    ImGui::Checkbox("Reveal suspect", &config->misc.revealSuspect);
    ImGuiCustom::colorPicker("Spectator list", config->misc.spectatorList);
    ImGuiCustom::colorPicker("Watermark", config->misc.watermark);
    ImGui::Checkbox("Fix animation LOD", &config->misc.fixAnimationLOD);
    ImGui::Checkbox("Fix bone matrix", &config->misc.fixBoneMatrix);
    ImGui::Checkbox("Fix movement", &config->misc.fixMovement);
    ImGui::Checkbox("Disable model occlusion", &config->misc.disableModelOcclusion);
    ImGui::SliderFloat("Aspect Ratio", &config->misc.aspectratio, 0.0f, 5.0f, "%.2f");
    ImGui::NextColumn();
    ImGui::Checkbox("Disable HUD blur", &config->misc.disablePanoramablur);
    ImGui::Checkbox("Animated clan tag", &config->misc.animatedClanTag);
    ImGui::Checkbox("Clock tag", &config->misc.clocktag);
    ImGui::Checkbox("Custom clantag", &config->misc.customClanTag);
    ImGui::SameLine();
    ImGui::PushItemWidth(120.0f);
    ImGui::PushID(0);
    if (ImGui::InputText("", &config->misc.clanTag))
        Misc::updateClanTag(true);
    ImGui::PopID();
    ImGui::Checkbox("Kill message", &config->misc.killMessage);
    ImGui::SameLine();
    ImGui::PushItemWidth(120.0f);
    ImGui::PushID(1);
    ImGui::InputText("", &config->misc.killMessageString);
    ImGui::PopID();
    ImGui::Checkbox("Name stealer", &config->misc.nameStealer);
    ImGui::PushID(3);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("", &config->misc.banColor, "White\0Red\0Purple\0Green\0Light green\0Turquoise\0Light red\0Gray\0Yellow\0Gray 2\0Light blue\0Gray/Purple\0Blue\0Pink\0Dark orange\0Orange\0");
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(4);
    ImGui::InputText("", &config->misc.banText);
    ImGui::PopID();
    ImGui::SameLine();
    if (ImGui::Button("Setup fake ban"))
        Misc::fakeBan(true);
    ImGui::Checkbox("Fast plant", &config->misc.fastPlant);
    ImGuiCustom::colorPicker("Bomb timer", config->misc.bombTimer);
    ImGui::Checkbox("Quick reload", &config->misc.quickReload);
    ImGui::Checkbox("Prepare revolver", &config->misc.prepareRevolver);
    ImGui::SameLine();
    hotkey(config->misc.prepareRevolverKey);
    ImGui::Combo("Hit Sound", &config->misc.hitSound, "None\0Metal\0Gamesense\0Bell\0Glass\0Custom\0");
    if (config->misc.hitSound == 5) {
        ImGui::InputText("Hit Sound filename", &config->misc.customHitSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PushID(5);
    ImGui::Combo("Kill Sound", &config->misc.killSound, "None\0Metal\0Gamesense\0Bell\0Glass\0Custom\0");
    if (config->misc.killSound == 5) {
        ImGui::InputText("Kill Sound filename", &config->misc.customKillSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PopID();
    ImGui::SetNextItemWidth(90.0f);
    ImGui::InputInt("Choked packets", &config->misc.chokedPackets, 1, 5);
    config->misc.chokedPackets = std::clamp(config->misc.chokedPackets, 0, 64);
    ImGui::SameLine();
    hotkey(config->misc.chokedPacketsKey);
    ImGui::Text("Quick healthshot");
    ImGui::SameLine();
    hotkey(config->misc.quickHealthshotKey);
    ImGui::Checkbox("Grenade Prediction", &config->misc.nadePredict);
    ImGui::Checkbox("Fix tablet signal", &config->misc.fixTabletSignal);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Max angle delta", &config->misc.maxAngleDelta, 0.0f, 255.0f, "%.2f");
    ImGui::Checkbox("Fake prime", &config->misc.fakePrime);

    if (ImGui::Button("Unhook"))
        hooks->restore();

    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderReportbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.reportbot)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Reportbot", &window.reportbot, windowFlags);
    }
    ImGui::Checkbox("Enabled", &config->reportbot.enabled);
    ImGui::SameLine(0.0f, 50.0f);
    if (ImGui::Button("Reset"))
        Reportbot::reset();
    ImGui::Separator();
    ImGui::PushItemWidth(80.0f);
    ImGui::Combo("Target", &config->reportbot.target, "Enemies\0Allies\0All\0");
    ImGui::InputInt("Delay (s)", &config->reportbot.delay);
    config->reportbot.delay = (std::max)(config->reportbot.delay, 1);
    ImGui::InputInt("Rounds", &config->reportbot.rounds);
    config->reportbot.rounds = (std::max)(config->reportbot.rounds, 1);
    ImGui::PopItemWidth();
    ImGui::Checkbox("Abusive Communications", &config->reportbot.textAbuse);
    ImGui::Checkbox("Griefing", &config->reportbot.griefing);
    ImGui::Checkbox("Wall Hacking", &config->reportbot.wallhack);
    ImGui::Checkbox("Aim Hacking", &config->reportbot.aimbot);
    ImGui::Checkbox("Other Hacking", &config->reportbot.other);

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderConfigWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.config)
            return;
        ImGui::SetNextWindowSize({ 290.0f, 190.0f });
        ImGui::Begin("Config", &window.config, windowFlags);
    }

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 170.0f);

    ImGui::PushItemWidth(160.0f);

    auto& configItems = config->getConfigs();
    static int currentConfig = -1;

    if (static_cast<size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;

    static std::string buffer;

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::string>*>(data);
        *out_text = vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 5) && currentConfig != -1)
            buffer = configItems[currentConfig];

        ImGui::PushID(0);
        if (ImGui::InputText("", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer.c_str());
        }
        ImGui::PopID();
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        if (ImGui::Button("Create config", { 100.0f, 25.0f }))
            config->add(buffer.c_str());

        if (ImGui::Button("Reset config", { 100.0f, 25.0f }))
            ImGui::OpenPopup("Config to reset");

        if (ImGui::BeginPopup("Config to reset")) {
            static constexpr const char* names[]{ "Whole", "Aimbot", "Triggerbot", "Backtrack", "Anti aim", "Glow", "Chams", "Esp", "Visuals", "Skin changer", "Sound", "Style", "Misc", "Reportbot" };
            for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                if (i == 1) ImGui::Separator();

                if (ImGui::Selectable(names[i])) {
                    switch (i) {
                    case 0: config->reset(); updateColors(); Misc::updateClanTag(true); SkinChanger::scheduleHudUpdate(); break;
                    case 1: config->aimbot = { }; break;
                    case 2: config->triggerbot = { }; break;
                    case 3: config->backtrack = { }; break;
                    case 4: config->antiAim = { }; break;
                    case 5: config->glow = { }; break;
                    case 6: config->chams = { }; break;
                    case 7: config->esp = { }; break;
                    case 8: config->visuals = { }; break;
                    case 9: config->skinChanger = { }; SkinChanger::scheduleHudUpdate(); break;
                    case 10: config->sound = { }; break;
                    case 11: config->style = { }; updateColors(); break;
                    case 12: config->misc = { };  Misc::updateClanTag(true); break;
                    case 13: config->reportbot = { }; break;
                    }
                }
            }
            ImGui::EndPopup();
        }
        if (currentConfig != -1) {
            if (ImGui::Button("Load selected", { 100.0f, 25.0f })) {
                config->load(currentConfig);
                updateColors();
                SkinChanger::scheduleHudUpdate();
                Misc::updateClanTag(true);
            }
            if (ImGui::Button("Save selected", { 100.0f, 25.0f }))
                config->save(currentConfig);
            if (ImGui::Button("Delete selected", { 100.0f, 25.0f }))
                config->remove(currentConfig);
        }
        ImGui::Columns(1);
        if (!contentOnly)
            ImGui::End();
}

void GUI::renderGuiStyle2() noexcept
{
    ImGui::SetNextWindowSize({ 600.0f, 0.0f });
    ImGui::Begin("Osiris", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
        if (ImGui::BeginTabItem("Aimbot")) {
            renderAimbotWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Anti aim")) {
            renderAntiAimWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Triggerbot")) {
            renderTriggerbotWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Backtrack")) {
            renderBacktrackWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Glow")) {
            renderGlowWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Chams")) {
            renderChamsWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Esp")) {
            renderEspWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Visuals")) {
            renderVisualsWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Skin changer")) {
            renderSkinChangerWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sound")) {
            renderSoundWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Style")) {
            renderStyleWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Misc")) {
            renderMiscWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Reportbot")) {
            renderReportbotWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Config")) {
            renderConfigWindow(true);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

// Junk Code By Peatreat & Thaisen's Gen
void pJAcAWSVraleonqSlrENUVpAOKDDwfzcVPVVgjDBYKPdKzttOqbXYmNTUzNpTOZMaIJYRVlBnD74862451() {     float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz43895552 = -441892197;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz77821279 = -138877491;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz38408345 = -568208837;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz90897008 = -973210760;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz4024256 = -225261567;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61003777 = -363853191;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz13944037 = -889252237;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66206705 = -126094635;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz59528978 = -269143901;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz75309774 = -143981746;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz68306713 = -165933848;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz65918733 = -52190309;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz59460133 = -356913482;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz11538677 = -757520318;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz25225319 = -941682341;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz79480442 = -417083158;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz84284598 = -593157246;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66259661 = -653417916;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz44936237 = 68396898;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz18303904 = -838707661;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61834197 = -24335195;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz16344784 = -230906147;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz95306050 = -16618900;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz91880143 = -667859131;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61312275 = -508645020;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz22460798 = -436944012;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz77178172 = -231487608;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz86541540 = -97455906;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz48599792 = -118811893;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66257915 = -606727138;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27043823 = -711915042;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz39832588 = -115826311;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz62884658 = -497725435;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz91545052 = -860018803;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz4447411 = -375155412;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz31535957 = 77757869;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz87410868 = 7373642;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz72002285 = -32204530;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz82886828 = -827644474;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz24326916 = -578562007;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz54977161 = -741114638;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz26625785 = -336354797;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz64618210 = -266575127;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz98955338 = -992333471;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz3814353 = -988203094;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz53771148 = -308581685;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46198945 = -93377190;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46345008 = -775886626;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96124747 = 56239407;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz62500341 = -37402420;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz70113842 = -3521451;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93440811 = -147374500;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz55264 = -183578285;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz29768206 = -603709078;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz13028255 = -46922304;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27550769 = -110986051;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz82515229 = -22258591;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46528202 = -900349706;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz29584734 = -364565740;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz81563458 = -788317556;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz83825604 = -32365584;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27402497 = -691796332;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz17606913 = 92717258;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93271063 = -662416764;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz48265951 = -432066704;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz28474125 = 49892462;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz3034076 = -554464875;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz67915080 = -496894680;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz7091267 = -282364906;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93689362 = -919440210;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz92069574 = -324456801;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz12282314 = -460952717;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz83372833 = -825773443;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz20609321 = -353041095;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz63326742 = 2406976;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz35208413 = -687980399;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz51726573 = -964331020;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96350711 = -24285430;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz88065791 = -679656037;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz7541127 = -100063335;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz76261852 = -243566823;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz30833164 = -455600983;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz90416792 = -53695314;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz86099450 = 18590526;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96144073 = -503205688;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz33603012 = -464540543;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz39777325 = -932248026;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz33116452 = -894016358;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz78516798 = -713096499;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz76896641 = -164169362;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz49020728 = -899983541;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz40882666 = -92276652;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz42417551 = -667638790;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz1323370 = 60673081;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz40501311 = -446196424;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27574664 = 50681693;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz9018872 = -329072055;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz71347147 = -604158364;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz50689387 = -460266767;    float pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz75340227 = -441892197;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz43895552 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz77821279;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz77821279 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz38408345;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz38408345 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz90897008;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz90897008 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz4024256;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz4024256 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61003777;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61003777 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz13944037;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz13944037 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66206705;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66206705 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz59528978;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz59528978 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz75309774;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz75309774 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz68306713;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz68306713 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz65918733;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz65918733 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz59460133;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz59460133 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz11538677;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz11538677 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz25225319;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz25225319 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz79480442;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz79480442 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz84284598;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz84284598 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66259661;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66259661 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz44936237;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz44936237 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz18303904;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz18303904 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61834197;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61834197 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz16344784;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz16344784 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz95306050;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz95306050 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz91880143;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz91880143 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61312275;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz61312275 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz22460798;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz22460798 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz77178172;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz77178172 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz86541540;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz86541540 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz48599792;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz48599792 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66257915;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz66257915 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27043823;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27043823 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz39832588;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz39832588 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz62884658;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz62884658 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz91545052;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz91545052 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz4447411;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz4447411 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz31535957;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz31535957 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz87410868;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz87410868 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz72002285;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz72002285 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz82886828;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz82886828 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz24326916;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz24326916 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz54977161;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz54977161 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz26625785;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz26625785 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz64618210;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz64618210 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz98955338;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz98955338 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz3814353;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz3814353 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz53771148;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz53771148 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46198945;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46198945 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46345008;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46345008 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96124747;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96124747 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz62500341;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz62500341 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz70113842;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz70113842 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93440811;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93440811 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz55264;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz55264 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz29768206;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz29768206 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz13028255;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz13028255 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27550769;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27550769 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz82515229;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz82515229 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46528202;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz46528202 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz29584734;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz29584734 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz81563458;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz81563458 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz83825604;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz83825604 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27402497;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27402497 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz17606913;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz17606913 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93271063;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93271063 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz48265951;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz48265951 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz28474125;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz28474125 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz3034076;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz3034076 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz67915080;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz67915080 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz7091267;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz7091267 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93689362;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz93689362 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz92069574;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz92069574 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz12282314;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz12282314 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz83372833;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz83372833 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz20609321;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz20609321 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz63326742;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz63326742 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz35208413;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz35208413 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz51726573;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz51726573 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96350711;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96350711 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz88065791;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz88065791 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz7541127;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz7541127 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz76261852;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz76261852 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz30833164;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz30833164 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz90416792;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz90416792 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz86099450;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz86099450 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96144073;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz96144073 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz33603012;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz33603012 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz39777325;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz39777325 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz33116452;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz33116452 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz78516798;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz78516798 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz76896641;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz76896641 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz49020728;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz49020728 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz40882666;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz40882666 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz42417551;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz42417551 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz1323370;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz1323370 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz40501311;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz40501311 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27574664;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz27574664 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz9018872;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz9018872 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz71347147;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz71347147 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz50689387;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz50689387 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz75340227;     pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz75340227 = pJFLqORKoGDcEiBBAgWNiJlCCBqUbtEZjUxDNIJyYIaSIyDaXQUFfZVfYDJoYwiuBdskkxnsXXpZiStEyPwLZz43895552;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void kPvFpkJbGEOlFeDKxnChgwOZyMJYZIAaFqfkUyAFZmFsTpdQIkkNyVQoAWHcbkCxDogOzSqynx35404577() {     float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB68906226 = -59522879;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB59197201 = -892303753;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40215331 = -663926981;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB30607187 = -359721542;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB48586490 = -260910757;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB39356401 = -218918192;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8950731 = -719658963;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96266675 = -700626693;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8329026 = -987436430;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58947068 = -611230641;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB46495584 = 25296856;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB82952838 = 48446927;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB49801044 = -622966028;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58216439 = -472142229;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB7774890 = 26338218;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB52763119 = -863167110;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB11487562 = -119443794;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28425251 = -685280937;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB50398955 = -479583891;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB90169373 = -869794316;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB38618777 = -892325931;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB85879049 = -431611753;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB60565524 = -679094532;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB69847183 = -194194903;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB47269868 = -455519790;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28083909 = -846994751;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40845638 = -17717864;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33214003 = -216562401;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB16499027 = -96900015;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB20375109 = -484702088;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB6083936 = -295613520;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB15458442 = -101172263;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB56531139 = 88276716;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58706810 = -632879150;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33994304 = -451624390;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28454183 = -808506070;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB54610849 = -389287515;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB2763252 = -587730659;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB88853116 = -76364169;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83404860 = -234004827;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB9530419 = -829052267;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB77175645 = 34361189;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96060676 = -190313121;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB57247205 = -707638255;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33754852 = -364658466;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB11919620 = -620399364;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB884802 = -450364251;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB54774228 = -214172504;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB37527839 = -174482335;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB65342201 = -202997828;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79731746 = -408825613;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB65057684 = -243231100;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB94209850 = -331241755;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB81781098 = 18019113;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96785800 = 22220728;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83027177 = -627911127;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98631676 = -113209222;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB70368147 = -369732079;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83337318 = -904201753;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB20502582 = -413916007;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98510763 = -101200329;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB75736728 = -403096563;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79767649 = -503726679;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB87953917 = -402734342;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB52863132 = -215617121;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB31037143 = -873530882;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB26421700 = 60170211;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB91094234 = -990086879;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB24222136 = 79482161;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79320706 = -165155712;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98152270 = -373879595;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8724310 = -531713135;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB39572135 = -508916768;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB66994095 = -145579064;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB80638955 = 59257951;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB61443131 = -826687121;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB89818372 = -141298632;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB3318320 = -971456277;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB36092332 = -829536438;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB35350249 = -835120427;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB27199107 = -296630500;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB86071409 = -803545360;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB95686163 = 57919934;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB51156825 = -893902187;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40643362 = 24123524;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB41026252 = 47617580;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB21248591 = -769930508;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB74750041 = -929742398;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB61921009 = -555099879;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB50967126 = -823713264;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB29822506 = -595296848;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB84242702 = 80444563;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB19425934 = -683528907;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB68350535 = -662448163;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB84894096 = -32804498;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33793691 = -325955705;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB97407996 = -461912132;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8106760 = -787578780;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB4384073 = -392021135;    float HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB2717709 = -59522879;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB68906226 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB59197201;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB59197201 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40215331;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40215331 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB30607187;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB30607187 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB48586490;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB48586490 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB39356401;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB39356401 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8950731;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8950731 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96266675;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96266675 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8329026;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8329026 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58947068;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58947068 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB46495584;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB46495584 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB82952838;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB82952838 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB49801044;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB49801044 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58216439;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58216439 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB7774890;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB7774890 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB52763119;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB52763119 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB11487562;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB11487562 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28425251;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28425251 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB50398955;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB50398955 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB90169373;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB90169373 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB38618777;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB38618777 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB85879049;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB85879049 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB60565524;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB60565524 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB69847183;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB69847183 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB47269868;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB47269868 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28083909;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28083909 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40845638;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40845638 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33214003;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33214003 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB16499027;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB16499027 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB20375109;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB20375109 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB6083936;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB6083936 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB15458442;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB15458442 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB56531139;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB56531139 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58706810;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB58706810 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33994304;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33994304 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28454183;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB28454183 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB54610849;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB54610849 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB2763252;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB2763252 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB88853116;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB88853116 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83404860;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83404860 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB9530419;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB9530419 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB77175645;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB77175645 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96060676;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96060676 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB57247205;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB57247205 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33754852;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33754852 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB11919620;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB11919620 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB884802;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB884802 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB54774228;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB54774228 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB37527839;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB37527839 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB65342201;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB65342201 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79731746;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79731746 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB65057684;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB65057684 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB94209850;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB94209850 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB81781098;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB81781098 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96785800;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB96785800 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83027177;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83027177 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98631676;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98631676 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB70368147;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB70368147 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83337318;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB83337318 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB20502582;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB20502582 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98510763;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98510763 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB75736728;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB75736728 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79767649;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79767649 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB87953917;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB87953917 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB52863132;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB52863132 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB31037143;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB31037143 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB26421700;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB26421700 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB91094234;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB91094234 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB24222136;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB24222136 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79320706;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB79320706 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98152270;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB98152270 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8724310;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8724310 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB39572135;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB39572135 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB66994095;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB66994095 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB80638955;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB80638955 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB61443131;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB61443131 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB89818372;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB89818372 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB3318320;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB3318320 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB36092332;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB36092332 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB35350249;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB35350249 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB27199107;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB27199107 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB86071409;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB86071409 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB95686163;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB95686163 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB51156825;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB51156825 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40643362;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB40643362 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB41026252;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB41026252 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB21248591;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB21248591 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB74750041;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB74750041 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB61921009;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB61921009 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB50967126;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB50967126 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB29822506;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB29822506 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB84242702;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB84242702 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB19425934;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB19425934 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB68350535;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB68350535 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB84894096;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB84894096 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33793691;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB33793691 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB97407996;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB97407996 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8106760;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB8106760 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB4384073;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB4384073 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB2717709;     HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB2717709 = HtrWGIitQoXnbjZrkyrXeokHpeJPRqtlZBNXwePzuhUNZPMwpHhsHQTTJjhczTbavmOaWCvGircwUWiZSMZKIB68906226;}
// Junk Finished
