#include "main.hpp"

#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/NoteCutHapticEffect.hpp"

#include "GlobalNamespace/OculusAdvancedHapticFeedbackPlayer.hpp"

#include "GlobalNamespace/SaberType.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

DECLARE_CONFIG(BetterHapticsConfig) {
    CONFIG_VALUE(globalBypass, bool, "globalBypass", false);
    CONFIG_VALUE(separateArcs, bool, "separateArcs", true);
    CONFIG_VALUE(noteStrength, float, "noteStrength", 1);
    CONFIG_VALUE(chainHeadStrength, float, "chainHeadStrength", 1);
    CONFIG_VALUE(chainLinkStrength, float, "chainLinkStrength", 1);
    CONFIG_VALUE(bombStrength, float, "bombStrength", 1);
    CONFIG_VALUE(badCutStrength, float, "badCutStrength", 1);
    CONFIG_VALUE(arcHeadTailStrength, float, "arcHeadTailStrength", 1);
};

bool get_global_bypass() {
    return getBetterHapticsConfig().globalBypass.GetValue();
}

void set_global_bypass(bool globalBypass) {
    getBetterHapticsConfig().globalBypass.SetValue(globalBypass);
}

bool get_separate_arcs() {
    return getBetterHapticsConfig().separateArcs.GetValue();
}

void set_separate_arcs(bool separateArcs) {
    getBetterHapticsConfig().separateArcs.SetValue(separateArcs);
}

// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    // File logging
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);

    PaperLogger.info("Completed setup!");
}

MAKE_HOOK_MATCH(
    NoteCutCoreEffectsSpawner_PlayHitNoteHapticEffect,
    &GlobalNamespace::NoteCutCoreEffectsSpawner::PlayHitNoteHapticEffect, void,
    GlobalNamespace::NoteCutCoreEffectsSpawner *self, GlobalNamespace::NoteCutInfo noteCutInfo, GlobalNamespace::NoteData *noteData
) {
    if (get_global_bypass() || get_separate_arcs()) {
        NoteCutCoreEffectsSpawner_PlayHitNoteHapticEffect(self, noteCutInfo, noteData);
        return;
    }

    if (!noteCutInfo.allIsOK) {
        self->_noteCutHapticEffect->HitNote(noteCutInfo.saberType, GlobalNamespace::NoteCutHapticEffect::Type::BadCut);
    } else {
        self->_noteCutHapticEffect->HitNote(noteCutInfo.saberType, GlobalNamespace::NoteCutHapticEffect::Type::Normal);
    }
}

MAKE_HOOK_MATCH(
    NoteCutCoreEffectsSpawner_PlayHitChainNoteHapticEffect,
    &GlobalNamespace::NoteCutCoreEffectsSpawner::PlayHitChainNoteHapticEffect, void,
    GlobalNamespace::NoteCutCoreEffectsSpawner *self, GlobalNamespace::NoteCutInfo noteCutInfo, bool isChainHead
) {
    if (get_global_bypass()) {
        NoteCutCoreEffectsSpawner_PlayHitChainNoteHapticEffect(self, noteCutInfo, isChainHead);
        return;
    }

    if (!noteCutInfo.allIsOK) {
        self->_noteCutHapticEffect->HitNote(noteCutInfo.saberType, GlobalNamespace::NoteCutHapticEffect::Type::BadCut);
    } else if (isChainHead) {
        self->_noteCutHapticEffect->HitNote(noteCutInfo.saberType, GlobalNamespace::NoteCutHapticEffect::Type::ChainHead);
    } else {
        self->_noteCutHapticEffect->HitNote(noteCutInfo.saberType, GlobalNamespace::NoteCutHapticEffect::Type::ChainLink);
    }
}

