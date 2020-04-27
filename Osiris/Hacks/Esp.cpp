#define NOMINMAX

#include <sstream>

#include "Esp.h"
#include "../Config.h"
#include "../Interfaces.h"
#include "../Memory.h"

#include "../SDK/ConVar.h"
#include "../SDK/Entity.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Localize.h"
#include "../SDK/Surface.h"
#include "../SDK/Vector.h"
#include "../SDK/WeaponData.h"

static constexpr bool worldToScreen(const Vector& in, Vector& out) noexcept
{
    const auto& matrix = interfaces->engine->worldToScreenMatrix();
    float w = matrix._41 * in.x + matrix._42 * in.y + matrix._43 * in.z + matrix._44;

    if (w > 0.001f) {
        const auto [width, height] = interfaces->surface->getScreenSize();
        out.x = width / 2 * (1 + (matrix._11 * in.x + matrix._12 * in.y + matrix._13 * in.z + matrix._14) / w);
        out.y = height / 2 * (1 - (matrix._21 * in.x + matrix._22 * in.y + matrix._23 * in.z + matrix._24) / w);
        out.z = 0.0f;
        return true;
    }
    return false;
}

static void renderSnaplines(Entity* entity, const Config::Esp::Shared& config) noexcept
{
    if (!config.snaplines.enabled)
        return;

    Vector position;
    if (!worldToScreen(entity->getAbsOrigin(), position))
        return;

    if (config.snaplines.rainbow)
        interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.snaplines.rainbowSpeed));
    else
        interfaces->surface->setDrawColor(config.snaplines.color);

    const auto [width, height] = interfaces->surface->getScreenSize();
    interfaces->surface->drawLine(width / 2, height, static_cast<int>(position.x), static_cast<int>(position.y));
}

static void renderEyeTraces(Entity* entity, const Config::Esp::Player& config) noexcept
{
    if (config.eyeTraces.enabled) {
        constexpr float maxRange{ 8192.0f };

        auto eyeAngles = entity->eyeAngles();
        Vector viewAngles{ cos(degreesToRadians(eyeAngles.x)) * cos(degreesToRadians(eyeAngles.y)) * maxRange,
                           cos(degreesToRadians(eyeAngles.x)) * sin(degreesToRadians(eyeAngles.y)) * maxRange,
                          -sin(degreesToRadians(eyeAngles.x)) * maxRange };
        static Trace trace;
        Vector headPosition{ entity->getBonePosition(8) };
        interfaces->engineTrace->traceRay({ headPosition, headPosition + viewAngles }, 0x46004009, { entity }, trace);
        Vector start, end;
        if (worldToScreen(trace.startpos, start) && worldToScreen(trace.endpos, end)) {
            if (config.eyeTraces.rainbow)
                interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.eyeTraces.rainbowSpeed));
            else
                interfaces->surface->setDrawColor(config.eyeTraces.color);

            interfaces->surface->drawLine(start.x, start.y, end.x, end.y);
        }
    }
}

static constexpr void renderPositionedText(unsigned font, const wchar_t* text, std::pair<float, float&> position) noexcept
{
    interfaces->surface->setTextFont(font);
    interfaces->surface->setTextPosition(position.first, position.second);
    position.second += interfaces->surface->getTextSize(font, text).second;
    interfaces->surface->printText(text);
}

struct BoundingBox {
    float x0, y0;
    float x1, y1;
    Vector vertices[8];

    BoundingBox(Entity* entity) noexcept
    {
        const auto [width, height] = interfaces->surface->getScreenSize();

        x0 = static_cast<float>(width * 2);
        y0 = static_cast<float>(height * 2);
        x1 = -x0;
        y1 = -y0;

        const auto& mins = entity->getCollideable()->obbMins();
        const auto& maxs = entity->getCollideable()->obbMaxs();

        for (int i = 0; i < 8; ++i) {
            const Vector point{ i & 1 ? maxs.x : mins.x,
                                i & 2 ? maxs.y : mins.y,
                                i & 4 ? maxs.z : mins.z };

            if (!worldToScreen(point.transform(entity->coordinateFrame()), vertices[i])) {
                valid = false;
                return;
            }
            x0 = std::min(x0, vertices[i].x);
            y0 = std::min(y0, vertices[i].y);
            x1 = std::max(x1, vertices[i].x);
            y1 = std::max(y1, vertices[i].y);
        }
        valid = true;
    }

    operator bool() noexcept
    {
        return valid;
    }
private:
    bool valid;
};

