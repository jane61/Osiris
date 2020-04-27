#include <sstream>

#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../Netvars.h"
#include "Misc.h"
#include "../SDK/ConVar.h"
#include "../SDK/Surface.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/WeaponData.h"
#include "EnginePrediction.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/GameEvent.h"
#include "../SDK/FrameStage.h"
#include "../SDK/Client.h"

void Misc::edgejump(UserCmd* cmd) noexcept
{
    if (!config->misc.edgejump || !GetAsyncKeyState(config->misc.edgejumpkey))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (const auto mt = localPlayer->moveType(); mt == MoveType::LADDER || mt == MoveType::NOCLIP)
        return;

    if ((EnginePrediction::getFlags() & 1) && !(localPlayer->flags() & 1))
        cmd->buttons |= UserCmd::IN_JUMP;
}

void Misc::slowwalk(UserCmd* cmd) noexcept
{
    if (!config->misc.slowwalk || !GetAsyncKeyState(config->misc.slowwalkKey))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon)
        return;

    const auto weaponData = activeWeapon->getWeaponData();
    if (!weaponData)
        return;

    const float maxSpeed = (localPlayer->isScoped() ? weaponData->maxSpeedAlt : weaponData->maxSpeed) / 3;

    if (cmd->forwardmove && cmd->sidemove) {
        const float maxSpeedRoot = maxSpeed * static_cast<float>(M_SQRT1_2);
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeedRoot : maxSpeedRoot;
    } else if (cmd->forwardmove) {
        cmd->forwardmove = cmd->forwardmove < 0.0f ? -maxSpeed : maxSpeed;
    } else if (cmd->sidemove) {
        cmd->sidemove = cmd->sidemove < 0.0f ? -maxSpeed : maxSpeed;
    }
}

void Misc::inverseRagdollGravity() noexcept
{
    static auto ragdollGravity = interfaces->cvar->findVar("cl_ragdoll_gravity");
    ragdollGravity->setValue(config->visuals.inverseRagdollGravity ? -600 : 600);
}

void Misc::updateClanTag(bool tagChanged) noexcept
{
    if (config->misc.customClanTag) {
        static std::string clanTag;

        if (tagChanged) {
            clanTag = config->misc.clanTag;
            if (!isblank(clanTag.front()) && !isblank(clanTag.back()))
                clanTag.push_back(' ');
        }

        static auto lastTime{ 0.0f };
        if (memory->globalVars->realtime - lastTime < 0.6f) return;
        lastTime = memory->globalVars->realtime;

        if (config->misc.animatedClanTag && !clanTag.empty())
            std::rotate(std::begin(clanTag), std::next(std::begin(clanTag)), std::end(clanTag));

        memory->setClanTag(clanTag.c_str(), clanTag.c_str());

        if (config->misc.clocktag) {
            const auto time{ std::time(nullptr) };
            const auto localTime{ std::localtime(&time) };

            const auto timeString{ '[' + std::to_string(localTime->tm_hour) + ':' + std::to_string(localTime->tm_min) + ':' + std::to_string(localTime->tm_sec) + ']' };
            memory->setClanTag(timeString.c_str(), timeString.c_str());
        }
    }
}

void Misc::spectatorList() noexcept
{
    if (!config->misc.spectatorList.enabled)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    interfaces->surface->setTextFont(Surface::font);

    if (config->misc.spectatorList.rainbow)
        interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config->misc.spectatorList.rainbowSpeed));
    else
        interfaces->surface->setTextColor(config->misc.spectatorList.color);

    const auto [width, height] = interfaces->surface->getScreenSize();

    auto textPositionY = static_cast<int>(0.5f * height);

    for (int i = 1; i <= interfaces->engine->getMaxClients(); ++i) {
        const auto entity = interfaces->entityList->getEntity(i);
        if (!entity || entity->isDormant() || entity->isAlive() || entity->getObserverTarget() != localPlayer.get())
            continue;

        PlayerInfo playerInfo;

        if (!interfaces->engine->getPlayerInfo(i, playerInfo))
            continue;

        if (wchar_t name[128]; MultiByteToWideChar(CP_UTF8, 0, playerInfo.name, -1, name, 128)) {
            const auto [textWidth, textHeight] = interfaces->surface->getTextSize(Surface::font, name);
            interfaces->surface->setTextPosition(width - textWidth - 5, textPositionY);
            textPositionY -= textHeight;
            interfaces->surface->printText(name);
        }
    }
}

void Misc::sniperCrosshair() noexcept
{
    static auto showSpread = interfaces->cvar->findVar("weapon_debug_spread_show");
    showSpread->setValue(config->misc.sniperCrosshair && localPlayer && !localPlayer->isScoped() ? 3 : 0);
}

void Misc::recoilCrosshair() noexcept
{
    static auto recoilCrosshair = interfaces->cvar->findVar("cl_crosshair_recoil");
    recoilCrosshair->setValue(config->misc.recoilCrosshair ? 1 : 0);
}

