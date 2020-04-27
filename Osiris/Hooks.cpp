#include <functional>
#include <intrin.h>
#include <string>
#include <Windows.h>
#include <Psapi.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "Config.h"
#include "GUI.h"
#include "Hooks.h"
#include "Interfaces.h"
#include "Memory.h"

#include "Hacks/Aimbot.h"
#include "Hacks/AntiAim.h"
#include "Hacks/Backtrack.h"
#include "Hacks/Chams.h"
#include "Hacks/EnginePrediction.h"
#include "Hacks/Esp.h"
#include "Hacks/Glow.h"
#include "Hacks/Misc.h"
#include "Hacks/Reportbot.h"
#include "Hacks/SkinChanger.h"
#include "Hacks/Triggerbot.h"
#include "Hacks/Visuals.h"

#include "SDK/Engine.h"
#include "SDK/Entity.h"
#include "SDK/EntityList.h"
#include "SDK/FrameStage.h"
#include "SDK/GameEvent.h"
#include "SDK/GameUI.h"
#include "SDK/InputSystem.h"
#include "SDK/MaterialSystem.h"
#include "SDK/ModelRender.h"
#include "SDK/Panel.h"
#include "SDK/RenderContext.h"
#include "SDK/SoundInfo.h"
#include "SDK/SoundEmitter.h"
#include "SDK/StudioRender.h"
#include "SDK/Surface.h"
#include "SDK/UserCmd.h"

static LRESULT __stdcall wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    if (msg == WM_KEYDOWN && LOWORD(wParam) == config->misc.menuKey
        || ((msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) && config->misc.menuKey == VK_LBUTTON)
        || ((msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) && config->misc.menuKey == VK_RBUTTON)
        || ((msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) && config->misc.menuKey == VK_MBUTTON)
        || ((msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) && config->misc.menuKey == HIWORD(wParam) + 4)) {
        gui->open = !gui->open;
        if (!gui->open) {
           // ImGui::GetIO().MouseDown[0] = false;
            interfaces->inputSystem->resetInputState();
        }
    }

    LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);

    interfaces->inputSystem->enableInput(!gui->open);

    return CallWindowProc(hooks->originalWndProc, window, msg, wParam, lParam);
}

static HRESULT __stdcall present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{
    static bool imguiInit{ ImGui_ImplDX9_Init(device) };

    device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
    IDirect3DVertexDeclaration9* vertexDeclaration;
    device->GetVertexDeclaration(&vertexDeclaration);

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (gui->open)
        gui->render();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    device->SetVertexDeclaration(vertexDeclaration);
    vertexDeclaration->Release();

    return hooks->originalPresent(device, src, dest, windowOverride, dirtyRegion);
}

static HRESULT __stdcall reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    auto result = hooks->originalReset(device, params);
    ImGui_ImplDX9_CreateDeviceObjects();
    return result;
}

static bool __stdcall createMove(float inputSampleTime, UserCmd* cmd) noexcept
{
    auto result = hooks->clientMode.callOriginal<bool, 24>(inputSampleTime, cmd);

    if (!cmd->commandNumber)
        return result;

    uintptr_t* framePointer;
    __asm mov framePointer, ebp;
    bool& sendPacket = *reinterpret_cast<bool*>(*framePointer - 0x1C);

    static auto previousViewAngles{ cmd->viewangles };
    const auto currentViewAngles{ cmd->viewangles };

    memory->globalVars->serverTime(cmd);
    Misc::nadePredict();
    Misc::antiAfkKick(cmd);
    Misc::fastPlant(cmd);
    Misc::prepareRevolver(cmd);
    Misc::sniperCrosshair();
    Misc::recoilCrosshair();
    Visuals::removeShadows();
    Visuals::skybox();
    Reportbot::run();
    Misc::bunnyHop(cmd);
    Misc::autoStrafe(cmd);
    Misc::removeCrouchCooldown(cmd);
    Misc::autoPistol(cmd);
    Misc::autoReload(cmd);
    Misc::updateClanTag();
    Misc::fakeBan();
    Misc::stealNames();
    Misc::revealRanks(cmd);
    Misc::quickReload(cmd);
    Misc::quickHealthshot(cmd);
    Misc::fixTabletSignal();
    Misc::slowwalk(cmd);

    EnginePrediction::run(cmd);

    Aimbot::run(cmd);
    Triggerbot::run(cmd);
    Backtrack::run(cmd);
    Misc::edgejump(cmd);
    Misc::moonwalk(cmd);

    if (!(cmd->buttons & (UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2))) {
        Misc::chokePackets(sendPacket);
        AntiAim::run(cmd, previousViewAngles, currentViewAngles, sendPacket);
    }

    auto viewAnglesDelta{ cmd->viewangles - previousViewAngles };
    viewAnglesDelta.normalize();
    viewAnglesDelta.x = std::clamp(viewAnglesDelta.x, -config->misc.maxAngleDelta, config->misc.maxAngleDelta);
    viewAnglesDelta.y = std::clamp(viewAnglesDelta.y, -config->misc.maxAngleDelta, config->misc.maxAngleDelta);

    cmd->viewangles = previousViewAngles + viewAnglesDelta;

    cmd->viewangles.normalize();
    Misc::fixMovement(cmd, currentViewAngles.y);

    cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f);
    cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
    cmd->viewangles.z = 0.0f;
    cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
    cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);

    previousViewAngles = cmd->viewangles;

    return false;
}

