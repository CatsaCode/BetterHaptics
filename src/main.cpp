#include "main.hpp"

#include "scotland2/shared/modloader.h"
#include "config-utils/shared/config-utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"
#include "bsml/shared/BSML.hpp"

#include "GlobalNamespace/NoteCutHapticEffect.hpp"
#include "GlobalNamespace/SliderHapticFeedbackInteractionEffect.hpp"
#include "GlobalNamespace/ObstacleSaberSparkleEffectManager.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SaberClashEffect.hpp"

#include "BeatSaber/Haptics/OculusAdvancedHapticFeedbackPlayer.hpp"

#include "VRUIControls/VRInputModule.hpp"

using HapticPresetSO = BeatSaber::Haptics::HapticPresetSO;

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

DECLARE_CONFIG(BetterHapticsConfig) {
    CONFIG_VALUE(enabled, bool, "enabled", true);
    CONFIG_VALUE(uiStrength, float, "uiStrength", 1);
    CONFIG_VALUE(noteStrength, float, "noteStrength", 1);
    CONFIG_VALUE(chainHeadStrength, float, "chainHeadStrength", 1);
    CONFIG_VALUE(chainLinkStrength, float, "chainLinkStrength", 1);
    CONFIG_VALUE(arcHeadStrength, float, "arcHeadStrength", 1);
    CONFIG_VALUE(arcTailStrength, float, "arcTailStrength", 1);
    CONFIG_VALUE(arcHeadTailStrength, float, "arcHeadTailStrength", 1);
    CONFIG_VALUE(bombStrength, float, "bombStrength", 1);
    CONFIG_VALUE(badCutStrength, float, "badCutStrength", 1);
    CONFIG_VALUE(arcStrength, float, "arcStrength", 0.75);
    CONFIG_VALUE(wallStrength, float, "wallStrength", 0.75);
    CONFIG_VALUE(saberClashStrength, float, "saberClashStrength", 0.75);
};

HapticPresetSO* copyHapticPreset(HapticPresetSO* hapticPreset) {
    auto* newHapticPreset = HapticPresetSO::New_ctor();
    newHapticPreset->_duration = hapticPreset->_duration;
    newHapticPreset->_strength = hapticPreset->_strength;
    newHapticPreset->_frequency = hapticPreset->_frequency;
    newHapticPreset->_continuous = hapticPreset->_continuous;
    newHapticPreset->_useAdvancedHapticsOnSupportedPlatforms = hapticPreset->_useAdvancedHapticsOnSupportedPlatforms;
    newHapticPreset->_oculusHapticsClip = hapticPreset->_oculusHapticsClip;
    newHapticPreset->_priority = hapticPreset->_priority;
    newHapticPreset->_overrideForTouchController = hapticPreset->_overrideForTouchController;
    newHapticPreset->_touchControllerOverrideHapticsClip = hapticPreset->_touchControllerOverrideHapticsClip;
    newHapticPreset->_hasOculusHapticsClip = hapticPreset->_hasOculusHapticsClip;
    return newHapticPreset;
}

void setHapticPresetOverflowStrength(HapticPresetSO* hapticPreset, float strength, float defaultDuration) {
    if (strength <= 1) {
        hapticPreset->_strength = strength;
        hapticPreset->_duration = defaultDuration;
    } else {
        hapticPreset->_strength = 1;
        hapticPreset->_duration = defaultDuration + (strength - 1);
    }
}

MAKE_HOOK_MATCH(
    OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset,
    &BeatSaber::Haptics::OculusAdvancedHapticFeedbackPlayer::CanPlayHapticPreset, bool,
    BeatSaber::Haptics::OculusAdvancedHapticFeedbackPlayer *self, HapticPresetSO *hapticPreset, UnityEngine::XR::XRNode node
) {
    if (getBetterHapticsConfig().enabled.GetValue())
        return false;

    return OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset(self, hapticPreset, node);
}

MAKE_HOOK_MATCH(
    VRInputModule_HandlePointerExitAndEnter,
    &VRUIControls::VRInputModule::HandlePointerExitAndEnter, void,
    VRUIControls::VRInputModule* self, UnityEngine::EventSystems::PointerEventData* pointerEventData, UnityEngine::GameObject* newEnterTarget
) {
    if (!getBetterHapticsConfig().enabled.GetValue())
        return VRInputModule_HandlePointerExitAndEnter(self, pointerEventData, newEnterTarget);

    constexpr float DEFAULT_DURATION = 0.01;
    setHapticPresetOverflowStrength(self->_rumblePreset, getBetterHapticsConfig().uiStrength.GetValue(), DEFAULT_DURATION);

    VRInputModule_HandlePointerExitAndEnter(self, pointerEventData, newEnterTarget);
}