void Misc::watermark() noexcept
{
    if (config->misc.watermark.enabled) {
        interfaces->surface->setTextFont(Surface::font);

        if (config->misc.watermark.rainbow)
            interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config->misc.watermark.rainbowSpeed));
        else
            interfaces->surface->setTextColor(config->misc.watermark.color);

        interfaces->surface->setTextPosition(5, 0);
        interfaces->surface->printText(L"Osiris");

        static auto frameRate = 1.0f;
        frameRate = 0.9f * frameRate + 0.1f * memory->globalVars->absoluteFrameTime;
        const auto [screenWidth, screenHeight] = interfaces->surface->getScreenSize();
        std::wstring fps{ L"FPS: " + std::to_wstring(static_cast<int>(1 / frameRate)) };
        const auto [fpsWidth, fpsHeight] = interfaces->surface->getTextSize(Surface::font, fps.c_str());
        interfaces->surface->setTextPosition(screenWidth - fpsWidth - 5, 0);
        interfaces->surface->printText(fps.c_str());

        float latency = 0.0f;
        if (auto networkChannel = interfaces->engine->getNetworkChannel(); networkChannel && networkChannel->getLatency(0) > 0.0f)
            latency = networkChannel->getLatency(0);

        std::wstring ping{ L"PING: " + std::to_wstring(static_cast<int>(latency * 1000)) + L" ms" };
        const auto pingWidth = interfaces->surface->getTextSize(Surface::font, ping.c_str()).first;
        interfaces->surface->setTextPosition(screenWidth - pingWidth - 5, fpsHeight);
        interfaces->surface->printText(ping.c_str());
    }
}

void Misc::prepareRevolver(UserCmd* cmd) noexcept
{
    constexpr auto timeToTicks = [](float time) {  return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick); };
    constexpr float revolverPrepareTime{ 0.234375f };

    static float readyTime;
    if (config->misc.prepareRevolver && localPlayer && (!config->misc.prepareRevolverKey || GetAsyncKeyState(config->misc.prepareRevolverKey))) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->itemDefinitionIndex2() == WeaponId::Revolver) {
            if (!readyTime) readyTime = memory->globalVars->serverTime() + revolverPrepareTime;
            auto ticksToReady = timeToTicks(readyTime - memory->globalVars->serverTime() - interfaces->engine->getNetworkChannel()->getLatency(0));
            if (ticksToReady > 0 && ticksToReady <= timeToTicks(revolverPrepareTime))
                cmd->buttons |= UserCmd::IN_ATTACK;
            else
                readyTime = 0.0f;
        }
    }
}

void Misc::fastPlant(UserCmd* cmd) noexcept
{
    if (config->misc.fastPlant) {
        static auto plantAnywhere = interfaces->cvar->findVar("mp_plant_c4_anywhere");

        if (plantAnywhere->getInt())
            return;

        if (!localPlayer || !localPlayer->isAlive() || localPlayer->inBombZone())
            return;

        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (!activeWeapon || activeWeapon->getClientClass()->classId != ClassId::C4)
            return;

        cmd->buttons &= ~UserCmd::IN_ATTACK;

        constexpr float doorRange{ 200.0f };
        Vector viewAngles{ cos(degreesToRadians(cmd->viewangles.x)) * cos(degreesToRadians(cmd->viewangles.y)) * doorRange,
                           cos(degreesToRadians(cmd->viewangles.x)) * sin(degreesToRadians(cmd->viewangles.y)) * doorRange,
                          -sin(degreesToRadians(cmd->viewangles.x)) * doorRange };
        Trace trace;
        interfaces->engineTrace->traceRay({ localPlayer->getEyePosition(), localPlayer->getEyePosition() + viewAngles }, 0x46004009, localPlayer.get(), trace);

        if (!trace.entity || trace.entity->getClientClass()->classId != ClassId::PropDoorRotating)
            cmd->buttons &= ~UserCmd::IN_USE;
    }
}