static int __stdcall doPostScreenEffects(int param) noexcept
{
    if (interfaces->engine->isInGame()) {
        Visuals::modifySmoke();
        Visuals::thirdperson();
        Misc::inverseRagdollGravity();
        Visuals::disablePostProcessing();
        Visuals::reduceFlashEffect();
        Visuals::removeBlur();
        Visuals::updateBrightness();
        Visuals::removeGrass();
        Visuals::remove3dSky();
        Glow::render();
    }
    return hooks->clientMode.callOriginal<int, 44>(param);
}

static float __stdcall getViewModelFov() noexcept
{
    float additionalFov = static_cast<float>(config->visuals.viewmodelFov);
    if (localPlayer) {
        if (const auto activeWeapon = localPlayer->getActiveWeapon(); activeWeapon && activeWeapon->getClientClass()->classId == ClassId::Tablet)
            additionalFov = 0.0f;
    }

    return hooks->clientMode.callOriginal<float, 35>() + additionalFov;
}

static void __stdcall drawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    if (interfaces->engine->isInGame() && !interfaces->studioRender->isForcedMaterialOverride()) {
        if (Visuals::removeHands(info.model->name) || Visuals::removeSleeves(info.model->name) || Visuals::removeWeapons(info.model->name))
            return;

        static Chams chams;
        if (chams.render(ctx, state, info, customBoneToWorld))
            hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);
        interfaces->studioRender->forcedMaterialOverride(nullptr);
    } else
        hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);
}

static bool __stdcall svCheatsGetBool() noexcept
{
    if (uintptr_t(_ReturnAddress()) == memory->cameraThink && config->visuals.thirdperson)
        return true;
    else
        return hooks->svCheats.callOriginal<bool, 13>();
}

static void __stdcall paintTraverse(unsigned int panel, bool forceRepaint, bool allowForce) noexcept
{
    if (interfaces->panel->getName(panel) == "MatSystemTopPanel") {
        Esp::render();
        Misc::drawBombTimer();
        Misc::spectatorList();
        Misc::watermark();        
        Visuals::hitMarker();
    }
    hooks->panel.callOriginal<void, 41>(panel, forceRepaint, allowForce);
}

static void __stdcall frameStageNotify(FrameStage stage) noexcept
{
    static auto backtrackInit = (Backtrack::init(), false);

    if (interfaces->engine->isConnected() && !interfaces->engine->isInGame())
        Misc::changeName(true, nullptr, 0.0f);

    if (stage == FrameStage::RENDER_START) {
        Misc::disablePanoramablur();
        Visuals::colorWorld();
        Misc::fakePrime();
    }
    if (interfaces->engine->isInGame()) {
        Visuals::playerModel(stage);
        Visuals::removeVisualRecoil(stage);
        Visuals::applyZoom(stage);
        Misc::fixAnimationLOD(stage);
        Backtrack::update(stage);
        SkinChanger::run(stage);
    }
    hooks->client.callOriginal<void, 37>(stage);
}

struct SoundData {
    std::byte pad[4];
    int entityIndex;
    int channel;
    const char* soundEntry;
    std::byte pad1[8];
    float volume;
    std::byte pad2[44];
};

