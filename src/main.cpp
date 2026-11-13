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

    CONFIG_VALUE(uiDuration, float, "uiDuration", 0.01);
    CONFIG_VALUE(noteDuration, float, "noteDuration", 0.13);
    CONFIG_VALUE(chainHeadDuration, float, "chainHeadDuration", 0.13);
    CONFIG_VALUE(chainLinkDuration, float, "chainLinkDuration", 0.13);
    CONFIG_VALUE(arcHeadDuration, float, "arcHeadDuration", 0.13);
    CONFIG_VALUE(arcTailDuration, float, "arcTailDuration", 0.13);
    CONFIG_VALUE(arcHeadTailDuration, float, "arcHeadTailDuration", 0.13);
    CONFIG_VALUE(bombDuration, float, "bombDuration", 0.13);
    CONFIG_VALUE(badCutDuration, float, "badCutDuration", 0.13);
};

namespace UIHapticSliders {
    static UnityW<UnityEngine::GameObject> uiSlider = nullptr;
    static UnityW<UnityEngine::GameObject> noteSlider = nullptr;
    static UnityW<UnityEngine::GameObject> chainHeadSlider = nullptr;
    static UnityW<UnityEngine::GameObject> chainLinkSlider = nullptr;
    static UnityW<UnityEngine::GameObject> arcHeadSlider = nullptr;
    static UnityW<UnityEngine::GameObject> arcTailSlider = nullptr;
    static UnityW<UnityEngine::GameObject> arcHeadTailSlider = nullptr;
    static UnityW<UnityEngine::GameObject> bombSlider = nullptr;
    static UnityW<UnityEngine::GameObject> badCutSlider = nullptr;
    static UnityW<UnityEngine::GameObject> arcSlider = nullptr;
    static UnityW<UnityEngine::GameObject> wallSlider = nullptr;
    static UnityW<UnityEngine::GameObject> saberClashSlider = nullptr;
}

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

    if (newEnterTarget && newEnterTarget->name == "BSMLSlider") {
        #define DURATION(name) getBetterHapticsConfig().name##Duration.GetValue()
        #define PREVIEW_HAPTIC_PRESET(name) \
        if (newEnterTarget->transform->parent->parent->gameObject == UIHapticSliders::name##Slider) { \
            self->_rumblePreset->_strength = getBetterHapticsConfig().name##Strength.GetValue(); \
            self->_rumblePreset->_duration = DURATION(name); \
            return VRInputModule_HandlePointerExitAndEnter(self, pointerEventData, newEnterTarget); \
        }

        PREVIEW_HAPTIC_PRESET(note)
        PREVIEW_HAPTIC_PRESET(chainHead) else
        PREVIEW_HAPTIC_PRESET(chainLink) else
        PREVIEW_HAPTIC_PRESET(arcHead) else
        PREVIEW_HAPTIC_PRESET(arcTail) else
        PREVIEW_HAPTIC_PRESET(arcHeadTail) else
        PREVIEW_HAPTIC_PRESET(bomb) else
        PREVIEW_HAPTIC_PRESET(badCut) else
        #undef DURATION
        #define DURATION(name) 1
        PREVIEW_HAPTIC_PRESET(arc) else
        PREVIEW_HAPTIC_PRESET(wall) else
        PREVIEW_HAPTIC_PRESET(saberClash)
        #undef DURATION
        #undef PREVIEW_HAPTIC_PRESET
    }

    self->_rumblePreset->_strength = getBetterHapticsConfig().uiStrength.GetValue();
    self->_rumblePreset->_duration = getBetterHapticsConfig().uiDuration.GetValue();

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

    self->_normalPreset->_strength = getBetterHapticsConfig().noteStrength.GetValue();
    self->_chainHeadPreset->_strength = getBetterHapticsConfig().chainHeadStrength.GetValue();
    self->_chainLinkPreset->_strength = getBetterHapticsConfig().chainLinkStrength.GetValue();
    self->_arcHeadPreset->_strength = getBetterHapticsConfig().arcHeadStrength.GetValue();
    self->_arcTailPreset->_strength = getBetterHapticsConfig().arcTailStrength.GetValue();
    self->_arcHeadAndTailPreset->_strength = getBetterHapticsConfig().arcHeadTailStrength.GetValue();
    self->_bombPreset->_strength = getBetterHapticsConfig().bombStrength.GetValue();
    self->_badCutPreset->_strength = getBetterHapticsConfig().badCutStrength.GetValue();

    self->_normalPreset->_duration = getBetterHapticsConfig().noteDuration.GetValue();
    self->_chainHeadPreset->_duration = getBetterHapticsConfig().chainHeadDuration.GetValue();
    self->_chainLinkPreset->_duration = getBetterHapticsConfig().chainLinkDuration.GetValue();
    self->_arcHeadPreset->_duration = getBetterHapticsConfig().arcHeadDuration.GetValue();
    self->_arcTailPreset->_duration = getBetterHapticsConfig().arcTailDuration.GetValue();
    self->_arcHeadAndTailPreset->_duration = getBetterHapticsConfig().arcHeadTailDuration.GetValue();
    self->_bombPreset->_duration = getBetterHapticsConfig().bombDuration.GetValue();
    self->_badCutPreset->_duration = getBetterHapticsConfig().badCutDuration.GetValue();

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