static void renderBox(const BoundingBox& bbox, const Config::Esp::Shared& config) noexcept
{
    if (config.box.enabled) {
        if (config.box.rainbow)
            interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.box.rainbowSpeed));
        else
            interfaces->surface->setDrawColor(config.box.color);
        
        switch (config.boxType) {
        case 0:
            interfaces->surface->drawOutlinedRect(bbox.x0, bbox.y0, bbox.x1, bbox.y1);

            if (config.outline.enabled) {
                if (config.outline.rainbow)
                    interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.outline.rainbowSpeed));
                else
                    interfaces->surface->setDrawColor(config.outline.color);

                interfaces->surface->drawOutlinedRect(bbox.x0 + 1, bbox.y0 + 1, bbox.x1 - 1, bbox.y1 - 1);
                interfaces->surface->drawOutlinedRect(bbox.x0 - 1, bbox.y0 - 1, bbox.x1 + 1, bbox.y1 + 1);
            }
            break;
        case 1:
            interfaces->surface->drawLine(bbox.x0, bbox.y0, bbox.x0, bbox.y0 + fabsf(bbox.y1 - bbox.y0) / 4);
            interfaces->surface->drawLine(bbox.x0, bbox.y0, bbox.x0 + fabsf(bbox.x1 - bbox.x0) / 4, bbox.y0);
            interfaces->surface->drawLine(bbox.x1, bbox.y0, bbox.x1 - fabsf(bbox.x1 - bbox.x0) / 4, bbox.y0);
            interfaces->surface->drawLine(bbox.x1, bbox.y0, bbox.x1, bbox.y0 + fabsf(bbox.y1 - bbox.y0) / 4);
            interfaces->surface->drawLine(bbox.x0, bbox.y1, bbox.x0, bbox.y1 - fabsf(bbox.y1 - bbox.y0) / 4);
            interfaces->surface->drawLine(bbox.x0, bbox.y1, bbox.x0 + fabsf(bbox.x1 - bbox.x0) / 4, bbox.y1);
            interfaces->surface->drawLine(bbox.x1, bbox.y1, bbox.x1 - fabsf(bbox.x1 - bbox.x0) / 4, bbox.y1);
            interfaces->surface->drawLine(bbox.x1, bbox.y1, bbox.x1, bbox.y1 - fabsf(bbox.y1 - bbox.y0) / 4);

            if (config.outline.enabled) {
                if (config.outline.rainbow)
                    interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.outline.rainbowSpeed));
                else
                    interfaces->surface->setDrawColor(config.outline.color);

                // TODO: get rid of fabsf()

                interfaces->surface->drawLine(bbox.x0 - 1, bbox.y0 - 1, bbox.x0 - 1, bbox.y0 + fabsf(bbox.y1 - bbox.y0) / 4);
                interfaces->surface->drawLine(bbox.x0 - 1, bbox.y0 - 1, bbox.x0 + fabsf(bbox.x1 - bbox.x0) / 4, bbox.y0 - 1);
                interfaces->surface->drawLine(bbox.x1 + 1, bbox.y0 - 1, bbox.x1 - fabsf(bbox.x1 - bbox.x0) / 4, bbox.y0 - 1);
                interfaces->surface->drawLine(bbox.x1 + 1, bbox.y0 - 1, bbox.x1 + 1, bbox.y0 + fabsf(bbox.y1 - bbox.y0) / 4);
                interfaces->surface->drawLine(bbox.x0 - 1, bbox.y1 + 1, bbox.x0 - 1, bbox.y1 - fabsf(bbox.y1 - bbox.y0) / 4);
                interfaces->surface->drawLine(bbox.x0 - 1, bbox.y1 + 1, bbox.x0 + fabsf(bbox.x1 - bbox.x0) / 4, bbox.y1 + 1);
                interfaces->surface->drawLine(bbox.x1 + 1, bbox.y1 + 1, bbox.x1 - fabsf(bbox.x1 - bbox.x0) / 4, bbox.y1 + 1);
                interfaces->surface->drawLine(bbox.x1 + 1, bbox.y1 + 1, bbox.x1 + 1, bbox.y1 - fabsf(bbox.y1 - bbox.y0) / 4);


                interfaces->surface->drawLine(bbox.x0 + 1, bbox.y0 + 1, bbox.x0 + 1, bbox.y0 + fabsf(bbox.y1 - bbox.y0) / 4);
                interfaces->surface->drawLine(bbox.x0 + 2, bbox.y0 + 1, bbox.x0 + fabsf(bbox.x1 - bbox.x0) / 4, bbox.y0 + 1);


                interfaces->surface->drawLine(bbox.x1 - 1, bbox.y0 + 1, bbox.x1 - fabsf(bbox.x1 - bbox.x0) / 4, (bbox.y0 + 1));
                interfaces->surface->drawLine(bbox.x1 - 1, bbox.y0 + 1, bbox.x1 - 1, bbox.y0 + fabsf(bbox.y1 - bbox.y0) / 4);

                interfaces->surface->drawLine(bbox.x0 + 1, bbox.y1 - 1, bbox.x0 + 1, (bbox.y1) - fabsf(bbox.y1 - bbox.y0) / 4);
                interfaces->surface->drawLine(bbox.x0 + 1, bbox.y1 - 1, bbox.x0 + fabsf(bbox.x1 - bbox.x0) / 4, bbox.y1 - 1);

                interfaces->surface->drawLine(bbox.x1 - 1, bbox.y1 - 1, bbox.x1 - fabsf(bbox.x1 - bbox.x0) / 4, bbox.y1 - 1);
                interfaces->surface->drawLine(bbox.x1 - 1, bbox.y1 - 2, (bbox.x1 - 1), (bbox.y1 - 1) - fabsf(bbox.y1 - bbox.y0) / 4);

                interfaces->surface->drawLine(bbox.x0 - 1, fabsf((bbox.y1 - bbox.y0) / 4) + bbox.y0, bbox.x0 + 2, fabsf((bbox.y1 - bbox.y0) / 4) + bbox.y0);
                interfaces->surface->drawLine(bbox.x1 + 1, fabsf((bbox.y1 - bbox.y0) / 4) + bbox.y0, bbox.x1 - 2, fabsf((bbox.y1 - bbox.y0) / 4) + bbox.y0);
                interfaces->surface->drawLine(bbox.x0 - 1, fabsf((bbox.y1 - bbox.y0) * 3 / 4) + bbox.y0, bbox.x0 + 2, fabsf((bbox.y1 - bbox.y0) * 3 / 4) + bbox.y0);
                interfaces->surface->drawLine(bbox.x1 + 1, fabsf((bbox.y1 - bbox.y0) * 3 / 4) + bbox.y0, bbox.x1 - 2, fabsf((bbox.y1 - bbox.y0) * 3 / 4) + bbox.y0);
                interfaces->surface->drawLine(fabsf((bbox.x1 - bbox.x0) / 4) + bbox.x0, bbox.y0 + 1, fabsf((bbox.x1 - bbox.x0) / 4) + bbox.x0, bbox.y0 - 2);
                interfaces->surface->drawLine(fabsf((bbox.x1 - bbox.x0) / 4) + bbox.x0, bbox.y1 + 1, fabsf((bbox.x1 - bbox.x0) / 4) + bbox.x0, bbox.y1 - 2);
                interfaces->surface->drawLine(fabsf((bbox.x1 - bbox.x0) * 3 / 4) + bbox.x0, bbox.y0 + 1, fabsf((bbox.x1 - bbox.x0) * 3 / 4) + bbox.x0, bbox.y0 - 2);
                interfaces->surface->drawLine(fabsf((bbox.x1 - bbox.x0) * 3 / 4) + bbox.x0, bbox.y1 + 1, fabsf((bbox.x1 - bbox.x0) * 3 / 4) + bbox.x0, bbox.y1 - 2);
            }
            break;
        case 2:
            for (int i = 0; i < 8; i++) {
                for (int j = 1; j <= 4; j <<= 1) {
                    if (!(i & j))
                        interfaces->surface->drawLine(bbox.vertices[i].x, bbox.vertices[i].y, bbox.vertices[i + j].x, bbox.vertices[i + j].y);
                }
            }
            break;
        case 3:
            for (int i = 0; i < 8; i++) {
                for (int j = 1; j <= 4; j <<= 1) {
                    if (!(i & j)) {
                        interfaces->surface->drawLine(bbox.vertices[i].x, bbox.vertices[i].y, bbox.vertices[i].x + (bbox.vertices[i + j].x - bbox.vertices[i].x) * 0.25f, bbox.vertices[i].y + (bbox.vertices[i + j].y - bbox.vertices[i].y) * 0.25f);
                        interfaces->surface->drawLine(bbox.vertices[i].x + (bbox.vertices[i + j].x - bbox.vertices[i].x) * 0.75f, bbox.vertices[i].y + (bbox.vertices[i + j].y - bbox.vertices[i].y) * 0.75f, bbox.vertices[i + j].x, bbox.vertices[i + j].y);
                    }
                }
            }
            break;
        }
    }
}