void Misc::drawBombTimer() noexcept
{
    if (config->misc.bombTimer.enabled) {
        for (int i = interfaces->engine->getMaxClients(); i <= interfaces->entityList->getHighestEntityIndex(); i++) {
            Entity* entity = interfaces->entityList->getEntity(i);
            if (!entity || entity->isDormant() || entity->getClientClass()->classId != ClassId::PlantedC4 || !entity->c4Ticking())
                continue;

            constexpr unsigned font{ 0xc1 };
            interfaces->surface->setTextFont(font);
            interfaces->surface->setTextColor(255, 255, 255);
            auto drawPositionY{ interfaces->surface->getScreenSize().second / 8 };
            auto bombText{ (std::wstringstream{ } << L"Bomb on " << (!entity->c4BombSite() ? 'A' : 'B') << L" : " << std::fixed << std::showpoint << std::setprecision(3) << (std::max)(entity->c4BlowTime() - memory->globalVars->currenttime, 0.0f) << L" s").str() };
            const auto bombTextX{ interfaces->surface->getScreenSize().first / 2 - static_cast<int>((interfaces->surface->getTextSize(font, bombText.c_str())).first / 2) };
            interfaces->surface->setTextPosition(bombTextX, drawPositionY);
            drawPositionY += interfaces->surface->getTextSize(font, bombText.c_str()).second;
            interfaces->surface->printText(bombText.c_str());

            const auto progressBarX{ interfaces->surface->getScreenSize().first / 3 };
            const auto progressBarLength{ interfaces->surface->getScreenSize().first / 3 };
            constexpr auto progressBarHeight{ 5 };

            interfaces->surface->setDrawColor(50, 50, 50);
            interfaces->surface->drawFilledRect(progressBarX - 3, drawPositionY + 2, progressBarX + progressBarLength + 3, drawPositionY + progressBarHeight + 8);
            if (config->misc.bombTimer.rainbow)
                interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config->misc.bombTimer.rainbowSpeed));
            else
                interfaces->surface->setDrawColor(config->misc.bombTimer.color);

            static auto c4Timer = interfaces->cvar->findVar("mp_c4timer");

            interfaces->surface->drawFilledRect(progressBarX, drawPositionY + 5, static_cast<int>(progressBarX + progressBarLength * std::clamp(entity->c4BlowTime() - memory->globalVars->currenttime, 0.0f, c4Timer->getFloat()) / c4Timer->getFloat()), drawPositionY + progressBarHeight + 5);

            if (entity->c4Defuser() != -1) {
                if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(interfaces->entityList->getEntityFromHandle(entity->c4Defuser())->index(), playerInfo)) {
                    if (wchar_t name[128];  MultiByteToWideChar(CP_UTF8, 0, playerInfo.name, -1, name, 128)) {
                        drawPositionY += interfaces->surface->getTextSize(font, L" ").second;
                        const auto defusingText{ (std::wstringstream{ } << name << L" is defusing: " << std::fixed << std::showpoint << std::setprecision(3) << (std::max)(entity->c4DefuseCountDown() - memory->globalVars->currenttime, 0.0f) << L" s").str() };

                        interfaces->surface->setTextPosition((interfaces->surface->getScreenSize().first - interfaces->surface->getTextSize(font, defusingText.c_str()).first) / 2, drawPositionY);
                        interfaces->surface->printText(defusingText.c_str());
                        drawPositionY += interfaces->surface->getTextSize(font, L" ").second;

                        interfaces->surface->setDrawColor(50, 50, 50);
                        interfaces->surface->drawFilledRect(progressBarX - 3, drawPositionY + 2, progressBarX + progressBarLength + 3, drawPositionY + progressBarHeight + 8);
                        interfaces->surface->setDrawColor(0, 255, 0);
                        interfaces->surface->drawFilledRect(progressBarX, drawPositionY + 5, progressBarX + static_cast<int>(progressBarLength * (std::max)(entity->c4DefuseCountDown() - memory->globalVars->currenttime, 0.0f) / (interfaces->entityList->getEntityFromHandle(entity->c4Defuser())->hasDefuser() ? 5.0f : 10.0f)), drawPositionY + progressBarHeight + 5);

                        drawPositionY += interfaces->surface->getTextSize(font, L" ").second;
                        const wchar_t* canDefuseText;

                        if (entity->c4BlowTime() >= entity->c4DefuseCountDown()) {
                            canDefuseText = L"Can Defuse";
                            interfaces->surface->setTextColor(0, 255, 0);
                        } else {
                            canDefuseText = L"Cannot Defuse";
                            interfaces->surface->setTextColor(255, 0, 0);
                        }

                        interfaces->surface->setTextPosition((interfaces->surface->getScreenSize().first - interfaces->surface->getTextSize(font, canDefuseText).first) / 2, drawPositionY);
                        interfaces->surface->printText(canDefuseText);
                    }
                }
            }
            break;
        }
    }
}

void Misc::stealNames() noexcept
{
    if (!config->misc.nameStealer)
        return;

    if (!localPlayer)
        return;

    static std::vector<int> stolenIds;

    for (int i = 1; i <= memory->globalVars->maxClients; ++i) {
        const auto entity = interfaces->entityList->getEntity(i);

        if (!entity || entity == localPlayer.get())
            continue;

        PlayerInfo playerInfo;
        if (!interfaces->engine->getPlayerInfo(entity->index(), playerInfo))
            continue;

        if (playerInfo.fakeplayer || std::find(stolenIds.cbegin(), stolenIds.cend(), playerInfo.userId) != stolenIds.cend())
            continue;

        if (changeName(false, (std::string{ playerInfo.name } +'\x1').c_str(), 1.0f))
            stolenIds.push_back(playerInfo.userId);

        return;
    }
    stolenIds.clear();
}

void Misc::disablePanoramablur() noexcept
{
    static auto blur = interfaces->cvar->findVar("@panorama_disable_blur");
    blur->setValue(config->misc.disablePanoramablur);
}

