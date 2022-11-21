#include "main.hpp"

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "UnityEngine/Vector2.hpp"
#include "GlobalNamespace/StandardLevelGameplayManager_InitData.hpp"
#include "GlobalNamespace/StandardLevelGameplayManager.hpp"
#include "GlobalNamespace/StandardLevelFailedController.hpp"
#include "System/Action.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/PauseAnimationController.hpp"
#include "GlobalNamespace/HMMainThreadDispatcher.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/Vector2.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"
#include "questui/shared/eventmacros.hpp"
using namespace GlobalNamespace;

#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
using namespace UnityEngine;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI::BeatSaberUI;

using namespace QuestUI;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace UnityEngine::Events;
using namespace HMUI;
using namespace TMPro;
using namespace GlobalNamespace;
static UnityEngine::UI::Button *button;
static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

bool shouldFail = false;
bool inGameplay;
StandardLevelGameplayManager * GameManager;
PauseController * pauser;
PauseMenuManager * pauserMENU;
// Loads the config from disk using our modInfo, then returns it for use
// other config tools such as config-utils don't use this config, so it can be removed if those are in use
Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

GameObject * PauseScreen;

void test(){
    shouldFail = true;
}

MAKE_HOOK_MATCH(AnUpdate, &HMMainThreadDispatcher::Update, void, HMMainThreadDispatcher* self) {
    AnUpdate(self);
    if(shouldFail){
        button->set_interactable(false);
        GameManager->levelFailedEvent->Invoke();
        shouldFail = false;
        //pauserMENU->continueButton->set_interactable(false);
        //pauserMENU->restartButton->set_interactable(false);
        //pauserMENU->backButton->set_interactable(false);
    }
    else{
    }
    
    //   if(button){
    //   if(button->IsHighlighted()){
    //    SetButtonText(button, "Are you sure?");
    //    }
    //   else{
    //    SetButtonText(button, "Fail");
    //  }
    //}
    
}



MAKE_HOOK_MATCH(PauseMenuHook, &PauseMenuManager::ShowMenu, void, PauseMenuManager *self)
{
    PauseMenuHook(self);
    PauseScreen = BeatSaberUI::CreateFloatingScreen(Vector2(0.0f, 0.0f), Vector3(-0.15f, 0.25f, 1.87f), Vector3(0.0f, 0.0f, 0.0f), 0.0f, true, false, 0);
    getLogger().info("created UI button");
    button = BeatSaberUI::CreateUIButton(PauseScreen->get_transform(), "Fail", "PlayButton", UnityEngine::Vector2(42.2f, 43.8f), UnityEngine::Vector2(11.3f, 11.3f), test);
    SetButtonTextSize(button, 4.0f);
    ToggleButtonWordWrapping(button, false);
    auto layoutElement = button->get_transform()->Find("Content")->GetComponent<UnityEngine::UI::LayoutElement*>();
    UnityEngine::Object::Destroy(layoutElement);
}

MAKE_HOOK_MATCH(PauseMenuContinue, &PauseMenuManager::ContinueButtonPressed, void, PauseMenuManager *self) {
    PauseMenuContinue(self);

    PauseScreen->SetActive(false);;
    
    }

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load();
    getLogger().info("Completed setup!");
}


MAKE_HOOK_MATCH(SceneChanged, &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged, void, UnityEngine::SceneManagement::Scene prevScene, UnityEngine::SceneManagement::Scene nextScene) {
    SceneChanged(prevScene, nextScene);
    // names = MainMenu, GameCore (QuestInit, EmptyTransition, HealthWarning, ShaderWarmup)
    if(nextScene && nextScene.get_name() == "MainMenu") {
        shouldFail = false;
        inGameplay = false;
    }

    if(nextScene && nextScene.get_name() == "GameCore") {
        inGameplay = true;

        pauser = UnityEngine::Resources::FindObjectsOfTypeAll<PauseController*>()[0];
        GameManager = UnityEngine::Resources::FindObjectsOfTypeAll<StandardLevelGameplayManager*>()[0];
    } else inGameplay = false;
}



MAKE_HOOK_MATCH(MainMenuUIHook, &GlobalNamespace::MainMenuViewController::DidActivate, void, GlobalNamespace::MainMenuViewController
*self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    // Run the original method before our code.
    // Note, you can run the original method after our code if you want to change arguments.
    MainMenuUIHook(self, firstActivation, addedToHierarchy, screenSystemEnabling); 
    
    shouldFail = false;

}


// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), PauseMenuHook);
    INSTALL_HOOK(getLogger(), MainMenuUIHook);
    INSTALL_HOOK(getLogger(), SceneChanged);
    INSTALL_HOOK(getLogger(), AnUpdate);
    INSTALL_HOOK(getLogger(), PauseMenuContinue);
    getLogger().info("Installed all hooks!");
}