#include "Plugin.h"
#include "DrawDebug.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        auto script = RE::ScriptEventSourceHolder::GetSingleton();
        script->AddEventSink(new GrabEventHandler());
    }
    if (message->type == SKSE::MessagingInterface::kPostLoadGame || message->type == SKSE::MessagingInterface::kNewGame) {
        Manager::GetSingleton()->SetGrabbing(false, nullptr);
    }
    DrawDebug::OnMessage(message);
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
    SetupLog();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    logger::info("Plugin loaded");
    Hooks::Install();
    Persistence::LoadConfiguration();
	logger::info("Grab and Throw is {}", is_grab_n_throw_installed ? "installed" : "not installed" );
    return true;
}