void Misc::quickReload(UserCmd* cmd) noexcept
{
    if (config->misc.quickReload) {
        static Entity* reloadedWeapon{ nullptr };

        if (reloadedWeapon) {
            for (auto weaponHandle : localPlayer->weapons()) {
                if (weaponHandle == -1)
                    break;

                if (interfaces->entityList->getEntityFromHandle(weaponHandle) == reloadedWeapon) {
                    cmd->weaponselect = reloadedWeapon->index();
                    cmd->weaponsubtype = reloadedWeapon->getWeaponSubType();
                    break;
                }
            }
            reloadedWeapon = nullptr;
        }

        if (auto activeWeapon{ localPlayer->getActiveWeapon() }; activeWeapon && activeWeapon->isInReload() && activeWeapon->clip() == activeWeapon->getWeaponData()->maxClip) {
            reloadedWeapon = activeWeapon;

            for (auto weaponHandle : localPlayer->weapons()) {
                if (weaponHandle == -1)
                    break;

                if (auto weapon{ interfaces->entityList->getEntityFromHandle(weaponHandle) }; weapon && weapon != reloadedWeapon) {
                    cmd->weaponselect = weapon->index();
                    cmd->weaponsubtype = weapon->getWeaponSubType();
                    break;
                }
            }
        }
    }
}

bool Misc::changeName(bool reconnect, const char* newName, float delay) noexcept
{
    static auto exploitInitialized{ false };

    static auto name{ interfaces->cvar->findVar("name") };

    if (reconnect) {
        exploitInitialized = false;
        return false;
    }

    if (!exploitInitialized && interfaces->engine->isInGame()) {
        if (PlayerInfo playerInfo; localPlayer && interfaces->engine->getPlayerInfo(localPlayer->index(), playerInfo) && (!strcmp(playerInfo.name, "?empty") || !strcmp(playerInfo.name, "\n\xAD\xAD\xAD"))) {
            exploitInitialized = true;
        } else {
            name->onChangeCallbacks.size = 0;
            name->setValue("\n\xAD\xAD\xAD");
            return false;
        }
    }

    static auto nextChangeTime{ 0.0f };
    if (nextChangeTime <= memory->globalVars->realtime) {
        name->setValue(newName);
        nextChangeTime = memory->globalVars->realtime + delay;
        return true;
    }
    return false;
}

void Misc::bunnyHop(UserCmd* cmd) noexcept
{
    if (!localPlayer)
        return;

    static auto wasLastTimeOnGround{ localPlayer->flags() & 1 };

    if (config->misc.bunnyHop && !(localPlayer->flags() & 1) && localPlayer->moveType() != MoveType::LADDER && !wasLastTimeOnGround)
        cmd->buttons &= ~UserCmd::IN_JUMP;

    wasLastTimeOnGround = localPlayer->flags() & 1;
}

void Misc::fakeBan(bool set) noexcept
{
    static bool shouldSet = false;

    if (set)
        shouldSet = set;

    if (shouldSet && interfaces->engine->isInGame() && changeName(false, std::string{ "\x1\xB" }.append(std::string{ static_cast<char>(config->misc.banColor + 1) }).append(config->misc.banText).append("\x1").c_str(), 5.0f))
        shouldSet = false;
}

void Misc::nadePredict() noexcept
{
    static auto nadeVar{ interfaces->cvar->findVar("cl_grenadepreview") };

    nadeVar->onChangeCallbacks.size = 0;
    nadeVar->setValue(config->misc.nadePredict);
}

void Misc::quickHealthshot(UserCmd* cmd) noexcept
{
    if (!localPlayer)
        return;

    static bool inProgress{ false };

    if (GetAsyncKeyState(config->misc.quickHealthshotKey))
        inProgress = true;

    if (auto activeWeapon{ localPlayer->getActiveWeapon() }; activeWeapon && inProgress) {
        if (activeWeapon->getClientClass()->classId == ClassId::Healthshot && localPlayer->nextAttack() <= memory->globalVars->serverTime() && activeWeapon->nextPrimaryAttack() <= memory->globalVars->serverTime())
            cmd->buttons |= UserCmd::IN_ATTACK;
        else {
            for (auto weaponHandle : localPlayer->weapons()) {
                if (weaponHandle == -1)
                    break;

                if (const auto weapon{ interfaces->entityList->getEntityFromHandle(weaponHandle) }; weapon && weapon->getClientClass()->classId == ClassId::Healthshot) {
                    cmd->weaponselect = weapon->index();
                    cmd->weaponsubtype = weapon->getWeaponSubType();
                    return;
                }
            }
        }
        inProgress = false;
    }
}

void Misc::fixTabletSignal() noexcept
{
    if (config->misc.fixTabletSignal && localPlayer) {
        if (auto activeWeapon{ localPlayer->getActiveWeapon() }; activeWeapon && activeWeapon->getClientClass()->classId == ClassId::Tablet)
            activeWeapon->tabletReceptionIsBlocked() = false;
    }
}