static void renderPlayerBox(Entity* entity, const Config::Esp::Player& config) noexcept
{
    if (BoundingBox bbox{ entity }) {
        renderBox(bbox, config);

        float drawPositionX = bbox.x0 - 5;

        if (config.healthBar.enabled) {
            static auto gameType{ interfaces->cvar->findVar("game_type") };
            static auto survivalMaxHealth{ interfaces->cvar->findVar("sv_dz_player_max_health") };

            const auto maxHealth{ (std::max)((gameType->getInt() == 6 ? survivalMaxHealth->getInt() : 100), entity->health()) };

            if (config.healthBar.rainbow)
                interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.healthBar.rainbowSpeed));
            else
                interfaces->surface->setDrawColor(config.healthBar.color);

            interfaces->surface->drawFilledRect(drawPositionX - 3, bbox.y0 + abs(bbox.y1 - bbox.y0) * (maxHealth - entity->health()) / static_cast<float>(maxHealth), drawPositionX, bbox.y1);
            
            if (config.outline.enabled) {
                if (config.outline.rainbow)
                    interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.outline.rainbowSpeed));
                else
                    interfaces->surface->setDrawColor(config.outline.color);

                interfaces->surface->drawOutlinedRect(drawPositionX - 4, bbox.y0 - 1, drawPositionX + 1, bbox.y1 + 1);
            }
            drawPositionX -= 7;
        }

        if (config.armorBar.enabled) {
            if (config.armorBar.rainbow)
                interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.armorBar.rainbowSpeed));
            else
                interfaces->surface->setDrawColor(config.armorBar.color);

            interfaces->surface->drawFilledRect(drawPositionX - 3, bbox.y0 + abs(bbox.y1 - bbox.y0) * (100.0f - entity->armor()) / 100.0f, drawPositionX, bbox.y1);

            if (config.outline.enabled) {
                if (config.outline.rainbow)
                    interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.outline.rainbowSpeed));
                else
                    interfaces->surface->setDrawColor(config.outline.color);

                interfaces->surface->drawOutlinedRect(drawPositionX - 4, bbox.y0 - 1, drawPositionX + 1, bbox.y1 + 1);
            }
            drawPositionX -= 7;
        }

        if (config.name.enabled) {
            if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(entity->index(), playerInfo)) {
                if (wchar_t name[128]; MultiByteToWideChar(CP_UTF8, 0, playerInfo.name, -1, name, 128)) {
                    const auto [width, height] { interfaces->surface->getTextSize(config.font, name) };
                    interfaces->surface->setTextFont(config.font);
                    if (config.name.rainbow)
                        interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.name.rainbowSpeed));
                    else
                        interfaces->surface->setTextColor(config.name.color);

                    interfaces->surface->setTextPosition((bbox.x0 + bbox.x1 - width) / 2, bbox.y0 - 5 - height);
                    interfaces->surface->printText(name);
                }
            }
        }

        if (const auto activeWeapon{ entity->getActiveWeapon() };  config.activeWeapon.enabled && activeWeapon) {
            const auto name{ interfaces->localize->find(activeWeapon->getWeaponData()->name) };
            const auto [width, height] { interfaces->surface->getTextSize(config.font, name) };
            interfaces->surface->setTextFont(config.font);
            if (config.activeWeapon.rainbow)
                interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.activeWeapon.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config.activeWeapon.color);

            interfaces->surface->setTextPosition((bbox.x0 + bbox.x1 - width) / 2, bbox.y1 + 5);
            interfaces->surface->printText(name);
        }     

        float drawPositionY = bbox.y0;

        if (config.health.enabled) {
            if (config.health.rainbow)
                interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.health.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config.health.color);

            renderPositionedText(config.font, (std::to_wstring(entity->health()) + L" HP").c_str(), { bbox.x1 + 5, drawPositionY });
         }

        if (config.armor.enabled) {
            if (config.armor.rainbow)
                interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.armor.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config.armor.color);

            renderPositionedText(config.font, (std::to_wstring(entity->armor()) + L" AR").c_str(), { bbox.x1 + 5, drawPositionY });
        }

        if (config.money.enabled) {
            if (config.money.rainbow)
                interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.money.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config.money.color);

            renderPositionedText(config.font, (L'$' + std::to_wstring(entity->account())).c_str(), { bbox.x1 + 5, drawPositionY });
        }

        if (config.distance.enabled && localPlayer) {
            if (config.distance.rainbow)
                interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.distance.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config.distance.color);

            renderPositionedText(config.font, (std::wostringstream{ } << std::fixed << std::showpoint << std::setprecision(2) << (entity->getAbsOrigin() - localPlayer->getAbsOrigin()).length() * 0.0254f << L'm').str().c_str(), { bbox.x1 + 5, drawPositionY });
        }
    }
}

