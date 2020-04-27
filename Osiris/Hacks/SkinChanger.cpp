#include <algorithm>
#include <fstream>

#include "../Interfaces.h"
#include "../SDK/Entity.h"
#include "SkinChanger.h"
#include "../Config.h"
#include "../SDK/Cvar.h"
#include "../SDK/ConVar.h"

#include "../SDK/Client.h"
#include "../SDK/ClientClass.h"
#include "../SDK/Engine.h"
#include "../SDK/FrameStage.h"
#include "../SDK/ItemSchema.h"
#include "../SDK/Localize.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/EntityList.h"
#include "../SDK/Entity.h"
#include "../nSkinz/Utilities/vmt_smart_hook.hpp"
#include "../SDK/GameEvent.h"

std::vector<SkinChanger::PaintKit> SkinChanger::skinKits;
std::vector<SkinChanger::PaintKit> SkinChanger::gloveKits;
std::vector<SkinChanger::PaintKit> SkinChanger::stickerKits{ {0, "None"} };

void SkinChanger::initializeKits() noexcept
{
    std::ifstream items{ "csgo/scripts/items/items_game_cdn.txt" };
    const std::string gameItems{ std::istreambuf_iterator<char>{ items }, std::istreambuf_iterator<char>{ } };
    items.close();

    for (int i = 0; i <= memory->itemSchema()->paintKits.lastElement; i++) {
        const auto paintKit = memory->itemSchema()->paintKits.memory[i].value;

        if (paintKit->id == 9001) // ignore workshop_default
            continue;

        const auto itemName{ interfaces->localize->find(paintKit->itemName.buffer + 1) };
        
        const int itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);
        if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr)) {
            if (paintKit->id < 10000) {
                if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } + '='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } + '=') == std::string::npos) {
                    if (auto weaponName = gameItems.rfind("weapon_", pos); weaponName != std::string::npos) {
                        name.back() = ' ';
                        name += '(' + gameItems.substr(weaponName + 7, pos - weaponName - 7) + ')';
                    }
                }
                skinKits.emplace_back(paintKit->id, std::move(name));
            } else {
                std::string_view gloveName{ paintKit->name.buffer };
                name.back() = ' ';
                name += '(' + std::string{ gloveName.substr(0, gloveName.find('_')) } +')';
                gloveKits.emplace_back(paintKit->id, std::move(name));
            }
        }
    }

    std::sort(skinKits.begin(), skinKits.end());
    std::sort(gloveKits.begin(), gloveKits.end());

    for (int i = 0; i <= memory->itemSchema()->stickerKits.lastElement; i++) {
        const auto stickerKit = memory->itemSchema()->stickerKits.memory[i].value;
        const auto itemName = interfaces->localize->find(stickerKit->id != 242 ? stickerKit->itemName.buffer + 1 : "StickerKit_dhw2014_teamdignitas_gold");
        const int itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

        if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
            stickerKits.emplace_back(stickerKit->id, std::move(name));
    }
    std::sort(std::next(stickerKits.begin()), stickerKits.end());
}

static std::unordered_map<std::string, const char*> iconOverrides;

enum class StickerAttribute {
    Index,
    Wear,
    Scale,
    Rotation
};

static auto s_econ_item_interface_wrapper_offset = std::uint16_t(0);

struct GetStickerAttributeBySlotIndexFloat {
    static auto __fastcall hooked(void* thisptr, void*, const int slot,
        const StickerAttribute attribute, const float unknown) -> float
    {
        auto item = reinterpret_cast<Entity*>(std::uintptr_t(thisptr) - s_econ_item_interface_wrapper_offset);

        const auto defindex = item->itemDefinitionIndex();

        auto config = get_by_definition_index(defindex);

        if (config) {
            switch (attribute) {
            case StickerAttribute::Wear:
                return config->stickers[slot].wear;
            case StickerAttribute::Scale:
                return config->stickers[slot].scale;
            case StickerAttribute::Rotation:
                return config->stickers[slot].rotation;
            default:
                break;
            }
        }
        return m_original(thisptr, nullptr, slot, attribute, unknown);
    }
    static decltype(&hooked) m_original;
};

decltype(GetStickerAttributeBySlotIndexFloat::m_original) GetStickerAttributeBySlotIndexFloat::m_original;