MAKE_HOOK_MATCH(
    NoteCutHapticEffect_HitNote,
    &GlobalNamespace::NoteCutHapticEffect::HitNote, void,
    GlobalNamespace::NoteCutHapticEffect* self, GlobalNamespace::SaberType saberType, GlobalNamespace::NoteCutHapticEffect::Type type
) {
    if (!getBetterHapticsConfig().enabled.GetValue())
        return NoteCutHapticEffect_HitNote(self, saberType, type);

    if (self->_arcHeadPreset == self->_normalPreset)
        self->_arcHeadPreset = copyHapticPreset(self->_normalPreset);
    if (self->_arcTailPreset == self->_normalPreset)
        self->_arcTailPreset = copyHapticPreset(self->_normalPreset);
    if (self->_arcHeadAndTailPreset == self->_normalPreset)
        self->_arcHeadAndTailPreset = copyHapticPreset(self->_normalPreset);

    constexpr float DEFAULT_DURATION = 0.13;
    setHapticPresetOverflowStrength(self->_normalPreset, getBetterHapticsConfig().noteStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_chainHeadPreset, getBetterHapticsConfig().chainHeadStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_chainLinkPreset, getBetterHapticsConfig().chainLinkStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_arcHeadPreset, getBetterHapticsConfig().arcHeadStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_arcTailPreset, getBetterHapticsConfig().arcTailStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_arcHeadAndTailPreset, getBetterHapticsConfig().arcHeadTailStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_bombPreset, getBetterHapticsConfig().bombStrength.GetValue(), DEFAULT_DURATION);
    setHapticPresetOverflowStrength(self->_badCutPreset, getBetterHapticsConfig().badCutStrength.GetValue(), DEFAULT_DURATION);

    NoteCutHapticEffect_HitNote(self, saberType, type);
}

MAKE_HOOK_MATCH(
    SliderHapticFeedbackInteractionEffect_Vibrate,
    &GlobalNamespace::SliderHapticFeedbackInteractionEffect::Vibrate, void,
    GlobalNamespace::SliderHapticFeedbackInteractionEffect* self
) {
    if (!getBetterHapticsConfig().enabled.GetValue())
        return SliderHapticFeedbackInteractionEffect_Vibrate(self);

    self->_hapticPreset->_strength = getBetterHapticsConfig().arcStrength.GetValue();

    SliderHapticFeedbackInteractionEffect_Vibrate(self);
}

MAKE_HOOK_MATCH(
    ObstacleSaberSparkleEffectManager_Update,
    &GlobalNamespace::ObstacleSaberSparkleEffectManager::Update, void,
    GlobalNamespace::ObstacleSaberSparkleEffectManager* self
) {
    if (!getBetterHapticsConfig().enabled.GetValue())
        return ObstacleSaberSparkleEffectManager_Update(self);

    self->_rumblePreset->_strength = getBetterHapticsConfig().wallStrength.GetValue();

    ObstacleSaberSparkleEffectManager_Update(self);
}

MAKE_HOOK_MATCH(
    SaberClashEffect_LateUpdate,
    &GlobalNamespace::SaberClashEffect::LateUpdate, void,
    GlobalNamespace::SaberClashEffect* self
) {
    if (!getBetterHapticsConfig().enabled.GetValue())
        return SaberClashEffect_LateUpdate(self);

    self->_rumblePreset->_strength = getBetterHapticsConfig().saberClashStrength.GetValue();

    SaberClashEffect_LateUpdate(self);
}

