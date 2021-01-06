#include "AutoSplitterComponent.h"
#include "autosplitters/AutoSplitterSupportedRun.h"

AutoSplitterComponent::AutoSplitterComponent(BakkesMod::Plugin::BakkesModPlugin *plugin)
        : PluginComponentBase(plugin),
          liveSplitClient(LiveSplitClient::getInstance()),
          autoSplitterFactory(AutoSplitterFactory::getInstance(plugin)),
          autoSplitter(nullptr),
          isAutoStartEnabled(true),
          isAutoSplitEnabled(true),
          isAutoResetEnabled(true)
{

}

void AutoSplitterComponent::onLoad()
{

}

void AutoSplitterComponent::render()
{
    ImGui::Text("Auto Splitter:");
    ImGui::Spacing();

    if (!this->liveSplitClient.isConnected())
    {
        ImGui::Text("Waiting for a connection to the LiveSplit Server...");
        return;
    }

    const char *runNames[] = {
            "None",
            AutoSplitterSupportedRun::TUTORIAL_BASIC.runName.c_str(),
            AutoSplitterSupportedRun::TUTORIAL_ADVANCED.runName.c_str()
    };
    static int comboIndex = 0;
    if (ImGui::Combo("Run/Category", &comboIndex, runNames, IM_ARRAYSIZE(runNames)))
    {
        this->autoSplitter = this->autoSplitterFactory.getAutoSplitterForRun(runNames[comboIndex]);
        this->isAutoStartEnabled = true;
        this->isAutoSplitEnabled = true;
        this->isAutoResetEnabled = true;
    }
    if (this->autoSplitter == nullptr) return;

    ImGui::Spacing();

    if (!this->autoSplitter->startDescription().empty())
    {
        ImGui::Spacing();
        ImGui::Text("Auto Start Description: %s", this->autoSplitter->startDescription().c_str());
    }
    if (!this->autoSplitter->splitDescription().empty())
    {
        ImGui::Spacing();
        ImGui::Text("Auto Split Description: %s", this->autoSplitter->splitDescription().c_str());
    }
    if (!this->autoSplitter->resetDescription().empty())
    {
        ImGui::Spacing();
        ImGui::Text("Auto Reset Description: %s", this->autoSplitter->resetDescription().c_str());
    }

    ImGui::Spacing();

    if (this->autoSplitter->supportsStart()) ImGui::Checkbox("Auto Start Enabled", &this->isAutoStartEnabled);
    if (this->autoSplitter->supportsSplit()) ImGui::Checkbox("Auto Split Enabled", &this->isAutoSplitEnabled);
    if (this->autoSplitter->supportsReset()) ImGui::Checkbox("Auto Reset Enabled", &this->isAutoResetEnabled);

    ImGui::Spacing();

    std::string debug = this->autoSplitter->getDebug();
    ImGui::InputTextMultiline("Debug Output", (char *) debug.c_str(), debug.capacity() + 1, ImVec2(0, ImGui::GetTextLineHeight() * 8),
                              ImGuiInputTextFlags_ReadOnly);
}

void AutoSplitterComponent::onEvent(const std::string &eventName, bool post, void *params)
{
    if (!this->liveSplitClient.isConnected()) return;
    if (this->autoSplitter == nullptr) return;

    this->autoSplitter->onEvent(eventName, post, params);

    if (this->isAutoStartEnabled && this->autoSplitter->supportsStart() && this->autoSplitter->start()) this->start();
    if (this->isAutoSplitEnabled && this->autoSplitter->supportsSplit() && this->autoSplitter->split()) this->split();
    if (this->isAutoResetEnabled && this->autoSplitter->supportsReset() && this->autoSplitter->reset()) this->reset();
}

void AutoSplitterComponent::start()
{
    this->liveSplitClient.start([this](const int &errorCode, const std::string &errorMessage) {
        std::string log = (errorCode == 0) ? "'start' message was successfully sent." :
                          "Error while sending message 'start' (" + std::to_string(errorCode) + ") \"" + errorMessage + "\".";
        this->log(log);
    });
}

void AutoSplitterComponent::split()
{
    this->liveSplitClient.split([this](const int &errorCode, const std::string &errorMessage) {
        std::string log = (errorCode == 0) ? "'split' message was successfully sent." :
                          "Error while sending message 'split' (" + std::to_string(errorCode) + ") \"" + errorMessage + "\".";
        this->log(log);
    });
}

void AutoSplitterComponent::reset()
{
    this->liveSplitClient.reset([this](const int &errorCode, const std::string &errorMessage) {
        std::string log = (errorCode == 0) ? "'reset' message was successfully sent." :
                          "Error while sending message 'reset' (" + std::to_string(errorCode) + ") \"" + errorMessage + "\".";
        this->log(log);
    });
}

void AutoSplitterComponent::log(const std::string &message)
{
    this->plugin->cvarManager->log("Auto Splitter: " + message);
    this->plugin->gameWrapper->Execute([this, message](GameWrapper *gw) {
        this->plugin->gameWrapper->LogToChatbox("Auto Splitter: " + message, "SPEEDRUNTOOLS");
    });
}