static void renderWeaponBox(Entity* entity, const Config::Esp::Weapon& config) noexcept
{
    BoundingBox bbox{ entity };

    if (!bbox)
        return;

    renderBox(bbox, config);

    if (config.name.enabled) {
        const auto name{ interfaces->localize->find(entity->getWeaponData()->name) };
        const auto [width, height] { interfaces->surface->getTextSize(config.font, name) };
        interfaces->surface->setTextFont(config.font);
        if (config.name.rainbow)
            interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.name.rainbowSpeed));
        else
            interfaces->surface->setTextColor(config.name.color);

        interfaces->surface->setTextPosition((bbox.x0 + bbox.x1 - width) / 2, bbox.y1 + 5);
        interfaces->surface->printText(name);
    }

    float drawPositionY = bbox.y0;

    if (!localPlayer || !config.distance.enabled)
        return;

    if (config.distance.rainbow)
        interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.distance.rainbowSpeed));
    else
        interfaces->surface->setTextColor(config.distance.color);

    renderPositionedText(config.font, (std::wostringstream{ } << std::fixed << std::showpoint << std::setprecision(2) << (entity->getAbsOrigin() - localPlayer->getAbsOrigin()).length() * 0.0254f << L'm').str().c_str(), { bbox.x1 + 5, drawPositionY });
}

static void renderEntityBox(Entity* entity, const Config::Esp::Shared& config, const wchar_t* name) noexcept
{
    if (BoundingBox bbox{ entity }) {
        renderBox(bbox, config);

        if (config.name.enabled) {
            const auto [width, height] { interfaces->surface->getTextSize(config.font, name) };
            interfaces->surface->setTextFont(config.font);
            if (config.name.rainbow)
                interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.name.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config.name.color);

            interfaces->surface->setTextPosition((bbox.x0 + bbox.x1 - width) / 2, bbox.y1 + 5);
            interfaces->surface->printText(name);
        }

        float drawPositionY = bbox.y0;

        if (!localPlayer || !config.distance.enabled)
            return;

        if (config.distance.rainbow)
            interfaces->surface->setTextColor(rainbowColor(memory->globalVars->realtime, config.distance.rainbowSpeed));
        else
            interfaces->surface->setTextColor(config.distance.color);

        renderPositionedText(config.font, (std::wostringstream{ } << std::fixed << std::showpoint << std::setprecision(2) << (entity->getAbsOrigin() - localPlayer->getAbsOrigin()).length() * 0.0254f << L'm').str().c_str(), { bbox.x1 + 5, drawPositionY });
    }
}

