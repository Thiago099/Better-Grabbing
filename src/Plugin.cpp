#include "Plugin.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        auto script = RE::ScriptEventSourceHolder::GetSingleton();
        script->AddEventSink(new GrabEventHandler());
    }
    if (message->type == SKSE::MessagingInterface::kPostLoad || message->type == SKSE::MessagingInterface::kNewGame) {
        Manager::GetSingleton()->SetGrabbing(false, nullptr);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    SetupLog();
    logger::info("Plugin loaded");
    Hooks::Install();
    Persistence::LoadConfiguration();
    return true;
}