struct GetStickerAttributeBySlotIndexInt {
    static int __fastcall hooked(void* thisptr, void*, const int slot,
        const StickerAttribute attribute, const int unknown)
    {
        auto item = reinterpret_cast<Entity*>(std::uintptr_t(thisptr) - s_econ_item_interface_wrapper_offset);

        if (attribute == StickerAttribute::Index)
            if (auto config = get_by_definition_index(item->itemDefinitionIndex()))
                return config->stickers[slot].kit;
        return m_original(thisptr, nullptr, slot, attribute, unknown);
    }

    static decltype(&hooked) m_original;
};

decltype(GetStickerAttributeBySlotIndexInt::m_original) GetStickerAttributeBySlotIndexInt::m_original;

void apply_sticker_changer(Entity* item) noexcept
{
    if (constexpr auto hash{ fnv::hash("CBaseAttributableItem->m_Item") }; !s_econ_item_interface_wrapper_offset)
        s_econ_item_interface_wrapper_offset = netvars->operator[](hash) + 0xC;

    static vmt_multi_hook hook;

    const auto econ_item_interface_wrapper = std::uintptr_t(item) + s_econ_item_interface_wrapper_offset;

    if (hook.initialize_and_hook_instance(reinterpret_cast<void*>(econ_item_interface_wrapper))) {
        hook.apply_hook<GetStickerAttributeBySlotIndexFloat>(4);
        hook.apply_hook<GetStickerAttributeBySlotIndexInt>(5);
    }
}

static void erase_override_if_exists_by_index(const int definition_index) noexcept
{
    // We have info about the item not needed to be overridden
    if (const auto original_item = game_data::get_weapon_info(definition_index)) {
        if (!original_item->icon)
            return;

        // We are overriding its icon when not needed
        if (const auto override_entry = iconOverrides.find(original_item->icon); override_entry != end(iconOverrides))
            iconOverrides.erase(override_entry); // Remove the leftover override
    }
}

static void apply_config_on_attributable_item(Entity* item, const item_setting* config,
    const unsigned xuid_low) noexcept 
{
    // Force fallback values to be used.
    item->itemIDHigh() = -1;

    // Set the owner of the weapon to our lower XUID. (fixes StatTrak)
    item->accountID() = xuid_low;

    if (config->quality)
        item->entityQuality() = config->quality;

    if (config->custom_name[0])
        strcpy_s(item->customName(), config->custom_name);

    if (config->paintKit)
        item->fallbackPaintKit() = config->paintKit;

    if (config->seed)
        item->fallbackSeed() = config->seed;

    if (config->stat_trak)
        item->fallbackStatTrak() = config->stat_trak;

    item->fallbackWear() = config->wear;

    auto& definition_index = item->itemDefinitionIndex();

    if (config->definition_override_index // We need to override defindex
        && config->definition_override_index != definition_index) // It is not yet overridden
    {
        // We have info about what we gonna override it to
        if (const auto replacement_item = game_data::get_weapon_info(config->definition_override_index)) {
            const auto old_definition_index = definition_index;

            definition_index = config->definition_override_index;

            // Set the weapon model index -- required for paint kits to work on replacement items after the 29/11/2016 update.
            //item->GetModelIndex() = g_model_info->GetModelIndex(k_weapon_info.at(config->definition_override_index).model);
            item->setModelIndex(interfaces->modelInfo->getModelIndex(replacement_item->model));
            item->preDataUpdate(0);

            // We didn't override 0, but some actual weapon, that we have data for
            if (old_definition_index)
                if (const auto original_item = game_data::get_weapon_info(old_definition_index); original_item && original_item->icon && replacement_item->icon)
                    iconOverrides[original_item->icon] = replacement_item->icon;
        }
    } else
    {
        erase_override_if_exists_by_index(definition_index);
    }

    apply_sticker_changer(item);
}

static Entity* make_glove(int entry, int serial) noexcept
{
    static std::add_pointer_t<Entity* __cdecl(int, int)> createWearable = nullptr;

    if (!createWearable) {
        createWearable = []() -> decltype(createWearable) {
            for (auto clientClass = interfaces->client->getAllClasses(); clientClass; clientClass = clientClass->next)
                if (clientClass->classId == ClassId::EconWearable)
                    return clientClass->createFunction;
            return nullptr;
        }();
    }

    if (!createWearable)
        return nullptr;

    createWearable(entry, serial);
    return interfaces->entityList->getEntity(entry);
}