static void renderHeadDot(Entity* entity, const Config::Esp::Player& config) noexcept
{
    if (!config.headDot.enabled)
        return;

    if (!localPlayer)
        return;

    Vector head;
    if (!worldToScreen(entity->getBonePosition(8), head))
        return;

    if (config.headDot.rainbow)
        interfaces->surface->setDrawColor(rainbowColor(memory->globalVars->realtime, config.headDot.rainbowSpeed));
    else
        interfaces->surface->setDrawColor(config.headDot.color);

    interfaces->surface->drawCircle(head.x, head.y, 0, static_cast<int>(100 / std::sqrt((localPlayer->getAbsOrigin() - entity->getAbsOrigin()).length())));
}

enum EspId {
    ALLIES_ALL = 0,
    ALLIES_VISIBLE,
    ALLIES_OCCLUDED,

    ENEMIES_ALL,
    ENEMIES_VISIBLE,
    ENEMIES_OCCLUDED
};

static bool isInRange(Entity* entity, float maxDistance) noexcept
{
    if (!localPlayer)
        return false;

    return maxDistance == 0.0f || (entity->getAbsOrigin() - localPlayer->getAbsOrigin()).length() * 0.0254f <= maxDistance;
}

static constexpr bool renderPlayerEsp(Entity* entity, EspId id) noexcept
{
    if (localPlayer && (config->esp.players[id].enabled ||
        config->esp.players[id].deadesp && !localPlayer->isAlive()) &&
        isInRange(entity, config->esp.players[id].maxDistance)) {
        renderSnaplines(entity, config->esp.players[id]);
        renderEyeTraces(entity, config->esp.players[id]);
        renderPlayerBox(entity, config->esp.players[id]);
        renderHeadDot(entity, config->esp.players[id]);
    }
    return config->esp.players[id].enabled;
}

static void renderWeaponEsp(Entity* entity) noexcept
{
    if (config->esp.weapon.enabled && isInRange(entity, config->esp.weapon.maxDistance)) {
        renderWeaponBox(entity, config->esp.weapon);
        renderSnaplines(entity, config->esp.weapon);
    }
}

static constexpr void renderEntityEsp(Entity* entity, const Config::Esp::Shared& config, const wchar_t* name) noexcept
{
    if (config.enabled && isInRange(entity, config.maxDistance)) {
        renderEntityBox(entity, config, name);
        renderSnaplines(entity, config);
    }
}

