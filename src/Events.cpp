#include "Events.h"
void Grab() {
    float bar = 0;
    using func_t = void(RE::PlayerCharacter* a1, float a2, void* a3, float* a4);
    REL::Relocation<func_t> func{RELOCATION_ID(40231, 41234)};
    return func(RE::PlayerCharacter::GetSingleton(), 0, nullptr, &bar);
}

RE::BSEventNotifyControl GrabEventHandler::ProcessEvent(
    const RE::TESGrabReleaseEvent* a_event, RE::BSTEventSource<RE::TESGrabReleaseEvent>* a_eventSource) {

    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    Manager::GetSingleton()->SetGrabbing(a_event->grabbed, a_event->ref);

    return RE::BSEventNotifyControl::kContinue;

}