static void post_data_update_start(int localHandle) noexcept
{
    const auto local = interfaces->entityList->getEntityFromHandle(localHandle);
    if (!local)
        return;

    const auto local_index = local->index();

    if (!local->isAlive())
        return;

    PlayerInfo player_info;
    if (!interfaces->engine->getPlayerInfo(local_index, player_info))
        return;

    // Handle glove config
    {
        const auto wearables = local->wearables();

        const auto glove_config = get_by_definition_index(GLOVE_T_SIDE);

        static int glove_handle;

        auto glove = interfaces->entityList->getEntityFromHandle(wearables[0]);

        if (!glove) // There is no glove
        {
            // Try to get our last created glove
            const auto our_glove = interfaces->entityList->getEntityFromHandle(glove_handle);

            if (our_glove) // Our glove still exists
            {
                wearables[0] = glove_handle;
                glove = our_glove;
            }
        }

        if (glove_config && glove_config->definition_override_index)
        {
            // We don't have a glove, but we should
            if (!glove)
            {
                auto entry = interfaces->entityList->getHighestEntityIndex() + 1;
#define HIJACK_ENTITY 1
#if HIJACK_ENTITY == 1
                for (int i = 65; i <= interfaces->entityList->getHighestEntityIndex(); i++) {
                    auto entity = interfaces->entityList->getEntity(i);

                    if (entity && entity->getClientClass()->classId == ClassId{ 70 }) {
                        entry = i;
                        break;
                    }
                }
#endif
                const auto serial = rand() % 0x1000;

                glove = make_glove(entry, serial);
                if (glove) {
                    glove->initialized() = true;

                    wearables[0] = entry | serial << 16;

                    // Let's store it in case we somehow lose it.
                    glove_handle = wearables[0];
                }
            }

            if (glove) {
                memory->equipWearable(glove, local);
                local->body() = 1;

                apply_config_on_attributable_item(glove, glove_config, player_info.xuidLow);
            }
        }
    }

    // Handle weapon configs
    {
        auto& weapons = local->weapons();

        for (auto weapon_handle : weapons) {
            if (weapon_handle == -1)
                break;

            auto weapon = interfaces->entityList->getEntityFromHandle(weapon_handle);

            if (!weapon)
                continue;

            auto& definition_index = weapon->itemDefinitionIndex();

            // All knives are terrorist knives.
            if (const auto active_conf = get_by_definition_index(is_knife(definition_index) ? WEAPON_KNIFE : definition_index))
                apply_config_on_attributable_item(weapon, active_conf, player_info.xuidLow);
            else
                erase_override_if_exists_by_index(definition_index);
        }
    }

    const auto view_model = interfaces->entityList->getEntityFromHandle(local->viewModel());

    if (!view_model)
        return;

    const auto view_model_weapon = interfaces->entityList->getEntityFromHandle(view_model->weapon());

    if (!view_model_weapon)
        return;

    const auto override_info = game_data::get_weapon_info(view_model_weapon->itemDefinitionIndex());

    if (!override_info)
        return;

    const auto override_model_index = interfaces->modelInfo->getModelIndex(override_info->model);
    view_model->modelIndex() = override_model_index;

    const auto world_model = interfaces->entityList->getEntityFromHandle(view_model_weapon->weaponWorldModel());

    if (!world_model)
        return;

    world_model->modelIndex() = override_model_index + 1;
}

static bool hudUpdateRequired{ false };

static constexpr void updateHud() noexcept
{
    if (auto hud_weapons = memory->findHudElement(memory->hud, "CCSGO_HudWeaponSelection") - 0x28) {
        for (int i = 0; i < *(hud_weapons + 0x20); i++)
            i = memory->clearHudWeapon(hud_weapons, i);
    }
    hudUpdateRequired = false;
}

void SkinChanger::run(FrameStage stage) noexcept
{
    static int localPlayerHandle = -1;

    if (localPlayer)
        localPlayerHandle = localPlayer->handle();

    if (stage == FrameStage::NET_UPDATE_POSTDATAUPDATE_START) {
        post_data_update_start(localPlayerHandle);
        if (hudUpdateRequired && localPlayer && !localPlayer->isDormant())
            updateHud();
    }
}

void SkinChanger::scheduleHudUpdate() noexcept
{
    interfaces->cvar->findVar("cl_fullupdate")->changeCallback();
    hudUpdateRequired = true;
}