void createSliderDefaultIndicator(BSML::SliderSetting* sliderSetting, float defaultValue) {
    auto slidingAreaTemplate = sliderSetting->transform->Find("BSMLSlider/SlidingArea")->gameObject;

    auto defaultIndicatorContainer = UnityEngine::GameObject::Instantiate(slidingAreaTemplate, sliderSetting->transform->Find("BSMLSlider"));
    defaultIndicatorContainer->name = "DefaultIndicator";
    defaultIndicatorContainer->transform->SetSiblingIndex(1);

    UnityEngine::GameObject::Destroy(defaultIndicatorContainer->transform->Find("Value")->gameObject);

    auto defaultIndicator = defaultIndicatorContainer->transform->Find("Handle");
    defaultIndicator->GetComponent<UnityEngine::RectTransform*>()->anchoredPosition = {
        33 * (defaultValue - sliderSetting->slider->minValue) / (sliderSetting->slider->maxValue - sliderSetting->slider->minValue), 0};
    defaultIndicator->GetComponent<HMUI::ImageView*>()->color = {1, 1, 1, 0.25};
}

UnityEngine::GameObject* createStrengthSlider(const BSML::Lite::TransformWrapper& parent, StringW label, float defaultStrength, float currentStrength, std::function<void(float)> onStrengthChange = nullptr) {
    auto* horizontal = BSML::Lite::CreateHorizontalLayoutGroup(parent);

    auto* strengthSlider = BSML::Lite::CreateSliderSetting(horizontal, label, 0.05, currentStrength, 0, 1, onStrengthChange);
    strengthSlider->GetComponent<UnityEngine::UI::LayoutElement*>()->preferredWidth = 65;
    createSliderDefaultIndicator(strengthSlider, defaultStrength);

    // strengthSlider really, really does not want to listen to preferredWidth or sizeDelta or anything
    auto* spacer = UnityEngine::GameObject::New_ctor("Spacer");
    auto* spacerTransform = spacer->AddComponent<UnityEngine::RectTransform*>();
    spacerTransform->SetParent(horizontal->transform, false);
    auto* spacerLayout = spacer->AddComponent<UnityEngine::UI::LayoutElement*>();
    spacerLayout->preferredWidth = 40;

    return horizontal->gameObject;
}

UnityEngine::GameObject* createStrengthDurationSlider(const BSML::Lite::TransformWrapper& parent, StringW label, float defaultStrength, float defaultDuration, float currentStrength, float currentDuration, std::function<void(float)> onStrengthChange = nullptr, std::function<void(float)> onDurationChange = nullptr) {
    auto* horizontal = BSML::Lite::CreateHorizontalLayoutGroup(parent);

    auto* strengthSlider = BSML::Lite::CreateSliderSetting(horizontal, label, 0.05, currentStrength, 0, 1, onStrengthChange);
    strengthSlider->GetComponent<UnityEngine::UI::LayoutElement*>()->preferredWidth = 65;
    createSliderDefaultIndicator(strengthSlider, defaultStrength);

    auto* durationSlider = BSML::Lite::CreateSliderSetting(horizontal, label, 0.01, currentDuration, 0.01, 1, onDurationChange);
    UnityEngine::GameObject::Destroy(durationSlider->transform->Find("Title")->gameObject);
    durationSlider->GetComponent<UnityEngine::UI::LayoutElement*>()->preferredWidth = 40;
    createSliderDefaultIndicator(durationSlider, defaultDuration);

    return horizontal->gameObject;
}