void Esp::render() noexcept
{
    if (interfaces->engine->isInGame()) {
        if (!localPlayer)
            return;

        const auto observerTarget = localPlayer->getObserverTarget();

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity == observerTarget
                || entity->isDormant() || !entity->isAlive())
                continue;

            if (!entity->isEnemy()) {
                if (!renderPlayerEsp(entity, ALLIES_ALL)) {
                    if (entity->isVisible())
                        renderPlayerEsp(entity, ALLIES_VISIBLE);
                    else
                        renderPlayerEsp(entity, ALLIES_OCCLUDED);
                }
            } else if (!renderPlayerEsp(entity, ENEMIES_ALL)) {
                if (entity->isVisible())
                    renderPlayerEsp(entity, ENEMIES_VISIBLE);
                else
                    renderPlayerEsp(entity, ENEMIES_OCCLUDED);
            }
        }

        for (int i = interfaces->engine->getMaxClients() + 1; i <= interfaces->entityList->getHighestEntityIndex(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity->isDormant())
                continue;

            if (entity->isWeapon() && entity->ownerEntity() == -1)
                renderWeaponEsp(entity);
            else {
                switch (entity->getClientClass()->classId) {
                case ClassId::Dronegun: {
                    renderEntityEsp(entity, config->esp.dangerZone[0], std::wstring{ interfaces->localize->find("#SFUI_WPNHUD_AutoSentry") }.append(L" (").append(std::to_wstring(entity->sentryHealth())).append(L" HP)").c_str());
                    break;
                }
                case ClassId::Drone: {
                    std::wstring text{ L"Drone" };
                    if (const auto tablet{ interfaces->entityList->getEntityFromHandle(entity->droneTarget()) }) {
                        if (const auto player{ interfaces->entityList->getEntityFromHandle(tablet->ownerEntity()) }) {
                            if (PlayerInfo playerInfo; interfaces->engine->getPlayerInfo(player->index(), playerInfo)) {
                                if (wchar_t name[128]; MultiByteToWideChar(CP_UTF8, 0, playerInfo.name, -1, name, 128)) {
                                    text += L" -> ";
                                    text += name;
                                }
                            }
                        }
                    }
                    renderEntityEsp(entity, config->esp.dangerZone[1], text.c_str());
                    break;
                }
                case ClassId::Cash:
                    renderEntityEsp(entity, config->esp.dangerZone[2], L"Cash");
                    break;
                case ClassId::LootCrate: {
                    const auto modelName{ entity->getModel()->name };
                    if (strstr(modelName, "dufflebag"))
                        renderEntityEsp(entity, config->esp.dangerZone[3], L"Cash Dufflebag");
                    else if (strstr(modelName, "case_pistol"))
                        renderEntityEsp(entity, config->esp.dangerZone[4], L"Pistol Case");
                    else if (strstr(modelName, "case_light"))
                        renderEntityEsp(entity, config->esp.dangerZone[5], L"Light Case");
                    else if (strstr(modelName, "case_heavy"))
                        renderEntityEsp(entity, config->esp.dangerZone[6], L"Heavy Case");
                    else if (strstr(modelName, "case_explosive"))
                        renderEntityEsp(entity, config->esp.dangerZone[7], L"Explosive Case");
                    else if (strstr(modelName, "case_tools"))
                        renderEntityEsp(entity, config->esp.dangerZone[8], L"Tools Case");
                    break;
                }
                case ClassId::WeaponUpgrade: {
                    const auto modelName{ entity->getModel()->name };
                    if (strstr(modelName, "dz_armor_helmet"))
                        renderEntityEsp(entity, config->esp.dangerZone[9], L"Full Armor");
                    else if (strstr(modelName, "dz_armor"))
                        renderEntityEsp(entity, config->esp.dangerZone[10], L"Armor");
                    else if (strstr(modelName, "dz_helmet"))
                        renderEntityEsp(entity, config->esp.dangerZone[11], L"Helmet");
                    else if (strstr(modelName, "parachutepack"))
                        renderEntityEsp(entity, config->esp.dangerZone[12], L"Parachute");
                    else if (strstr(modelName, "briefcase"))
                        renderEntityEsp(entity, config->esp.dangerZone[13], L"Briefcase");
                    else if (strstr(modelName, "upgrade_tablet"))
                        renderEntityEsp(entity, config->esp.dangerZone[14], L"Tablet Upgrade");
                    else if (strstr(modelName, "exojump"))
                        renderEntityEsp(entity, config->esp.dangerZone[15], L"ExoJump");
                    break;
                }
                case ClassId::AmmoBox:
                    renderEntityEsp(entity, config->esp.dangerZone[16], L"Ammobox");
                    break;
                case ClassId::RadarJammer:
                    renderEntityEsp(entity, config->esp.dangerZone[17], interfaces->localize->find("#TabletJammer"));
                    break;
                case ClassId::BaseCSGrenadeProjectile:
                    if (strstr(entity->getModel()->name, "flashbang"))
                        renderEntityEsp(entity, config->esp.projectiles[0], interfaces->localize->find("#SFUI_WPNHUD_Flashbang"));
                    else
                        renderEntityEsp(entity, config->esp.projectiles[1], interfaces->localize->find("#SFUI_WPNHUD_HE_Grenade"));
                    break;
                case ClassId::BreachChargeProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[2], interfaces->localize->find("#SFUI_WPNHUD_BreachCharge"));
                    break;
                case ClassId::BumpMineProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[3], interfaces->localize->find("#SFUI_WPNHUD_BumpMine"));
                    break;
                case ClassId::DecoyProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[4], interfaces->localize->find("#SFUI_WPNHUD_Decoy"));
                    break;
                case ClassId::MolotovProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[5], interfaces->localize->find("#SFUI_WPNHUD_Molotov"));
                    break;
                case ClassId::SensorGrenadeProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[6], interfaces->localize->find("#SFUI_WPNHUD_TAGrenade"));
                    break;
                case ClassId::SmokeGrenadeProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[7], interfaces->localize->find("#SFUI_WPNHUD_SmokeGrenade"));
                    break;
                case ClassId::SnowballProjectile:
                    renderEntityEsp(entity, config->esp.projectiles[8], interfaces->localize->find("#SFUI_WPNHUD_Snowball"));
                    break;
                }
            }   
        }
    }
}