void Misc::fakePrime() noexcept
{
    static bool lastState = false;

    if (config->misc.fakePrime != lastState) {
        lastState = config->misc.fakePrime;

        if (DWORD oldProtect; VirtualProtect(memory->fakePrime, 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            constexpr uint8_t patch[]{ 0x74, 0xEB };
            *memory->fakePrime = patch[config->misc.fakePrime];
            VirtualProtect(memory->fakePrime, 1, oldProtect, nullptr);
        }
    }
}

void Misc::killMessage(GameEvent& event) noexcept
{
    if (!config->misc.killMessage)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    PlayerInfo localInfo;

    if (!interfaces->engine->getPlayerInfo(localPlayer->index(), localInfo))
        return;

    if (event.getInt("attacker") != localInfo.userId || event.getInt("userid") == localInfo.userId)
        return;

    std::string cmd = "say \"";
    cmd += config->misc.killMessageString;
    cmd += '"';
    interfaces->engine->clientCmdUnrestricted(cmd.c_str());
}

void Misc::fixMovement(UserCmd* cmd, float yaw) noexcept
{
    if (config->misc.fixMovement) {
        float oldYaw = yaw + (yaw < 0.0f ? 360.0f : 0.0f);
        float newYaw = cmd->viewangles.y + (cmd->viewangles.y < 0.0f ? 360.0f : 0.0f);
        float yawDelta = newYaw < oldYaw ? fabsf(newYaw - oldYaw) : 360.0f - fabsf(newYaw - oldYaw);
        yawDelta = 360.0f - yawDelta;

        const float forwardmove = cmd->forwardmove;
        const float sidemove = cmd->sidemove;
        cmd->forwardmove = std::cos(degreesToRadians(yawDelta)) * forwardmove + std::cos(degreesToRadians(yawDelta + 90.0f)) * sidemove;
        cmd->sidemove = std::sin(degreesToRadians(yawDelta)) * forwardmove + std::sin(degreesToRadians(yawDelta + 90.0f)) * sidemove;
    }
}

void Misc::antiAfkKick(UserCmd* cmd) noexcept
{
    if (config->misc.antiAfkKick && cmd->commandNumber % 2)
        cmd->buttons |= 1 << 26;
}

void Misc::fixAnimationLOD(FrameStage stage) noexcept
{
    if (config->misc.fixAnimationLOD && stage == FrameStage::RENDER_START) {
        if (!localPlayer)
            return;

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            Entity* entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()) continue;
            *reinterpret_cast<int*>(entity + 0xA28) = 0;
            *reinterpret_cast<int*>(entity + 0xA30) = memory->globalVars->framecount;
        }
    }
}

void Misc::autoPistol(UserCmd* cmd) noexcept
{
    if (config->misc.autoPistol && localPlayer) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && activeWeapon->isPistol() && activeWeapon->nextPrimaryAttack() > memory->globalVars->serverTime()) {
            if (activeWeapon->itemDefinitionIndex2() == WeaponId::Revolver)
                cmd->buttons &= ~UserCmd::IN_ATTACK2;
            else
                cmd->buttons &= ~UserCmd::IN_ATTACK;
        }
    }
}

void Misc::chokePackets(bool& sendPacket) noexcept
{
    if (!config->misc.chokedPacketsKey || GetAsyncKeyState(config->misc.chokedPacketsKey))
        sendPacket = interfaces->engine->getNetworkChannel()->chokedPackets >= config->misc.chokedPackets;
}

void Misc::autoReload(UserCmd* cmd) noexcept
{
    if (config->misc.autoReload && localPlayer) {
        const auto activeWeapon = localPlayer->getActiveWeapon();
        if (activeWeapon && getWeaponIndex(activeWeapon->itemDefinitionIndex2()) && !activeWeapon->clip())
            cmd->buttons &= ~(UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2);
    }
}

void Misc::revealRanks(UserCmd* cmd) noexcept
{
    if (config->misc.revealRanks && cmd->buttons & UserCmd::IN_SCORE)
        interfaces->client->dispatchUserMessage(50, 0, 0, nullptr);
}

void Misc::autoStrafe(UserCmd* cmd) noexcept
{
    if (localPlayer
        && config->misc.autoStrafe
        && !(localPlayer->flags() & 1)
        && localPlayer->moveType() != MoveType::NOCLIP) {
        if (cmd->mousedx < 0)
            cmd->sidemove = -450.0f;
        else if (cmd->mousedx > 0)
            cmd->sidemove = 450.0f;
    }
}

void Misc::removeCrouchCooldown(UserCmd* cmd) noexcept
{
    if (config->misc.fastDuck)
        cmd->buttons |= UserCmd::IN_BULLRUSH;
}