MAKE_HOOK_MATCH(
    OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset,
    &GlobalNamespace::OculusAdvancedHapticFeedbackPlayer::CanPlayHapticPreset, bool,
    GlobalNamespace::OculusAdvancedHapticFeedbackPlayer *self, Libraries::HM::HMLib::VR::HapticPresetSO *hapticPreset, UnityEngine::XR::XRNode node
) {
    if (get_global_bypass()) {
        return OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset(self, hapticPreset, node);
    }

    return false;
}

const float DEFAULT_HAPTIC_DURATION = 0.13f;
MAKE_HOOK_MATCH(
    NoteCutHapticEffect_HitNote,
    &GlobalNamespace::NoteCutHapticEffect::HitNote, void,
    GlobalNamespace::NoteCutHapticEffect* self, GlobalNamespace::SaberType saberType, GlobalNamespace::NoteCutHapticEffect::Type type
) {
    self->_normalPreset->_duration = getBetterHapticsConfig().noteStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_chainHeadPreset->_duration = getBetterHapticsConfig().chainHeadStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_chainLinkPreset->_duration = getBetterHapticsConfig().chainLinkStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_bombPreset->_duration = getBetterHapticsConfig().bombStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_badCutPreset->_duration = getBetterHapticsConfig().badCutStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_arcHeadPreset->_duration = getBetterHapticsConfig().arcHeadTailStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_arcTailPreset->_duration = getBetterHapticsConfig().arcHeadTailStrength.GetValue() * DEFAULT_HAPTIC_DURATION;
    self->_arcHeadAndTailPreset->_duration = getBetterHapticsConfig().arcHeadTailStrength.GetValue() * DEFAULT_HAPTIC_DURATION;

    NoteCutHapticEffect_HitNote(self, saberType, type);
}

void viewControllerDidActivate(HMUI::ViewController *self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    auto* container = BSML::Lite::CreateScrollableSettingsContainer(self->transform);

    BSML::Lite::CreateToggle(container, "Global Bypass", get_global_bypass(), set_global_bypass);
    BSML::Lite::CreateToggle(container, "Separate Arcs", get_separate_arcs(), set_separate_arcs);

    BSML::Lite::CreateSliderSetting(container, "Note", 0.1, getBetterHapticsConfig().noteStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().noteStrength.SetValue(value);});
    BSML::Lite::CreateSliderSetting(container, "Chain Head", 0.1, getBetterHapticsConfig().chainHeadStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().chainHeadStrength.SetValue(value);});
    BSML::Lite::CreateSliderSetting(container, "Chain Link", 0.1, getBetterHapticsConfig().chainLinkStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().chainLinkStrength.SetValue(value);});
    BSML::Lite::CreateSliderSetting(container, "Bomb", 0.1, getBetterHapticsConfig().bombStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().bombStrength.SetValue(value);});
    BSML::Lite::CreateSliderSetting(container, "Bad Cut", 0.1, getBetterHapticsConfig().badCutStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().badCutStrength.SetValue(value);});
    BSML::Lite::CreateSliderSetting(container, "Arc Head / Tail", 0.1, getBetterHapticsConfig().arcHeadTailStrength.GetValue(), 0, 2, [](float value){getBetterHapticsConfig().arcHeadTailStrength.SetValue(value);});
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();

    getBetterHapticsConfig().Init(modInfo);
    BSML::Init();

    BSML::Register::RegisterMainMenuViewControllerMethod(
        "BetterHaptics by Crunkle",
        "BetterHaptics",
        "",
        viewControllerDidActivate
    );

    PaperLogger.info("Installing hooks...");

    INSTALL_HOOK(PaperLogger, NoteCutCoreEffectsSpawner_PlayHitNoteHapticEffect);
    INSTALL_HOOK(PaperLogger, NoteCutCoreEffectsSpawner_PlayHitChainNoteHapticEffect);
    INSTALL_HOOK(PaperLogger, OculusAdvancedHapticFeedbackPlayer_CanPlayHapticPreset);
    INSTALL_HOOK(PaperLogger, NoteCutHapticEffect_HitNote);

    PaperLogger.info("Installed all hooks!");
}