static void __stdcall emitSound(SoundData data) noexcept
{
    auto modulateVolume = [&data](int(*get)(int)) {
        if (const auto entity = interfaces->entityList->getEntity(data.entityIndex); localPlayer && entity && entity->isPlayer()) {
            if (data.entityIndex == localPlayer->index())
                data.volume *= get(0) / 100.0f;
            else if (!entity->isEnemy())
                data.volume *= get(1) / 100.0f;
            else
                data.volume *= get(2) / 100.0f;
        }
    };

    modulateVolume([](int index) { return config->sound.players[index].masterVolume; });

    if (strstr(data.soundEntry, "Weapon") && strstr(data.soundEntry, "Single")) {
        modulateVolume([](int index) { return config->sound.players[index].weaponVolume; });
    } else if (config->misc.autoAccept && !strcmp(data.soundEntry, "UIPanorama.popup_accept_match_beep")) {
        memory->acceptMatch("");
        auto window = FindWindowW(L"Valve001", NULL);
        FLASHWINFO flash{ sizeof(FLASHWINFO), window, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
        FlashWindowEx(&flash);
        ShowWindow(window, SW_RESTORE);
    }
    data.volume = std::clamp(data.volume, 0.0f, 1.0f);
    hooks->sound.callOriginal<void, 5>(data);
}

static bool __stdcall shouldDrawFog() noexcept
{
    return !config->visuals.noFog;
}

static bool __stdcall shouldDrawViewModel() noexcept
{
    if (config->visuals.zoom && localPlayer && localPlayer->fov() < 45 && localPlayer->fovStart() < 45)
        return false;
    return hooks->clientMode.callOriginal<bool, 27>();
}

static void __stdcall lockCursor() noexcept
{
    if (gui->open)
        return interfaces->surface->unlockCursor();
    return hooks->surface.callOriginal<void, 67>();
}

static void __stdcall setDrawColor(int r, int g, int b, int a) noexcept
{
    auto returnAddress = reinterpret_cast<uintptr_t>(_ReturnAddress());
    if (config->visuals.noScopeOverlay && (returnAddress == memory->scopeArc || returnAddress == memory->scopeLens))
        a = 0;
    hooks->surface.callOriginal<void, 15>(r, g, b, a);
}

static bool __stdcall fireEventClientSide(GameEvent* event) noexcept
{
    if (event) {
        switch (fnv::hashRuntime(event->getName())) {
        case fnv::hash("player_death"):
            Misc::killMessage(*event);
            Misc::killSound(*event);
            SkinChanger::overrideHudIcon(*event);
            break;
        case fnv::hash("player_hurt"):
            Misc::playHitSound(*event);
            Visuals::hitEffect(event);                
            Visuals::hitMarker(event);
            break;
        }
    }
    return hooks->gameEventManager.callOriginal<bool, 9>(event);
}

struct ViewSetup {
    std::byte pad[176];
    float fov;
    std::byte pad1[32];
    float farZ;
};

static void __stdcall overrideView(ViewSetup* setup) noexcept
{
    if (localPlayer && !localPlayer->isScoped())
        setup->fov += config->visuals.fov;
    setup->farZ += config->visuals.farZ * 10;
    hooks->clientMode.callOriginal<void, 18>(setup);
}

struct RenderableInfo {
    Entity* renderable;
    std::byte pad[18];
    uint16_t flags;
    uint16_t flags2;
};

static int __stdcall listLeavesInBox(const Vector& mins, const Vector& maxs, unsigned short* list, int listMax) noexcept
{
    if (std::uintptr_t(_ReturnAddress()) == memory->listLeaves) {
        if (const auto info = *reinterpret_cast<RenderableInfo**>(std::uintptr_t(_AddressOfReturnAddress()) + 0x14); info && info->renderable) {
            if (const auto ent = callVirtualMethod<Entity*>(info->renderable - 4, 7); ent && ent->isPlayer()) {
                if (config->misc.disableModelOcclusion) {
                    // FIXME: sometimes players are rendered above smoke, maybe sort render list?
                    info->flags &= ~0x100;
                    info->flags2 |= 0x40;

                    constexpr float maxCoord = 16384.0f;
                    constexpr float minCoord = -maxCoord;
                    constexpr Vector min{ minCoord, minCoord, minCoord };
                    constexpr Vector max{ maxCoord, maxCoord, maxCoord };
                    return hooks->bspQuery.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);
                }
            }
        }
    }
    return hooks->bspQuery.callOriginal<int, 6>(std::cref(mins), std::cref(maxs), list, listMax);
}