void Misc::moonwalk(UserCmd* cmd) noexcept
{
    if (config->misc.moonwalk && localPlayer && localPlayer->moveType() != MoveType::LADDER)
        cmd->buttons ^= UserCmd::IN_FORWARD | UserCmd::IN_BACK | UserCmd::IN_MOVELEFT | UserCmd::IN_MOVERIGHT;
}

void Misc::playHitSound(GameEvent& event) noexcept
{
    if (!config->misc.hitSound)
        return;

    if (!localPlayer)
        return;

    PlayerInfo localInfo;

    if (!interfaces->engine->getPlayerInfo(localPlayer->index(), localInfo))
        return;

    if (event.getInt("attacker") != localInfo.userId || event.getInt("userid") == localInfo.userId)
        return;

    constexpr std::array hitSounds{
        "play physics/metal/metal_solid_impact_bullet2",
        "play buttons/arena_switch_press_02",
        "play training/timer_bell",
        "play physics/glass/glass_impact_bullet1"
    };

    if (static_cast<std::size_t>(config->misc.hitSound - 1) < hitSounds.size())
        interfaces->engine->clientCmdUnrestricted(hitSounds[config->misc.hitSound - 1]);
    else if (config->misc.hitSound == 5)
        interfaces->engine->clientCmdUnrestricted(("play " + config->misc.customHitSound).c_str());
}

void Misc::killSound(GameEvent& event) noexcept
{
    if (!config->misc.killSound)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    PlayerInfo localInfo;

    if (!interfaces->engine->getPlayerInfo(localPlayer->index(), localInfo))
        return;

    if (event.getInt("attacker") != localInfo.userId || event.getInt("userid") == localInfo.userId)
        return;

    constexpr std::array killSounds{
        "play physics/metal/metal_solid_impact_bullet2",
        "play buttons/arena_switch_press_02",
        "play training/timer_bell",
        "play physics/glass/glass_impact_bullet1"
    };

    if (static_cast<std::size_t>(config->misc.killSound - 1) < killSounds.size())
        interfaces->engine->clientCmdUnrestricted(killSounds[config->misc.killSound - 1]);
    else if (config->misc.killSound == 5)
        interfaces->engine->clientCmdUnrestricted(("play " + config->misc.customKillSound).c_str());
}

