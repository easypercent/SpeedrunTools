#pragma once

#include "../../plugin/PluginComponent.h"
#include "../../models/SaveState.h"

class SaveStateComponent : public PluginComponent
{
public:
    explicit SaveStateComponent(BakkesMod::Plugin::BakkesModPlugin *plugin);

    void onLoad() override;
    void onUnload() override;
    void render() override;

    void save();
    void load();
    bool isStateSaved() const;

private:
    SaveState saveState;
    bool isSaved;
};