static int __fastcall dispatchSound(SoundInfo& soundInfo) noexcept
{
    if (const char* soundName = interfaces->soundEmitter->getSoundName(soundInfo.soundIndex)) {
        auto modulateVolume = [&soundInfo](int(*get)(int)) {
            if (auto entity{ interfaces->entityList->getEntity(soundInfo.entityIndex) }; entity && entity->isPlayer()) {
                if (localPlayer && soundInfo.entityIndex == localPlayer->index())
                    soundInfo.volume *= get(0) / 100.0f;
                else if (!entity->isEnemy())
                    soundInfo.volume *= get(1) / 100.0f;
                else
                    soundInfo.volume *= get(2) / 100.0f;
            }
        };

        modulateVolume([](int index) { return config->sound.players[index].masterVolume; });

        if (!strcmp(soundName, "Player.DamageHelmetFeedback"))
            modulateVolume([](int index) { return config->sound.players[index].headshotVolume; });
        else if (strstr(soundName, "Step"))
            modulateVolume([](int index) { return config->sound.players[index].footstepVolume; });
        else if (strstr(soundName, "Chicken"))
            soundInfo.volume *= config->sound.chickenVolume / 100.0f;
    }
    soundInfo.volume = std::clamp(soundInfo.volume, 0.0f, 1.0f);
    return hooks->originalDispatchSound(soundInfo);
}

static int __stdcall render2dEffectsPreHud(int param) noexcept
{
    Visuals::applyScreenEffects();
    Visuals::hitEffect();
    return hooks->viewRender.callOriginal<int, 39>(param);
}

static void* __stdcall getDemoPlaybackParameters() noexcept
{
    if (uintptr_t returnAddress = uintptr_t(_ReturnAddress()); config->misc.revealSuspect && (returnAddress == memory->test || returnAddress == memory->test2))
        return nullptr;

    return hooks->engine.callOriginal<void*, 218>();
}

static bool __stdcall isPlayingDemo() noexcept
{
    if (config->misc.revealMoney
        && *static_cast<uintptr_t*>(_ReturnAddress()) == 0x0975C084  // client_panorama.dll : 84 C0 75 09 38 05
        && **reinterpret_cast<uintptr_t**>(uintptr_t(_AddressOfReturnAddress()) + 4) == 0x0C75C084) { // client_panorama.dll : 84 C0 75 0C 5B
        return true;
    }
    return hooks->engine.callOriginal<bool, 82>();
}

static void __stdcall updateColorCorrectionWeights() noexcept
{
    hooks->clientMode.callOriginal<void, 58>();

    if (const auto& cfg = config->visuals.colorCorrection; cfg.enabled) {
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x498) = cfg.blue;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4A0) = cfg.red;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4A8) = cfg.mono;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4B0) = cfg.saturation;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4C0) = cfg.ghost;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4C8) = cfg.green;
        *reinterpret_cast<float*>(std::uintptr_t(memory->clientMode) + 0x4D0) = cfg.yellow;
    }

    if (config->visuals.noScopeOverlay)
        *memory->vignette = 0.0f;
}

static float __stdcall getScreenAspectRatio(int width, int height) noexcept
{
    if (config->misc.aspectratio)
        return config->misc.aspectratio;
    return hooks->engine.callOriginal<float, 101>(width, height);
}

static void __stdcall renderSmokeOverlay(bool update) noexcept
{
    if (config->visuals.noSmoke || config->visuals.wireframeSmoke)
        *reinterpret_cast<float*>(std::uintptr_t(memory->viewRender) + 0x588) = 0.0f;
    else
        hooks->viewRender.callOriginal<void, 41>(update);
}

Hooks::Hooks(HMODULE cheatModule) : module{ cheatModule }
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    originalWndProc = WNDPROC(SetWindowLongPtrA(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, LONG_PTR(wndProc)));
}