// Junk Code By Peatreat & Thaisen's Gen
void ttXszumSFrvPGJhiyQbfUXnCeLcnOZeCqokumoJcMqNVBRlpBvyzexBWFUOnRzEfNsYSYnvKHy15324266() {     float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW6686573 = -947539783;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW60662930 = -277265160;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW30176236 = -733721758;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW10027939 = -107007414;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW90905283 = -915811541;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW78209773 = -366201896;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW8741293 = -281521958;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW91015279 = -242479904;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW42878666 = -720667441;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW84048843 = 54535774;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW80553641 = -861015809;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW87150480 = -207914353;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW55713750 = 30547717;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95779590 = -281070727;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW64652790 = -876633532;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW26982101 = -993377809;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW98146846 = -942220786;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW7567747 = -735824502;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5775665 = 72414403;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16903078 = -428933462;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82256186 = -182801679;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW11670404 = -843270583;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW6407549 = -428404413;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW53649692 = -567513752;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW20874517 = -674066555;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16348885 = -746220765;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW10927523 = -639594104;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW27341821 = -364528770;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5708010 = -673476397;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW36941628 = -480039762;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW46420310 = -148011867;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW54608518 = -350679668;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW94841289 = 52877942;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW68409298 = -582261721;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW75739414 = -790701849;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW94638929 = 74019074;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW90218902 = -532795675;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW27268048 = 44622388;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW12177017 = -409983989;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW71686976 = -440905088;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW25516719 = -775630708;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW45054995 = -89322069;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW29289160 = -420660882;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW24159953 = -565782382;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70809303 = -251028503;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW97078301 = -247866622;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW53007620 = -779931776;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44247289 = -199668388;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW51195005 = -709539671;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44416694 = -958170218;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW28496808 = -572489504;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW60706915 = -606950527;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW2954153 = -715541061;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16780228 = -619220931;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW33068080 = -223692332;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95016169 = -4269201;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW54255382 = -848860747;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW76526544 = -66208007;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW89153422 = -432940860;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW74556398 = -69590777;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW67282251 = -726607793;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW81399471 = -916993189;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW85307270 = -569003507;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5937039 = -140627679;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37628534 = -797452360;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW25945123 = -410336142;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW92309190 = -160792296;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW87304452 = -387190562;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW20040176 = -490368879;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70013861 = -850652607;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW36763198 = -360582135;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70878798 = -886843174;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95390729 = -225840514;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW34088689 = -486680510;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW91386359 = -653302755;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37201192 = 6520390;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82381243 = -322609702;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82247595 = -862622032;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82840388 = -216485249;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW23796216 = -326199933;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW63341265 = -966288990;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW66680233 = -339925716;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW76146816 = -654989099;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW61291315 = -715306180;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW8444820 = -907550259;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW85713394 = -541061340;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW51654366 = -635138607;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW78061061 = -327901127;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW35341218 = -258569390;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW80723244 = -686432648;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW40383548 = -77120179;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW13692359 = -366587668;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW38114626 = -522436753;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37620618 = -240393213;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW4404725 = -714297296;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44117247 = -858637519;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW59747725 = -520318563;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW23352122 = -180033203;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW86531419 = -768330022;    float SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44864181 = -947539783;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW6686573 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW60662930;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW60662930 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW30176236;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW30176236 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW10027939;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW10027939 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW90905283;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW90905283 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW78209773;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW78209773 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW8741293;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW8741293 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW91015279;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW91015279 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW42878666;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW42878666 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW84048843;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW84048843 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW80553641;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW80553641 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW87150480;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW87150480 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW55713750;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW55713750 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95779590;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95779590 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW64652790;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW64652790 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW26982101;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW26982101 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW98146846;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW98146846 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW7567747;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW7567747 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5775665;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5775665 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16903078;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16903078 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82256186;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82256186 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW11670404;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW11670404 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW6407549;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW6407549 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW53649692;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW53649692 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW20874517;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW20874517 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16348885;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16348885 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW10927523;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW10927523 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW27341821;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW27341821 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5708010;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5708010 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW36941628;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW36941628 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW46420310;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW46420310 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW54608518;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW54608518 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW94841289;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW94841289 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW68409298;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW68409298 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW75739414;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW75739414 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW94638929;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW94638929 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW90218902;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW90218902 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW27268048;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW27268048 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW12177017;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW12177017 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW71686976;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW71686976 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW25516719;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW25516719 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW45054995;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW45054995 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW29289160;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW29289160 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW24159953;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW24159953 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70809303;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70809303 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW97078301;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW97078301 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW53007620;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW53007620 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44247289;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44247289 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW51195005;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW51195005 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44416694;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44416694 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW28496808;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW28496808 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW60706915;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW60706915 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW2954153;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW2954153 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16780228;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW16780228 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW33068080;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW33068080 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95016169;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95016169 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW54255382;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW54255382 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW76526544;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW76526544 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW89153422;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW89153422 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW74556398;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW74556398 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW67282251;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW67282251 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW81399471;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW81399471 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW85307270;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW85307270 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5937039;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW5937039 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37628534;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37628534 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW25945123;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW25945123 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW92309190;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW92309190 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW87304452;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW87304452 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW20040176;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW20040176 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70013861;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70013861 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW36763198;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW36763198 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70878798;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW70878798 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95390729;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW95390729 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW34088689;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW34088689 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW91386359;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW91386359 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37201192;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37201192 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82381243;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82381243 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82247595;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82247595 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82840388;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW82840388 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW23796216;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW23796216 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW63341265;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW63341265 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW66680233;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW66680233 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW76146816;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW76146816 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW61291315;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW61291315 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW8444820;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW8444820 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW85713394;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW85713394 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW51654366;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW51654366 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW78061061;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW78061061 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW35341218;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW35341218 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW80723244;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW80723244 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW40383548;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW40383548 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW13692359;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW13692359 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW38114626;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW38114626 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37620618;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW37620618 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW4404725;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW4404725 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44117247;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44117247 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW59747725;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW59747725 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW23352122;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW23352122 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW86531419;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW86531419 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44864181;     SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW44864181 = SeJRAManlSbeYtnOoRaVLNliIJsTRNAosffXMoUBNdNmEiTffKzdjmnEsbJrUHVYSmmsNPBzxZHhAzeZpMnHNW6686573;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void ZVzenHIxWtZpxpZnJbwUjySHkAyEauBOezTVIpuCXXfSCWUDYycRFGbbDsumIEWbrKKyWEeUgu52200881() {     float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE36356498 = -273304942;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE85107233 = -478309893;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE66730958 = -971978056;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE32363163 = 90785015;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE3289113 = -822629479;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE44632720 = -575843124;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98438188 = -360419005;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE48339816 = -989447207;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE67398001 = -875578788;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE52838488 = -688909236;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE22113353 = -775537624;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE56230541 = 57907566;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE35982291 = -847801278;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE70040618 = -998931428;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE93547352 = -943699966;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE9108903 = 4975413;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE83292003 = -527396234;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE95079523 = -258448274;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72137782 = -847891234;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2347781 = -896367925;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE33463085 = -186025785;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98735058 = -552926384;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE80478056 = 41594916;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE50307519 = -432199948;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE56252709 = -948834387;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75782386 = -678825842;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98918725 = -755068854;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE710641 = -584961883;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53996086 = -272256658;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE3426829 = -892199150;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE52106442 = -540262944;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72683089 = 96344172;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42054330 = -591179359;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE30014820 = -952731579;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE28021936 = -890548643;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE46762555 = -175416152;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98167345 = -430143418;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE68567994 = -388385753;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE92427756 = -881141224;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75846645 = -872582743;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45137626 = -635160571;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE43006269 = -180794108;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE81519289 = -315556503;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72022294 = -87043499;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE47720327 = -230056921;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE80239398 = -494440412;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53393844 = -253015754;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE44566132 = 15964378;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45947071 = -342168707;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE27066150 = -573516256;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53283725 = -951092912;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE60795494 = -106263799;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE97250481 = -959079483;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE68521174 = -319439337;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE18737796 = -732783523;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE37621440 = -720378559;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE4629178 = -419904809;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE16423440 = -439778108;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE76110453 = 39619401;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE27506727 = -43803638;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45713995 = -820774271;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE97727548 = -775457122;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE94343729 = -617190549;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE63971173 = -983379638;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE732046 = -48646292;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE49430264 = -771881797;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE14176212 = -350913075;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE5967472 = -895069699;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42018683 = -8382785;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE46784797 = -668283815;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE10941557 = -564881169;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE14724009 = -39010481;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2651768 = -377307050;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE96291136 = -975308492;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE57210155 = -161207354;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE90456816 = 94768322;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE17215769 = -137369881;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE8455762 = -871361586;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2587193 = -102143027;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE76013310 = -354393975;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE22388542 = -325810089;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE54352593 = -671033232;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE54763569 = -142793177;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE26929936 = -698740403;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE50143103 = -941106239;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE91310947 = -333999146;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75432608 = 55423655;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE73533155 = -171740023;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE11277025 = -119948057;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE90400496 = -70170084;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42133378 = -755511343;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE81743906 = -990365311;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE92457541 = -328005155;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE64921030 = -737337586;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE30132651 = 48191528;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE47410078 = -859703449;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE48662539 = -563603559;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE17548117 = -332176866;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE71290249 = 61602793;    float wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98290062 = -273304942;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE36356498 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE85107233;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE85107233 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE66730958;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE66730958 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE32363163;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE32363163 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE3289113;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE3289113 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE44632720;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE44632720 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98438188;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98438188 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE48339816;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE48339816 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE67398001;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE67398001 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE52838488;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE52838488 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE22113353;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE22113353 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE56230541;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE56230541 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE35982291;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE35982291 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE70040618;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE70040618 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE93547352;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE93547352 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE9108903;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE9108903 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE83292003;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE83292003 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE95079523;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE95079523 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72137782;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72137782 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2347781;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2347781 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE33463085;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE33463085 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98735058;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98735058 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE80478056;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE80478056 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE50307519;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE50307519 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE56252709;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE56252709 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75782386;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75782386 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98918725;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98918725 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE710641;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE710641 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53996086;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53996086 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE3426829;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE3426829 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE52106442;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE52106442 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72683089;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72683089 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42054330;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42054330 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE30014820;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE30014820 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE28021936;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE28021936 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE46762555;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE46762555 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98167345;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98167345 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE68567994;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE68567994 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE92427756;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE92427756 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75846645;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75846645 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45137626;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45137626 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE43006269;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE43006269 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE81519289;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE81519289 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72022294;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE72022294 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE47720327;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE47720327 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE80239398;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE80239398 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53393844;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53393844 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE44566132;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE44566132 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45947071;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45947071 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE27066150;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE27066150 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53283725;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE53283725 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE60795494;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE60795494 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE97250481;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE97250481 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE68521174;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE68521174 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE18737796;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE18737796 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE37621440;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE37621440 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE4629178;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE4629178 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE16423440;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE16423440 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE76110453;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE76110453 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE27506727;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE27506727 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45713995;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE45713995 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE97727548;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE97727548 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE94343729;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE94343729 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE63971173;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE63971173 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE732046;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE732046 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE49430264;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE49430264 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE14176212;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE14176212 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE5967472;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE5967472 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42018683;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42018683 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE46784797;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE46784797 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE10941557;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE10941557 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE14724009;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE14724009 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2651768;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2651768 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE96291136;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE96291136 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE57210155;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE57210155 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE90456816;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE90456816 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE17215769;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE17215769 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE8455762;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE8455762 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2587193;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE2587193 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE76013310;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE76013310 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE22388542;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE22388542 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE54352593;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE54352593 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE54763569;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE54763569 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE26929936;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE26929936 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE50143103;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE50143103 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE91310947;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE91310947 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75432608;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE75432608 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE73533155;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE73533155 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE11277025;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE11277025 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE90400496;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE90400496 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42133378;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE42133378 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE81743906;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE81743906 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE92457541;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE92457541 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE64921030;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE64921030 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE30132651;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE30132651 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE47410078;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE47410078 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE48662539;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE48662539 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE17548117;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE17548117 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE71290249;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE71290249 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98290062;     wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE98290062 = wMSzEOfugcSkJYYrRQvItSADasLWhTTIkKJOIDofGOYuJHAmaemOYYBeMOZMDJNMIJzyOJiOcPBfuFtTrZvChE36356498;}
// Junk Finished
