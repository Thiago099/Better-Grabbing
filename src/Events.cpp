#include "Events.h"

RE::BSEventNotifyControl GrabEventHandler::ProcessEvent(
    const RE::TESGrabReleaseEvent* a_event, RE::BSTEventSource<RE::TESGrabReleaseEvent>* a_eventSource) {

    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    Manager::GetSingleton()->SetGrabbing(a_event->grabbed, a_event->ref);

    return RE::BSEventNotifyControl::kContinue;

}