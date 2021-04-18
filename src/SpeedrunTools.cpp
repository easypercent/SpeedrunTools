#include "SpeedrunTools.h"
#include "components/generaltools/GeneralToolsComponent.h"
#include "components/livesplit/LiveSplitComponent.h"
#include "components/kismet/KismetEditorComponent.h"
#include "components/savestates/SaveStateComponent.h"
#include "components/maptools/MapToolsComponent.h"

BAKKESMOD_PLUGIN(SpeedrunTools, SpeedrunTools::PLUGIN_TITLE, SpeedrunTools::PLUGIN_VERSION, PLUGINTYPE_CUSTOM_TRAINING)

const char *SpeedrunTools::PLUGIN_VERSION = "2.1";
const char *SpeedrunTools::PLUGIN_TITLE = "Speedrun Tools";
const char *SpeedrunTools::PLUGIN_MENU_NAME = "speedruntools";

SpeedrunTools::SpeedrunTools()
        : BaseBakkesModPlugin(SpeedrunTools::PLUGIN_TITLE, SpeedrunTools::PLUGIN_MENU_NAME),
          tabs()
{
    this->tabs.emplace_back("Map Tools", std::make_unique<MapToolsComponent>(this));
    this->tabs.emplace_back("General Tools", std::make_unique<GeneralToolsComponent>(this));
    this->tabs.emplace_back("Save State", std::make_unique<SaveStateComponent>(this));
    this->tabs.emplace_back("LiveSplit", std::make_unique<LiveSplitComponent>(this));
    this->tabs.emplace_back("Kismet", std::make_unique<KismetEditorComponent>(this));
}

void SpeedrunTools::onLoad()
{
    this->setupEvents();
    for (auto &tab : this->tabs)
    {
        tab.second->onLoad();
    }
}

void SpeedrunTools::onUnload()
{
    for (auto &tab : this->tabs)
    {
        tab.second->onUnload();
    }
}

void SpeedrunTools::renderBody()
{
    static bool showDemoWindow = false;
    ImGui::Checkbox("Show Demo Window", &showDemoWindow);
    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow();
    }

    ImGui::Text("%s (version %s)", PLUGIN_TITLE, PLUGIN_VERSION);
    ImGui::Spacing();

    if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_None))
    {
        for (auto &tab : this->tabs)
        {
            if (ImGui::BeginTabItem(tab.first.c_str()))
            {
                tab.second->render();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}
void SpeedrunTools::setupEvents()
{
    static bool lock = false;
    static bool jump = false;
    static float jumpDelay = 0.03f; // in seconds

    // tick (params for controller input override)
    this->gameWrapper->HookEventWithCaller<CarWrapper>(
            "Function TAGame.Car_TA.SetVehicleInput",
            [this](CarWrapper cw, void *params, const std::string &eventName) {
                for (auto &tab : this->tabs) tab.second->onEvent(eventName, false, params);

                if (cw.IsOnGround())
                {
                    if (!lock && !jump)
                    {
                        lock = true;
                        this->gameWrapper->SetTimeout([this](GameWrapper *gw) {
                            jump = true;
                        }, jumpDelay);
                    }
                    if (jump)
                    {
                        auto *controllerInput = (ControllerInput *) params;
                        controllerInput->Jump = 1;
                        this->cvarManager->log("jump");
                        jump = false;

                        this->gameWrapper->SetTimeout([this](GameWrapper *gw) {
                            lock = false;
                        }, 0.1);
                    }
                }
            }
    );
    this->setupEventPost("Function TAGame.Car_TA.SetVehicleInput");

    // map start end
    this->setupEventPost("Function TAGame.GameEvent_Soccar_TA.InitGame");
    this->setupEventPost("Function TAGame.GameEvent_Soccar_TA.Destroyed");

    // player resets
    this->setupEventPost("Function TAGame.GameEvent_TA.PlayerResetTraining");

    // tutorial only
    this->setupEventPost("Function TAGame.GameInfo_Tutorial_TA.PostBeginPlay");
    this->setupEventPost("Function GameEvent_Tutorial_Basic_TA.Active.HandleHitGoal");
    this->setupEventPost("Function TAGame.GameEvent_Tutorial_TA.Destroyed");

    // modal popup
    this->setupEventPost("Function TAGame.GFxShell_TA.ShowModalObject");
}

void SpeedrunTools::setupEvent(const std::string &eventName)
{
    this->gameWrapper->HookEvent(
            eventName,
            [this](const std::string &eventName) {
                for (auto &tab : this->tabs) tab.second->onEvent(eventName, false, nullptr);
            }
    );
}

void SpeedrunTools::setupEventPost(const std::string &eventName)
{
    this->gameWrapper->HookEventPost(
            eventName,
            [this](const std::string &eventName) {
                for (auto &tab : this->tabs) tab.second->onEvent(eventName, true, nullptr);
            }
    );
}