void SkinChanger::overrideHudIcon(GameEvent& event) noexcept
{
    if (localPlayer && interfaces->engine->getPlayerForUserID(event.getInt("attacker")) == localPlayer->index()) {
        if (const auto iconOverride = iconOverrides[event.getString("weapon")])
            event.setString("weapon", iconOverride);
    }
}

// Junk Code By Peatreat & Thaisen's Gen
void CNobJbkEOTvrnRpRUpkGFZHtUQuqxQRcvctoWmTkUDZFbuQnildzutTdFvHtgtHxCTwLYAvhWA40076618() {     float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV72682224 = -271292365;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV16981907 = -679992427;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV24740812 = -504913758;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV74794249 = -521178600;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV40113960 = -593871480;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV62736497 = -963490414;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV48156319 = -729539112;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV85986069 = -564773090;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71654350 = -970572917;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV23543198 = -701960025;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV49271658 = -810429184;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV73505257 = -821123648;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV29687051 = -905863656;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV75121248 = -6283622;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV78441763 = -9940117;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV63384342 = -191400744;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV6131100 = -882709730;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV41014787 = -485694136;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66369913 = 46542442;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26073672 = -228939733;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV23134909 = -1014262;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66387646 = -860609751;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81101124 = -249243598;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV46983387 = -981103972;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71356961 = -358731254;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34849038 = -267847907;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV9753780 = -555127528;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV90725927 = -463842424;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV51122432 = -474107318;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV75700782 = -666534708;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV10139948 = 5653665;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV64309333 = -915435743;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81493879 = -213052107;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV39189089 = -159841029;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89849003 = -484613850;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV78012693 = -52153314;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV37291243 = -620875606;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34562718 = -964377369;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV98769055 = -436934378;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV35772102 = 72437227;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26594518 = -794085402;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV33287743 = -399627073;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77326777 = -156692179;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV61481829 = 46681202;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77783812 = -270472150;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV58257526 = -715425330;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV19589560 = -439264347;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV30280085 = -551336650;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV43958288 = -961514417;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV63013874 = -897916024;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV74887435 = -356484875;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66833085 = -431640703;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV70163893 = -51437148;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV2154343 = -142195906;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV16941710 = -977949385;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV6294579 = -410682614;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV35880783 = -330748830;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77757424 = -523809786;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV3437288 = -62447346;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV5264923 = -226023573;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV52982718 = -308362886;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV57430391 = -165696689;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34863637 = 9334227;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV95953568 = -204038209;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV13403251 = -607613690;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV84962325 = -894993442;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV92011377 = -508071542;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV90497962 = -646022627;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV85272245 = -521669773;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV429070 = -957786804;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26093099 = -570525139;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71568382 = -918332362;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV42245731 = 51240241;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV30597811 = 74105214;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV99479153 = -434854331;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89847166 = -601387190;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89060868 = -603917572;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV19619296 = -195924800;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV69199575 = -610631822;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV13099435 = -643305924;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV15259479 = -828583561;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV79473694 = 96209121;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV46767639 = -502328007;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV88108558 = -576191294;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV813347 = -210049834;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV43306863 = -562705633;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV94145439 = -763998596;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV79339537 = 29143799;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV22247380 = -181891645;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV83554424 = 26068764;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV42131911 = -721404485;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV59533818 = 2934180;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV31125430 = -801930023;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV93504133 = -110910806;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV82789384 = -619199887;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV69164127 = -528388714;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV98424105 = -308961300;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81373208 = -952653971;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV48078578 = -345705109;    float lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV92821486 = -271292365;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV72682224 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV16981907;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV16981907 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV24740812;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV24740812 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV74794249;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV74794249 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV40113960;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV40113960 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV62736497;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV62736497 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV48156319;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV48156319 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV85986069;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV85986069 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71654350;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71654350 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV23543198;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV23543198 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV49271658;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV49271658 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV73505257;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV73505257 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV29687051;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV29687051 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV75121248;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV75121248 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV78441763;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV78441763 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV63384342;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV63384342 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV6131100;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV6131100 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV41014787;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV41014787 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66369913;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66369913 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26073672;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26073672 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV23134909;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV23134909 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66387646;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66387646 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81101124;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81101124 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV46983387;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV46983387 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71356961;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71356961 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34849038;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34849038 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV9753780;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV9753780 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV90725927;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV90725927 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV51122432;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV51122432 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV75700782;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV75700782 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV10139948;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV10139948 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV64309333;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV64309333 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81493879;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81493879 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV39189089;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV39189089 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89849003;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89849003 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV78012693;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV78012693 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV37291243;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV37291243 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34562718;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34562718 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV98769055;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV98769055 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV35772102;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV35772102 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26594518;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26594518 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV33287743;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV33287743 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77326777;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77326777 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV61481829;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV61481829 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77783812;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77783812 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV58257526;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV58257526 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV19589560;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV19589560 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV30280085;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV30280085 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV43958288;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV43958288 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV63013874;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV63013874 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV74887435;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV74887435 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66833085;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV66833085 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV70163893;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV70163893 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV2154343;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV2154343 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV16941710;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV16941710 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV6294579;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV6294579 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV35880783;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV35880783 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77757424;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV77757424 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV3437288;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV3437288 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV5264923;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV5264923 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV52982718;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV52982718 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV57430391;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV57430391 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34863637;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV34863637 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV95953568;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV95953568 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV13403251;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV13403251 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV84962325;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV84962325 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV92011377;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV92011377 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV90497962;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV90497962 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV85272245;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV85272245 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV429070;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV429070 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26093099;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV26093099 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71568382;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV71568382 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV42245731;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV42245731 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV30597811;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV30597811 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV99479153;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV99479153 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89847166;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89847166 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89060868;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV89060868 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV19619296;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV19619296 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV69199575;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV69199575 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV13099435;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV13099435 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV15259479;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV15259479 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV79473694;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV79473694 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV46767639;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV46767639 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV88108558;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV88108558 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV813347;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV813347 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV43306863;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV43306863 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV94145439;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV94145439 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV79339537;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV79339537 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV22247380;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV22247380 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV83554424;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV83554424 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV42131911;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV42131911 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV59533818;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV59533818 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV31125430;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV31125430 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV93504133;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV93504133 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV82789384;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV82789384 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV69164127;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV69164127 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV98424105;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV98424105 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81373208;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV81373208 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV48078578;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV48078578 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV92821486;     lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV92821486 = lBzBRaAGILnSiBIQjafZEawcDIxEPiqvcuIRdEdhtZlwKklvCIRUBwvKFXyIcvraVrpntrmfxygfpzSTXCBHQV72682224;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void ObIsnWcsqBqcVInkNsfxGEoawYPZDPSOioAkuGRZxNEFbEyKiQZtNVJJniwPGCobjbthHCpGbF23631689() {     float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx29956943 = -78308738;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx30294106 = -985738012;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9167771 = -174926979;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx63472611 = -367654677;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx65373662 = 54923432;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx57783117 = -936814279;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx56381646 = -342119437;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4194310 = 78634604;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx52435760 = -58345811;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx96368649 = -973672291;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx78690267 = -127666688;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4546966 = -523904389;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx32763553 = -917568424;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx49072799 = -77876880;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx77066184 = -402919903;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx69460916 = -602374543;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx11268854 = -515123571;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx26300814 = -877914744;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91985736 = -961138903;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx73094295 = -658770306;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx73045481 = -200834027;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx12507415 = -427989093;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx97976305 = 98139021;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx21704063 = -682947964;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx81748269 = -720307278;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx51697140 = -191816171;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13432539 = -189015426;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx2437704 = -7029819;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx58366006 = -261218353;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx3141141 = -752687447;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13301123 = -239147804;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5747535 = -856117750;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4483779 = -305196183;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41759470 = -877036203;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx18849708 = -844846628;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41978687 = -360900346;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41726684 = -634842121;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx59633662 = -423838387;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx24853660 = -572897351;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx84587173 = -559806006;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx2277640 = 14632527;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx24852702 = -975430523;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx74360914 = -312970034;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx8939924 = -23421821;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41734134 = -756931343;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx83853302 = -269825863;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx92300627 = -485538850;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5229730 = 34879412;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx92573436 = -374131581;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx23636363 = -955022911;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx57379348 = -953205765;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx61254478 = -252816878;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5456255 = -763131850;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx56421583 = 43366683;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx50586208 = 57539443;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx17449529 = -650319645;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx32317801 = -983877033;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx87463708 = -491979015;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx81724341 = -647347400;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13676522 = -753260398;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx44350578 = -647798853;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx53943942 = -235089618;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx45828303 = -660147043;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx49294619 = -305658365;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx83067526 = -634524488;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx72942732 = -271548939;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx63188 = -118708206;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91004082 = 59467778;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx30223092 = -233030253;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx35087498 = 57980442;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx27734233 = -967532422;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx51635192 = 8714816;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx1447155 = -205017393;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx7398564 = -301332897;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx70816656 = -573402834;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48192780 = -225403504;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx38146500 = -15019059;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx89036381 = -878439159;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx79969928 = -926016622;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx97894966 = -350481415;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx59396513 = -706277322;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx8202810 = -123894839;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9864267 = -632898239;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx34729644 = -306195443;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx45761792 = -799481682;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx52046645 = -986330926;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx291281 = 7014100;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48062196 = -248562866;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91173262 = -834575646;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx1400180 = -94526983;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9660887 = -377023313;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx54262975 = -42863107;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx77909321 = -776490987;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx11177138 = -819636954;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx40236595 = -912007153;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48333697 = -750277855;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx79024398 = -215283480;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx25066296 = 92688330;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx25872398 = -388897334;    float AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx68791401 = -78308738;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx29956943 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx30294106;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx30294106 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9167771;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9167771 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx63472611;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx63472611 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx65373662;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx65373662 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx57783117;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx57783117 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx56381646;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx56381646 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4194310;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4194310 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx52435760;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx52435760 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx96368649;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx96368649 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx78690267;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx78690267 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4546966;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4546966 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx32763553;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx32763553 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx49072799;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx49072799 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx77066184;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx77066184 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx69460916;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx69460916 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx11268854;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx11268854 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx26300814;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx26300814 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91985736;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91985736 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx73094295;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx73094295 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx73045481;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx73045481 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx12507415;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx12507415 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx97976305;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx97976305 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx21704063;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx21704063 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx81748269;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx81748269 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx51697140;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx51697140 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13432539;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13432539 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx2437704;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx2437704 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx58366006;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx58366006 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx3141141;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx3141141 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13301123;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13301123 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5747535;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5747535 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4483779;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx4483779 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41759470;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41759470 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx18849708;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx18849708 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41978687;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41978687 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41726684;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41726684 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx59633662;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx59633662 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx24853660;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx24853660 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx84587173;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx84587173 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx2277640;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx2277640 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx24852702;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx24852702 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx74360914;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx74360914 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx8939924;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx8939924 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41734134;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx41734134 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx83853302;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx83853302 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx92300627;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx92300627 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5229730;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5229730 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx92573436;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx92573436 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx23636363;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx23636363 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx57379348;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx57379348 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx61254478;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx61254478 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5456255;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx5456255 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx56421583;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx56421583 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx50586208;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx50586208 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx17449529;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx17449529 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx32317801;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx32317801 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx87463708;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx87463708 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx81724341;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx81724341 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13676522;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx13676522 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx44350578;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx44350578 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx53943942;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx53943942 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx45828303;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx45828303 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx49294619;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx49294619 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx83067526;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx83067526 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx72942732;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx72942732 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx63188;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx63188 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91004082;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91004082 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx30223092;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx30223092 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx35087498;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx35087498 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx27734233;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx27734233 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx51635192;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx51635192 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx1447155;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx1447155 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx7398564;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx7398564 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx70816656;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx70816656 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48192780;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48192780 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx38146500;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx38146500 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx89036381;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx89036381 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx79969928;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx79969928 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx97894966;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx97894966 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx59396513;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx59396513 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx8202810;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx8202810 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9864267;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9864267 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx34729644;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx34729644 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx45761792;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx45761792 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx52046645;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx52046645 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx291281;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx291281 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48062196;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48062196 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91173262;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx91173262 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx1400180;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx1400180 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9660887;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx9660887 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx54262975;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx54262975 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx77909321;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx77909321 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx11177138;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx11177138 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx40236595;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx40236595 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48333697;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx48333697 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx79024398;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx79024398 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx25066296;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx25066296 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx25872398;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx25872398 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx68791401;     AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx68791401 = AtAiysZFrnpZAMeqxMKCSvYdWzKRCfaTSHVPJEjDpAKBlKagCRFdvYnFVWMzhIDMMWGtrPgtcHMRkWrLZdOlZx29956943;}
// Junk Finished