void viewControllerDidActivate(HMUI::ViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    auto* container = BSML::Lite::CreateScrollableSettingsContainer(self->transform);

    BSML::Lite::CreateToggle(container, "Enable", getBetterHapticsConfig().enabled.GetValue(), [](bool value){getBetterHapticsConfig().enabled.SetValue(value);});

    constexpr float increment = 0.05;
    auto* uiSlider = BSML::Lite::CreateSliderSetting(container, "UI", increment, getBetterHapticsConfig().uiStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().uiStrength.SetValue(value);});
    auto* noteSlider = BSML::Lite::CreateSliderSetting(container, "Notes", increment, getBetterHapticsConfig().noteStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().noteStrength.SetValue(value);});
    auto* chainHeadSlider = BSML::Lite::CreateSliderSetting(container, "Chain Heads", increment, getBetterHapticsConfig().chainHeadStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().chainHeadStrength.SetValue(value);});
    auto* chainLinkSlider = BSML::Lite::CreateSliderSetting(container, "Chain Links", increment, getBetterHapticsConfig().chainLinkStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().chainLinkStrength.SetValue(value);});
    auto* arcHeadSlider = BSML::Lite::CreateSliderSetting(container, "Arc Heads", increment, getBetterHapticsConfig().arcHeadStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().arcHeadStrength.SetValue(value);});
    auto* arcTailSlider = BSML::Lite::CreateSliderSetting(container, "Arc Tails", increment, getBetterHapticsConfig().arcTailStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().arcTailStrength.SetValue(value);});
    auto* arcHeadTailSlider = BSML::Lite::CreateSliderSetting(container, "Arc Head & Tails", increment, getBetterHapticsConfig().arcHeadTailStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().arcHeadTailStrength.SetValue(value);});
    auto* bombSlider = BSML::Lite::CreateSliderSetting(container, "Bombs", increment, getBetterHapticsConfig().bombStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().bombStrength.SetValue(value);});
    auto* badCutSlider = BSML::Lite::CreateSliderSetting(container, "Bad Cuts", increment, getBetterHapticsConfig().badCutStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().badCutStrength.SetValue(value);});
    auto* arcSlider = BSML::Lite::CreateSliderSetting(container, "Arcs", increment, getBetterHapticsConfig().arcStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().arcStrength.SetValue(value);});
    auto* wallSlider = BSML::Lite::CreateSliderSetting(container, "Walls", increment, getBetterHapticsConfig().wallStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().wallStrength.SetValue(value);});
    auto* saberClashSlider = BSML::Lite::CreateSliderSetting(container, "Saber Clash", increment, getBetterHapticsConfig().saberClashStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().saberClashStrength.SetValue(value);});

    BSML::Lite::AddHoverHint(uiSlider, "Default 1");
    BSML::Lite::AddHoverHint(noteSlider, "Default 1");
    BSML::Lite::AddHoverHint(chainHeadSlider, "Default 1");
    BSML::Lite::AddHoverHint(chainLinkSlider, "Default 1");
    BSML::Lite::AddHoverHint(arcHeadSlider, "Default 1");
    BSML::Lite::AddHoverHint(arcTailSlider, "Default 1");
    BSML::Lite::AddHoverHint(arcHeadTailSlider, "Default 1");
    BSML::Lite::AddHoverHint(bombSlider, "Default 1");
    BSML::Lite::AddHoverHint(badCutSlider, "Default 1");
    BSML::Lite::AddHoverHint(arcSlider, "Default 0.75");
    BSML::Lite::AddHoverHint(wallSlider, "Default 0.75");
    BSML::Lite::AddHoverHint(saberClashSlider, "Default 0.75");
}

// Called in the early stages of game loading
// (see https://github.com/sc2ad/scotland2?tab=readme-ov-file#installationusage)
// Often used to initialize and load configs, in addition to its contents here
extern "C" __attribute__((visibility("default")))
void setup(CModInfo* info) noexcept {
    *info = modInfo.to_c();

    // Paper::Logger::RegisterFileContextId(MOD_ID);

    logger.info("Completed setup!");
}

// Called later on in the game loading, after all mods have been opened
// Often used to install hooks and use the APIs of other mods or libraries
extern "C" __attribute__((visibility("default")))
void late_load() noexcept {
    i2c::functions::initialize();
    BSML::Init();

    getBetterHapticsConfig().Init(modInfo);

    BSML::Register::RegisterMainMenuViewControllerMethod(
        "BetterHaptics",
        "BetterHaptics",
        "",
        viewControllerDidActivate
    );

    INSTALL_HOOK(logger, OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset);
    INSTALL_HOOK(logger, VRInputModule_HandlePointerExitAndEnter);
    INSTALL_HOOK(logger, NoteCutHapticEffect_HitNote);
    INSTALL_HOOK(logger, SliderHapticFeedbackInteractionEffect_Vibrate);
    INSTALL_HOOK(logger, ObstacleSaberSparkleEffectManager_Update);
    INSTALL_HOOK(logger, SaberClashEffect_LateUpdate);

    logger.info("Completed load!");
}