void Hooks::install() noexcept
{
    SkinChanger::initializeKits();

    originalPresent = **reinterpret_cast<decltype(originalPresent)**>(memory->present);
    **reinterpret_cast<decltype(present)***>(memory->present) = present;
    originalReset = **reinterpret_cast<decltype(originalReset)**>(memory->reset);
    **reinterpret_cast<decltype(reset)***>(memory->reset) = reset;

    bspQuery.hookAt(6, listLeavesInBox);
    client.hookAt(37, frameStageNotify);
    clientMode.hookAt(17, shouldDrawFog);
    clientMode.hookAt(18, overrideView);
    clientMode.hookAt(24, createMove);
    clientMode.hookAt(27, shouldDrawViewModel);
    clientMode.hookAt(35, getViewModelFov);
    clientMode.hookAt(44, doPostScreenEffects);
    clientMode.hookAt(58, updateColorCorrectionWeights);
    engine.hookAt(82, isPlayingDemo);
    engine.hookAt(101, getScreenAspectRatio);
    engine.hookAt(218, getDemoPlaybackParameters);
    gameEventManager.hookAt(9, fireEventClientSide);
    modelRender.hookAt(21, drawModelExecute);
    panel.hookAt(41, paintTraverse);
    sound.hookAt(5, emitSound);
    surface.hookAt(15, setDrawColor);
    surface.hookAt(67, lockCursor);
    svCheats.hookAt(13, svCheatsGetBool);
    viewRender.hookAt(39, render2dEffectsPreHud);
    viewRender.hookAt(41, renderSmokeOverlay);

    if (DWORD oldProtection; VirtualProtect(memory->dispatchSound, 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        originalDispatchSound = decltype(originalDispatchSound)(uintptr_t(memory->dispatchSound + 1) + *memory->dispatchSound);
        *memory->dispatchSound = uintptr_t(dispatchSound) - uintptr_t(memory->dispatchSound + 1);
        VirtualProtect(memory->dispatchSound, 4, oldProtection, nullptr);
    }
}

void Hooks::restore() noexcept
{
    bspQuery.restore();
    client.restore();
    clientMode.restore();
    engine.restore();
    gameEventManager.restore();
    modelRender.restore();
    panel.restore();
    sound.restore();
    surface.restore();
    svCheats.restore();
    viewRender.restore();

    netvars->restore();

    Glow::clearCustomObjects();

    SetWindowLongPtrA(FindWindowW(L"Valve001", nullptr), GWLP_WNDPROC, LONG_PTR(originalWndProc));
    **reinterpret_cast<void***>(memory->present) = originalPresent;
    **reinterpret_cast<void***>(memory->reset) = originalReset;

    if (DWORD oldProtection; VirtualProtect(memory->dispatchSound, 4, PAGE_EXECUTE_READWRITE, &oldProtection)) {
        *memory->dispatchSound = uintptr_t(originalDispatchSound) - uintptr_t(memory->dispatchSound + 1);
        VirtualProtect(memory->dispatchSound, 4, oldProtection, nullptr);
    }

    interfaces->inputSystem->enableInput(true);
}

uintptr_t* Hooks::Vmt::findFreeDataPage(void* const base, size_t vmtSize) noexcept
{
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(base, &mbi, sizeof(mbi));
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess(), static_cast<HMODULE>(mbi.AllocationBase), &moduleInfo, sizeof(moduleInfo));

    auto moduleEnd{ reinterpret_cast<uintptr_t*>(static_cast<std::byte*>(moduleInfo.lpBaseOfDll) + moduleInfo.SizeOfImage) };

    for (auto currentAddress = moduleEnd - vmtSize; currentAddress > moduleInfo.lpBaseOfDll; currentAddress -= *currentAddress ? vmtSize : 1)
        if (!*currentAddress)
            if (VirtualQuery(currentAddress, &mbi, sizeof(mbi)) && mbi.State == MEM_COMMIT
                && mbi.Protect == PAGE_READWRITE && mbi.RegionSize >= vmtSize * sizeof(uintptr_t)
                && std::all_of(currentAddress, currentAddress + vmtSize, [](uintptr_t a) { return !a; }))
                return currentAddress;

    return nullptr;
}

auto Hooks::Vmt::calculateLength(uintptr_t* vmt) noexcept
{
    size_t length{ 0 };
    MEMORY_BASIC_INFORMATION memoryInfo;
    while (VirtualQuery(LPCVOID(vmt[length]), &memoryInfo, sizeof(memoryInfo)) && memoryInfo.Protect == PAGE_EXECUTE_READ)
        length++;
    return length;
}