// Junk Code By Peatreat & Thaisen's Gen
void HfdDaChbsldpwFiGReoHblOzAvmMpgndmzJAhYXLFhNeBuduEKEpquWVReDePVmMxADwwhYtHd90571914() {     float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ40690922 = -523787201;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ4343953 = -974537894;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35611660 = -962529758;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45261628 = -792836228;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ41696607 = -137751603;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ93683049 = -868913378;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ69326267 = -933504804;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96044489 = 79813283;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ14102981 = -470761966;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44554490 = -288968427;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11835624 = -911602434;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ795704 = -694705059;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81740450 = -133040911;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ16437933 = -555857832;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ50863817 = -643326948;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ90579859 = -695354874;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ90162593 = 98268158;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ74120705 = -985954868;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45181416 = 98286364;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ7732483 = -628927191;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ41377463 = -364589096;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ56953161 = -825931414;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ31713972 = -607565229;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ60315996 = -153923531;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ70392072 = -989401855;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ97848732 = -124593623;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ12101266 = -724060679;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ63957715 = -265215115;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ60293587 = -872845477;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ98182473 = -293544817;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ82700672 = -301677398;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44907704 = -885923593;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ8188700 = -781192009;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ97629506 = 95317586;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ61629825 = 3210152;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11265165 = -899808538;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ43146563 = -444715743;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ19973379 = -46377856;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ25584977 = -383033600;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ7601850 = -954247403;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ24438919 = -757176013;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ56822247 = -879017066;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81251543 = -684629584;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ86838077 = -78245965;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ63834795 = -231584856;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35899076 = -880307914;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ86425680 = -20599204;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ58214493 = -948000126;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ58431722 = -457564926;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ25819515 = 81575588;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ82106180 = -788494133;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ54580746 = -782260351;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35744411 = -279644975;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ31406113 = 3754044;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ49194451 = -569435280;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ83737760 = -697855787;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ72629980 = -266972665;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ75295664 = -708606227;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ74869556 = -803434373;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ43847875 = 86842019;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81581784 = -44852699;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ5368553 = -568289689;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35750903 = -47341240;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ15920508 = -77217149;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ61853818 = -987291029;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ66927920 = 74321159;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ92607003 = -913513050;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ84110943 = -128358498;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ54808107 = -459067985;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ39598652 = -743518410;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ47433297 = -150639132;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ70189215 = -855353986;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ48535728 = -502921269;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ37579567 = 52533766;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ83293564 = -871751178;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ84555216 = -485572031;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ75701618 = -41301831;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44875895 = -429319265;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96481201 = -922338676;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ34492996 = -9093942;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11423052 = -3994419;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ53886773 = -776060554;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ5525993 = -807650190;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ34474073 = -854421066;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ16076293 = -505050684;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ28119926 = -519417048;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ9163293 = -506278619;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ76782586 = -684946053;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ48435055 = -335247135;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ77892064 = -298934061;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ38635185 = -532835874;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ67850899 = -736109516;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45103822 = -242943483;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81737102 = -369875620;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ26020065 = -809394705;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ19070366 = -88886325;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ21071344 = -731675826;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ65331035 = -507412435;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ24984260 = -90954936;    float CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96906875 = -523787201;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ40690922 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ4343953;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ4343953 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35611660;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35611660 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45261628;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45261628 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ41696607;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ41696607 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ93683049;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ93683049 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ69326267;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ69326267 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96044489;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96044489 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ14102981;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ14102981 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44554490;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44554490 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11835624;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11835624 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ795704;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ795704 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81740450;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81740450 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ16437933;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ16437933 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ50863817;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ50863817 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ90579859;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ90579859 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ90162593;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ90162593 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ74120705;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ74120705 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45181416;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45181416 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ7732483;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ7732483 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ41377463;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ41377463 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ56953161;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ56953161 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ31713972;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ31713972 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ60315996;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ60315996 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ70392072;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ70392072 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ97848732;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ97848732 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ12101266;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ12101266 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ63957715;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ63957715 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ60293587;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ60293587 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ98182473;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ98182473 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ82700672;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ82700672 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44907704;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44907704 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ8188700;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ8188700 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ97629506;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ97629506 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ61629825;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ61629825 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11265165;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11265165 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ43146563;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ43146563 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ19973379;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ19973379 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ25584977;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ25584977 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ7601850;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ7601850 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ24438919;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ24438919 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ56822247;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ56822247 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81251543;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81251543 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ86838077;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ86838077 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ63834795;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ63834795 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35899076;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35899076 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ86425680;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ86425680 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ58214493;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ58214493 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ58431722;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ58431722 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ25819515;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ25819515 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ82106180;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ82106180 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ54580746;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ54580746 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35744411;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35744411 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ31406113;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ31406113 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ49194451;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ49194451 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ83737760;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ83737760 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ72629980;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ72629980 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ75295664;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ75295664 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ74869556;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ74869556 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ43847875;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ43847875 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81581784;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81581784 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ5368553;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ5368553 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35750903;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ35750903 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ15920508;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ15920508 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ61853818;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ61853818 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ66927920;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ66927920 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ92607003;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ92607003 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ84110943;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ84110943 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ54808107;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ54808107 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ39598652;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ39598652 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ47433297;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ47433297 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ70189215;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ70189215 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ48535728;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ48535728 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ37579567;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ37579567 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ83293564;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ83293564 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ84555216;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ84555216 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ75701618;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ75701618 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44875895;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ44875895 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96481201;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96481201 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ34492996;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ34492996 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11423052;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ11423052 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ53886773;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ53886773 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ5525993;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ5525993 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ34474073;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ34474073 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ16076293;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ16076293 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ28119926;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ28119926 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ9163293;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ9163293 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ76782586;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ76782586 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ48435055;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ48435055 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ77892064;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ77892064 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ38635185;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ38635185 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ67850899;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ67850899 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45103822;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ45103822 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81737102;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ81737102 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ26020065;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ26020065 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ19070366;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ19070366 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ21071344;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ21071344 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ65331035;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ65331035 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ24984260;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ24984260 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96906875;     CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ96906875 = CtYjxFSrHfbxmrdnjDUsvkvAgBrgyWlLiNrdwhgxMXNHkANLTIyVYOhSyXCwOltktQPYzzqCWWgYWoPgngMpYQ40690922;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void neLYzqDsFtrtWsgeaOpgCImFDPmEtzTmKVwugmfxHMBtFYZsfZZkLYyfsGUkDzyGZEjBvmxoir25371096() {     float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63688550 = -312675628;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE54676806 = -736145956;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21112373 = -371823902;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE79270738 = -321860568;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33884871 = -307580608;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE25615845 = -315843933;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE82578038 = 92037009;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE11016830 = -361598335;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE49230083 = -838770920;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46674845 = -825704718;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE96178546 = -568611856;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE76894140 = -233695706;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE94001262 = 91672425;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE1140669 = -546118428;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE74780307 = -375226143;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73069260 = -935507631;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE41318316 = -349485221;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE36627418 = -267426791;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE32426880 = -527310308;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE7109737 = -60032659;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE40798211 = -687217581;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE90639153 = 21345475;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21054175 = -732558413;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18284122 = -921029964;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE7797001 = 9729277;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58972302 = -199525788;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72247501 = -256891208;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE782498 = -682262572;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE64436089 = -252826360;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68577130 = -731004604;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE52899698 = -524379281;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE49636001 = -365537770;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE61792951 = -992980004;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE77130638 = -610280685;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33505485 = -254994829;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58304684 = 35410360;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE51563564 = -5616695;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72618354 = -328903254;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE91327383 = -812604463;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58935173 = -169663278;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE82225575 = -900477726;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72070352 = -339216090;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE56806861 = -916461471;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE57095570 = -156159999;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE14698820 = -766371169;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE77585223 = -394801716;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE40857356 = -455583979;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24742101 = -341290791;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE78124662 = -344210905;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE84452914 = 96742762;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE30895968 = -545784407;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE44576126 = -352187479;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE31528222 = -634996705;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE39541350 = -143442690;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE84572884 = -563063405;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73049396 = -234021103;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33622631 = 96412457;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE2828251 = -450793938;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE71473738 = -231589846;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE74912569 = -8054821;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE53368343 = 41047274;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE81795541 = -225700419;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46580740 = -8771976;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE80652953 = -7766317;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE93775147 = -201325437;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46542545 = -103074086;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE15101190 = -240715703;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16870625 = -298046890;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE67635183 = -191123600;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16475624 = -310636504;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21505697 = -829890937;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68699961 = 79418033;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE45300035 = -454822329;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73491706 = -257647031;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24884161 = -159554933;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68727859 = -248001491;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33832293 = -62193055;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63958604 = -476398415;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE3585303 = -54658796;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE30211777 = -595469007;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18114946 = -743941810;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE47505400 = -915600418;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE22657835 = -238051667;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE79983175 = -249569122;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE37681163 = -85220197;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE8323572 = -72191802;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18107780 = -730541066;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE22251601 = -749537314;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE92557753 = 52782720;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE60456089 = 79026274;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24682053 = 38997903;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE48735313 = -554822757;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE1144617 = 2686591;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16414814 = -704549642;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE5566831 = -110710553;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE430034 = -574777308;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE25489612 = -230444114;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE76153908 = -808695155;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63320423 = -954834563;    float ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68156274 = -312675628;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63688550 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE54676806;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE54676806 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21112373;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21112373 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE79270738;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE79270738 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33884871;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33884871 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE25615845;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE25615845 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE82578038;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE82578038 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE11016830;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE11016830 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE49230083;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE49230083 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46674845;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46674845 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE96178546;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE96178546 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE76894140;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE76894140 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE94001262;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE94001262 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE1140669;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE1140669 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE74780307;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE74780307 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73069260;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73069260 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE41318316;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE41318316 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE36627418;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE36627418 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE32426880;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE32426880 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE7109737;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE7109737 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE40798211;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE40798211 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE90639153;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE90639153 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21054175;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21054175 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18284122;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18284122 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE7797001;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE7797001 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58972302;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58972302 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72247501;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72247501 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE782498;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE782498 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE64436089;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE64436089 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68577130;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68577130 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE52899698;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE52899698 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE49636001;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE49636001 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE61792951;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE61792951 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE77130638;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE77130638 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33505485;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33505485 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58304684;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58304684 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE51563564;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE51563564 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72618354;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72618354 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE91327383;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE91327383 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58935173;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE58935173 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE82225575;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE82225575 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72070352;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE72070352 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE56806861;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE56806861 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE57095570;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE57095570 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE14698820;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE14698820 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE77585223;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE77585223 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE40857356;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE40857356 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24742101;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24742101 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE78124662;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE78124662 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE84452914;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE84452914 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE30895968;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE30895968 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE44576126;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE44576126 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE31528222;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE31528222 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE39541350;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE39541350 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE84572884;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE84572884 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73049396;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73049396 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33622631;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33622631 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE2828251;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE2828251 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE71473738;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE71473738 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE74912569;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE74912569 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE53368343;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE53368343 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE81795541;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE81795541 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46580740;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46580740 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE80652953;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE80652953 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE93775147;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE93775147 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46542545;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE46542545 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE15101190;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE15101190 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16870625;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16870625 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE67635183;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE67635183 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16475624;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16475624 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21505697;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE21505697 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68699961;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68699961 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE45300035;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE45300035 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73491706;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE73491706 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24884161;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24884161 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68727859;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68727859 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33832293;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE33832293 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63958604;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63958604 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE3585303;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE3585303 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE30211777;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE30211777 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18114946;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18114946 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE47505400;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE47505400 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE22657835;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE22657835 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE79983175;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE79983175 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE37681163;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE37681163 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE8323572;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE8323572 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18107780;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE18107780 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE22251601;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE22251601 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE92557753;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE92557753 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE60456089;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE60456089 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24682053;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE24682053 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE48735313;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE48735313 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE1144617;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE1144617 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16414814;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE16414814 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE5566831;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE5566831 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE430034;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE430034 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE25489612;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE25489612 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE76153908;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE76153908 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63320423;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63320423 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68156274;     ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE68156274 = ZqHtzeptYUZiblWRBaRvtINvTBoYxtbkBTzHXyHpuKaegrdxedkXXnyQdLaypQvfsphHmOtDTbAjhjPcXruUIE63688550;}
// Junk Finished
