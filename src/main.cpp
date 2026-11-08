#include "main.hpp"

#include "GlobalNamespace/NoteCutHapticEffect.hpp"
#include "GlobalNamespace/SliderHapticFeedbackInteractionEffect.hpp"
#include "GlobalNamespace/ObstacleSaberSparkleEffectManager.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SaberClashEffect.hpp"

#include "GlobalNamespace/OculusAdvancedHapticFeedbackPlayer.hpp"

#include "VRUIControls/VRInputModule.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

DECLARE_CONFIG(BetterHapticsConfig) {
    CONFIG_VALUE(enabled, bool, "enabled", true);
    CONFIG_VALUE(uiStrength, float, "uiStrength", 1);
    CONFIG_VALUE(noteStrength, float, "noteStrength", 1);
    CONFIG_VALUE(chainHeadStrength, float, "chainHeadStrength", 1);
    CONFIG_VALUE(chainLinkStrength, float, "chainLinkStrength", 1);
    CONFIG_VALUE(arcHeadTailStrength, float, "arcHeadTailStrength", 1);
    CONFIG_VALUE(bombStrength, float, "bombStrength", 1);
    CONFIG_VALUE(badCutStrength, float, "badCutStrength", 1);
    CONFIG_VALUE(arcStrength, float, "arcStrength", 0.75);
    CONFIG_VALUE(wallStrength, float, "wallStrength", 0.75);
    CONFIG_VALUE(saberClashStrength, float, "saberClashStrength", 0.75);
};

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    // File logging
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);

    PaperLogger.info("Completed setup!");
}

MAKE_HOOK_MATCH(
    OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset,
    &GlobalNamespace::OculusAdvancedHapticFeedbackPlayer::CanPlayHapticPreset, bool,
    GlobalNamespace::OculusAdvancedHapticFeedbackPlayer *self, Libraries::HM::HMLib::VR::HapticPresetSO *hapticPreset, UnityEngine::XR::XRNode node
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

    self->_rumblePreset->_strength = getBetterHapticsConfig().uiStrength.GetValue();

    VRInputModule_HandlePointerExitAndEnter(self, pointerEventData, newEnterTarget);
}

MAKE_HOOK_MATCH(
    NoteCutHapticEffect_HitNote,
    &GlobalNamespace::NoteCutHapticEffect::HitNote, void,
    GlobalNamespace::NoteCutHapticEffect* self, GlobalNamespace::SaberType saberType, GlobalNamespace::NoteCutHapticEffect::Type type
) {
    if (!getBetterHapticsConfig().enabled.GetValue())
        return NoteCutHapticEffect_HitNote(self, saberType, type);

    self->_normalPreset->_strength = getBetterHapticsConfig().noteStrength.GetValue();
    self->_chainHeadPreset->_strength = getBetterHapticsConfig().chainHeadStrength.GetValue();
    self->_chainLinkPreset->_strength = getBetterHapticsConfig().chainLinkStrength.GetValue();
    self->_arcHeadPreset->_strength = getBetterHapticsConfig().arcHeadTailStrength.GetValue();
    self->_arcTailPreset->_strength = getBetterHapticsConfig().arcHeadTailStrength.GetValue();
    self->_arcHeadAndTailPreset->_strength = getBetterHapticsConfig().arcHeadTailStrength.GetValue();
    self->_bombPreset->_strength = getBetterHapticsConfig().bombStrength.GetValue();
    self->_badCutPreset->_strength = getBetterHapticsConfig().badCutStrength.GetValue();

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

    const float increment = 0.05;
    auto* uiSlider = BSML::Lite::CreateSliderSetting(container, "UI", increment, getBetterHapticsConfig().uiStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().uiStrength.SetValue(value);});
    auto* noteSlider = BSML::Lite::CreateSliderSetting(container, "Notes", increment, getBetterHapticsConfig().noteStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().noteStrength.SetValue(value);});
    auto* chainHeadSlider = BSML::Lite::CreateSliderSetting(container, "Chain Heads", increment, getBetterHapticsConfig().chainHeadStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().chainHeadStrength.SetValue(value);});
    auto* chainLinkSlider = BSML::Lite::CreateSliderSetting(container, "Chain Links", increment, getBetterHapticsConfig().chainLinkStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().chainLinkStrength.SetValue(value);});
    auto* arcHeadTailSlider = BSML::Lite::CreateSliderSetting(container, "Arc Heads / Tails", increment, getBetterHapticsConfig().arcHeadTailStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().arcHeadTailStrength.SetValue(value);});
    auto* bombSlider = BSML::Lite::CreateSliderSetting(container, "Bombs", increment, getBetterHapticsConfig().bombStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().bombStrength.SetValue(value);});
    auto* badCutSlider = BSML::Lite::CreateSliderSetting(container, "Bad Cuts", increment, getBetterHapticsConfig().badCutStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().badCutStrength.SetValue(value);});
    auto* arcSlider = BSML::Lite::CreateSliderSetting(container, "Arcs", increment, getBetterHapticsConfig().arcStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().arcStrength.SetValue(value);});
    auto* wallSlider = BSML::Lite::CreateSliderSetting(container, "Walls", increment, getBetterHapticsConfig().wallStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().wallStrength.SetValue(value);});
    auto* saberClashSlider = BSML::Lite::CreateSliderSetting(container, "Saber Clash", increment, getBetterHapticsConfig().saberClashStrength.GetValue(), 0, 1, [](float value){getBetterHapticsConfig().saberClashStrength.SetValue(value);});

    BSML::Lite::AddHoverHint(uiSlider, "Default 1");
    BSML::Lite::AddHoverHint(noteSlider, "Default 1");
    BSML::Lite::AddHoverHint(chainHeadSlider, "Default 1");
    BSML::Lite::AddHoverHint(chainLinkSlider, "Default 1");
    BSML::Lite::AddHoverHint(arcHeadTailSlider, "Default 1");
    BSML::Lite::AddHoverHint(bombSlider, "Default 1");
    BSML::Lite::AddHoverHint(badCutSlider, "Default 1");
    BSML::Lite::AddHoverHint(arcSlider, "Default 0.75");
    BSML::Lite::AddHoverHint(wallSlider, "Default 0.75");
    BSML::Lite::AddHoverHint(saberClashSlider, "Default 0.75");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();

    getBetterHapticsConfig().Init(modInfo);
    BSML::Init();

    BSML::Register::RegisterMainMenuViewControllerMethod(
        "BetterHaptics",
        "BetterHaptics",
        "",
        viewControllerDidActivate
    );

    PaperLogger.info("Installing hooks...");

    INSTALL_HOOK(PaperLogger, OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset);
    INSTALL_HOOK(PaperLogger, VRInputModule_HandlePointerExitAndEnter);
    INSTALL_HOOK(PaperLogger, NoteCutHapticEffect_HitNote);
    INSTALL_HOOK(PaperLogger, SliderHapticFeedbackInteractionEffect_Vibrate);
    INSTALL_HOOK(PaperLogger, ObstacleSaberSparkleEffectManager_Update);
    INSTALL_HOOK(PaperLogger, SaberClashEffect_LateUpdate);

    PaperLogger.info("Installed all hooks!");
}