bool Hooks::Vmt::init(void* const base) noexcept
{
    assert(base);
    this->base = base;
    bool init = false;

    if (!oldVmt) {
        oldVmt = *reinterpret_cast<uintptr_t**>(base);
        length = calculateLength(oldVmt) + 1;

        if (newVmt = findFreeDataPage(base, length))
            std::copy(oldVmt - 1, oldVmt - 1 + length, newVmt);
        assert(newVmt);
        init = true;
    }
    if (newVmt)
        *reinterpret_cast<uintptr_t**>(base) = newVmt + 1;
    return init;
}

void Hooks::Vmt::restore() noexcept
{
    if (base && oldVmt)
        *reinterpret_cast<uintptr_t**>(base) = oldVmt;
    if (newVmt)
        ZeroMemory(newVmt, length * sizeof(uintptr_t));
}

// Junk Code By Peatreat & Thaisen's Gen
void uKzReKyUabafRfFTpZcbiHkvwGJFqIzWVBjikSSduCxNzDAOyeaXjyRmZEVYAtdRbDKPvXjWGz99614803() {     float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs9891204 = -865644779;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34140257 = -541604757;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs32972921 = -339400836;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs55663319 = -287381945;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs53232932 = 96678494;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs45530501 = -961141710;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs53359063 = -237269392;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs61177494 = -448387821;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88304663 = -519049377;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14804128 = -900477545;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs37024730 = -115347223;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs52273510 = -665399604;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs33433434 = -193324854;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs90880334 = -482733213;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs39014293 = -74988925;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs15882684 = -715106093;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs92268851 = -533646190;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs99706701 = -403287551;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs5530485 = 42524937;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs27474498 = -638713933;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs2712921 = -942547778;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71062026 = -248245316;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69999626 = -937458085;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs85213838 = 18550648;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs11794720 = -193309719;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs40960950 = 41428845;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs76004429 = -147021033;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49925646 = -196769560;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs94014214 = 80557186;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs5017069 = -793222084;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs90763460 = -558249511;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49533403 = -680582386;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49537248 = -763655484;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs62324844 = -437598110;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs18557000 = -69067413;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14909721 = -48414519;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34483208 = -80706289;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs79296954 = 58795714;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69478867 = -854594863;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88412042 = -65219692;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs56054961 = -759569333;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14858533 = -646659800;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12655828 = -2606425;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs36277215 = -379869888;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs10788861 = 92353259;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14950374 = -776140393;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12780885 = -852709761;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs32377805 = -27554888;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88888031 = -195735338;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs81097520 = 22851774;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs16504469 = -887516822;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs99566980 = 27935324;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs67265004 = -619474372;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs15142321 = -126684053;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs96901883 = -801179357;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs38829178 = -517399464;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs64140630 = -604146673;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs47759082 = -257951485;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs43868600 = 5927773;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12271982 = -944750352;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69526071 = -714120677;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs3433417 = 59500168;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs67163280 = -428945008;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs83287594 = -725827293;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs24040668 = -242228035;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs87491327 = -434764838;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs2736263 = -901744121;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71108589 = -755726745;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs72323335 = -313665800;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs24104572 = 73425594;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs81399475 = -534399804;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12971898 = -492441904;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs30227835 = -548692688;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs17118443 = -892255371;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71419537 = -879144600;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs87854387 = -195887979;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs58406198 = -145638891;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs33722412 = -457588197;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs74424978 = 26197389;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs96844346 = -417169326;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs28180066 = -105861394;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs43626625 = -19466146;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs61037615 = 98965778;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12916694 = -942294589;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88512600 = -905705263;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs91196479 = -486184835;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs82268398 = 38891985;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34394927 = -536971432;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs65422961 = -636418754;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs79727821 = -551667949;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs50769091 = -444267847;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs86724126 = -822754805;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs35428355 = -947132060;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs57206885 = -909844512;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs18885971 = -351099015;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs52621544 = -719069502;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs47695252 = -117714792;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs29368234 = -276779132;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12236547 = -37641854;    float hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs23297534 = -865644779;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs9891204 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34140257;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34140257 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs32972921;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs32972921 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs55663319;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs55663319 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs53232932;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs53232932 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs45530501;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs45530501 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs53359063;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs53359063 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs61177494;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs61177494 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88304663;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88304663 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14804128;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14804128 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs37024730;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs37024730 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs52273510;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs52273510 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs33433434;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs33433434 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs90880334;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs90880334 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs39014293;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs39014293 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs15882684;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs15882684 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs92268851;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs92268851 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs99706701;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs99706701 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs5530485;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs5530485 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs27474498;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs27474498 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs2712921;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs2712921 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71062026;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71062026 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69999626;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69999626 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs85213838;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs85213838 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs11794720;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs11794720 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs40960950;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs40960950 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs76004429;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs76004429 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49925646;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49925646 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs94014214;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs94014214 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs5017069;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs5017069 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs90763460;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs90763460 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49533403;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49533403 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49537248;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs49537248 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs62324844;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs62324844 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs18557000;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs18557000 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14909721;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14909721 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34483208;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34483208 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs79296954;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs79296954 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69478867;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69478867 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88412042;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88412042 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs56054961;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs56054961 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14858533;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14858533 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12655828;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12655828 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs36277215;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs36277215 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs10788861;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs10788861 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14950374;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs14950374 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12780885;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12780885 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs32377805;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs32377805 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88888031;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88888031 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs81097520;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs81097520 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs16504469;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs16504469 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs99566980;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs99566980 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs67265004;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs67265004 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs15142321;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs15142321 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs96901883;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs96901883 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs38829178;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs38829178 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs64140630;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs64140630 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs47759082;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs47759082 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs43868600;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs43868600 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12271982;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12271982 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69526071;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs69526071 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs3433417;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs3433417 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs67163280;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs67163280 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs83287594;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs83287594 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs24040668;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs24040668 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs87491327;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs87491327 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs2736263;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs2736263 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71108589;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71108589 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs72323335;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs72323335 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs24104572;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs24104572 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs81399475;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs81399475 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12971898;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12971898 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs30227835;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs30227835 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs17118443;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs17118443 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71419537;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs71419537 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs87854387;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs87854387 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs58406198;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs58406198 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs33722412;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs33722412 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs74424978;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs74424978 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs96844346;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs96844346 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs28180066;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs28180066 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs43626625;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs43626625 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs61037615;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs61037615 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12916694;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12916694 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88512600;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs88512600 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs91196479;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs91196479 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs82268398;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs82268398 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34394927;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs34394927 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs65422961;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs65422961 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs79727821;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs79727821 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs50769091;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs50769091 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs86724126;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs86724126 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs35428355;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs35428355 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs57206885;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs57206885 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs18885971;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs18885971 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs52621544;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs52621544 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs47695252;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs47695252 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs29368234;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs29368234 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12236547;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs12236547 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs23297534;     hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs23297534 = hfKWwtFvGMicnWkxIgxucAYfmvtlTauDQjRlbDHbSFmRjzPneyOCYLXgGekAAUbgReLDFzVFYWcWNHwmfFGVMs9891204;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void dIiuONwmzaGuSnAlanMbJQnWymBHKmtRStIFsbohJvrCpZEuZgmmxgrNULvbLpcNgpcYfutyze26353860() {     float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72104385 = -950152016;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa71246844 = -99238475;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa44325227 = -226734673;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61322428 = -360636904;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa19754241 = -816776043;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa41482560 = -513290818;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa22115837 = -379942863;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa35642941 = -610711928;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa36495188 = -499464435;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa86341865 = -103050831;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa23287278 = -288506630;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa62367546 = -144516686;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa59373641 = -868497074;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa7161404 = 81042234;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa27308500 = -234730027;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa16228250 = -917691044;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61462326 = -630483768;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa43564739 = -749815607;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa70582045 = -27455623;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72814557 = -591599238;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa38744935 = -262538403;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa23566938 = -346578369;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61035999 = -547106045;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa45061438 = -589051025;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa66632761 = -401456808;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa69270051 = 32448932;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa76144126 = -925944506;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa91201353 = -657770180;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa17483678 = 11063254;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa64540420 = -473315783;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83839765 = 7055601;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa95107182 = -822896742;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa96679520 = -930200926;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa52905194 = -364897272;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa59169927 = 10696121;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa91553163 = 89294590;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10816523 = -36851508;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa24709838 = -568820013;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa81468563 = -133656460;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72002819 = -567751117;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa27924329 = -900934791;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa32676845 = -716768624;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa39710704 = -331388919;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa28596365 = -78690205;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa57752461 = -494536105;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa70219660 = -960717806;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa88355858 = 95577142;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18288325 = -650820360;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa93879254 = -324441864;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa22068451 = 90999278;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa80998777 = -302407938;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18465007 = 9509002;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa33969051 = -127229074;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa90393461 = -21363860;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa87608924 = -312165574;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa48537448 = -503573648;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10210845 = -552132431;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa99263788 = -637683648;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa94689667 = -959180097;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa50484190 = -749224975;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65338434 = -587346313;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30914484 = -722172684;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18159263 = -521775183;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa71954767 = 73851348;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa2502101 = -10106432;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa28180095 = -465609888;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65688025 = -214315760;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6468448 = -403599803;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa47991476 = -929653887;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa35755337 = -224024618;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa5411728 = -780839537;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa36752488 = 38336244;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa62096175 = -516159148;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa98579226 = -459704507;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa44890228 = -690664448;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6068091 = -545769780;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83856233 = 84810550;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa32439634 = -368415840;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa87308977 = 5485079;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa96413100 = -440739002;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa80914192 = 36871790;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa57855802 = -175124146;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa97322098 = -233328316;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa95415227 = 20063976;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83541642 = -70907846;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65374759 = 97546598;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61138132 = -595667669;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6286059 = -808837067;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65296269 = 47268302;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10632479 = -485730232;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa81342319 = -358572979;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa11552734 = -399167860;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30020171 = -609639916;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30984374 = -384431485;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6664385 = -980404805;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa97009845 = -78762107;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa14517582 = -94993441;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa67755937 = -305240267;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa26094264 = 31416226;    float SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa29572366 = -950152016;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72104385 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa71246844;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa71246844 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa44325227;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa44325227 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61322428;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61322428 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa19754241;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa19754241 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa41482560;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa41482560 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa22115837;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa22115837 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa35642941;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa35642941 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa36495188;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa36495188 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa86341865;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa86341865 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa23287278;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa23287278 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa62367546;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa62367546 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa59373641;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa59373641 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa7161404;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa7161404 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa27308500;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa27308500 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa16228250;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa16228250 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61462326;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61462326 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa43564739;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa43564739 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa70582045;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa70582045 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72814557;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72814557 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa38744935;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa38744935 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa23566938;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa23566938 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61035999;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61035999 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa45061438;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa45061438 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa66632761;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa66632761 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa69270051;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa69270051 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa76144126;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa76144126 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa91201353;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa91201353 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa17483678;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa17483678 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa64540420;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa64540420 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83839765;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83839765 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa95107182;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa95107182 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa96679520;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa96679520 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa52905194;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa52905194 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa59169927;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa59169927 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa91553163;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa91553163 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10816523;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10816523 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa24709838;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa24709838 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa81468563;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa81468563 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72002819;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72002819 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa27924329;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa27924329 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa32676845;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa32676845 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa39710704;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa39710704 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa28596365;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa28596365 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa57752461;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa57752461 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa70219660;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa70219660 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa88355858;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa88355858 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18288325;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18288325 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa93879254;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa93879254 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa22068451;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa22068451 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa80998777;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa80998777 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18465007;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18465007 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa33969051;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa33969051 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa90393461;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa90393461 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa87608924;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa87608924 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa48537448;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa48537448 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10210845;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10210845 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa99263788;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa99263788 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa94689667;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa94689667 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa50484190;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa50484190 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65338434;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65338434 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30914484;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30914484 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18159263;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa18159263 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa71954767;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa71954767 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa2502101;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa2502101 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa28180095;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa28180095 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65688025;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65688025 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6468448;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6468448 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa47991476;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa47991476 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa35755337;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa35755337 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa5411728;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa5411728 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa36752488;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa36752488 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa62096175;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa62096175 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa98579226;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa98579226 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa44890228;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa44890228 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6068091;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6068091 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83856233;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83856233 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa32439634;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa32439634 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa87308977;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa87308977 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa96413100;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa96413100 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa80914192;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa80914192 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa57855802;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa57855802 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa97322098;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa97322098 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa95415227;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa95415227 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83541642;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa83541642 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65374759;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65374759 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61138132;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa61138132 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6286059;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6286059 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65296269;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa65296269 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10632479;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa10632479 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa81342319;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa81342319 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa11552734;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa11552734 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30020171;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30020171 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30984374;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa30984374 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6664385;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa6664385 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa97009845;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa97009845 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa14517582;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa14517582 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa67755937;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa67755937 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa26094264;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa26094264 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa29572366;     SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa29572366 = SNYlgZiGidAHTlBukgfMVTSeVAxfPemnnTNepjeWtFHyNZVqXkJMTmIyutQfbCqDrMUAYXFBCJSQMURwfHIfUa72104385;}
// Junk Finished