void viewControllerDidActivate(HMUI::ViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    auto* container = BSML::Lite::CreateScrollableSettingsContainer(self->transform);

    BSML::Lite::CreateToggle(container, "Enable", getBetterHapticsConfig().enabled.GetValue(), [](bool value){getBetterHapticsConfig().enabled.SetValue(value);});

    auto* tableLabels = UnityEngine::GameObject::New_ctor("TableLabels");
    auto* tableLabelsTransform = tableLabels->AddComponent<UnityEngine::RectTransform*>();
    tableLabelsTransform->SetParent(container->transform, false);
    tableLabelsTransform->anchorMin = {0, 1};
    tableLabelsTransform->anchorMax = {0, 1};
    tableLabelsTransform->sizeDelta = {0, 7};
    auto* tableStrengthLabel = BSML::Lite::CreateText(tableLabels, "Strength", {-1, 0}, {20, 0});
    tableStrengthLabel->verticalAlignment = TMPro::VerticalAlignmentOptions::Capline;
    auto* tableDurationLabel = BSML::Lite::CreateText(tableLabels, "Duration", {39, 0}, {20, 0});
    tableDurationLabel->verticalAlignment = TMPro::VerticalAlignmentOptions::Capline;

    UIHapticSliders::uiSlider = createStrengthDurationSlider(container, "UI", getBetterHapticsConfig().uiStrength.GetDefaultValue(), getBetterHapticsConfig().uiDuration.GetDefaultValue(), getBetterHapticsConfig().uiStrength.GetValue(), getBetterHapticsConfig().uiDuration.GetValue(), [](float value){getBetterHapticsConfig().uiStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().uiDuration.SetValue(value);});
    UIHapticSliders::noteSlider = createStrengthDurationSlider(container, "Notes", getBetterHapticsConfig().noteStrength.GetDefaultValue(), getBetterHapticsConfig().noteDuration.GetDefaultValue(), getBetterHapticsConfig().noteStrength.GetValue(), getBetterHapticsConfig().noteDuration.GetValue(), [](float value){getBetterHapticsConfig().noteStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().noteDuration.SetValue(value);});
    UIHapticSliders::chainHeadSlider = createStrengthDurationSlider(container, "Chain Heads", getBetterHapticsConfig().chainHeadStrength.GetDefaultValue(), getBetterHapticsConfig().chainHeadDuration.GetDefaultValue(), getBetterHapticsConfig().chainHeadStrength.GetValue(), getBetterHapticsConfig().chainHeadDuration.GetValue(), [](float value){getBetterHapticsConfig().chainHeadStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().chainHeadDuration.SetValue(value);});
    UIHapticSliders::chainLinkSlider = createStrengthDurationSlider(container, "Chain Links", getBetterHapticsConfig().chainLinkStrength.GetDefaultValue(), getBetterHapticsConfig().chainLinkDuration.GetDefaultValue(), getBetterHapticsConfig().chainLinkStrength.GetValue(), getBetterHapticsConfig().chainLinkDuration.GetValue(), [](float value){getBetterHapticsConfig().chainLinkStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().chainLinkDuration.SetValue(value);});
    UIHapticSliders::arcHeadSlider = createStrengthDurationSlider(container, "Arc Heads", getBetterHapticsConfig().arcHeadStrength.GetDefaultValue(), getBetterHapticsConfig().arcHeadDuration.GetDefaultValue(), getBetterHapticsConfig().arcHeadStrength.GetValue(), getBetterHapticsConfig().arcHeadDuration.GetValue(), [](float value){getBetterHapticsConfig().arcHeadStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().arcHeadDuration.SetValue(value);});
    UIHapticSliders::arcTailSlider = createStrengthDurationSlider(container, "Arc Tails", getBetterHapticsConfig().arcTailStrength.GetDefaultValue(), getBetterHapticsConfig().arcTailDuration.GetDefaultValue(), getBetterHapticsConfig().arcTailStrength.GetValue(), getBetterHapticsConfig().arcTailDuration.GetValue(), [](float value){getBetterHapticsConfig().arcTailStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().arcTailDuration.SetValue(value);});
    UIHapticSliders::arcHeadTailSlider = createStrengthDurationSlider(container, "Arc Head & Tails", getBetterHapticsConfig().arcHeadTailStrength.GetDefaultValue(), getBetterHapticsConfig().arcHeadTailDuration.GetDefaultValue(), getBetterHapticsConfig().arcHeadTailStrength.GetValue(), getBetterHapticsConfig().arcHeadTailDuration.GetValue(), [](float value){getBetterHapticsConfig().arcHeadTailStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().arcHeadTailDuration.SetValue(value);});
    UIHapticSliders::bombSlider = createStrengthDurationSlider(container, "Bombs", getBetterHapticsConfig().bombStrength.GetDefaultValue(), getBetterHapticsConfig().bombDuration.GetDefaultValue(), getBetterHapticsConfig().bombStrength.GetValue(), getBetterHapticsConfig().bombDuration.GetValue(), [](float value){getBetterHapticsConfig().bombStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().bombDuration.SetValue(value);});
    UIHapticSliders::badCutSlider = createStrengthDurationSlider(container, "Bad Cuts", getBetterHapticsConfig().badCutStrength.GetDefaultValue(), getBetterHapticsConfig().badCutDuration.GetDefaultValue(), getBetterHapticsConfig().badCutStrength.GetValue(), getBetterHapticsConfig().badCutDuration.GetValue(), [](float value){getBetterHapticsConfig().badCutStrength.SetValue(value);}, [](float value){getBetterHapticsConfig().badCutDuration.SetValue(value);});

    UIHapticSliders::arcSlider = createStrengthSlider(container, "Arcs", getBetterHapticsConfig().arcStrength.GetDefaultValue(), getBetterHapticsConfig().arcStrength.GetValue(), [](float value){getBetterHapticsConfig().arcStrength.SetValue(value);});
    UIHapticSliders::wallSlider = createStrengthSlider(container, "Walls", getBetterHapticsConfig().wallStrength.GetDefaultValue(), getBetterHapticsConfig().wallStrength.GetValue(), [](float value){getBetterHapticsConfig().wallStrength.SetValue(value);});
    UIHapticSliders::saberClashSlider = createStrengthSlider(container, "Saber Clash", getBetterHapticsConfig().saberClashStrength.GetDefaultValue(), getBetterHapticsConfig().saberClashStrength.GetValue(), [](float value){getBetterHapticsConfig().saberClashStrength.SetValue(value);});
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
