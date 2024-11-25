#pragma once
#include "Manager.h"

class GrabEventHandler final : public RE::BSTEventSink<RE::TESGrabReleaseEvent> {
public:
    // This method will be called when the event is triggered
    RE::BSEventNotifyControl ProcessEvent(const RE::TESGrabReleaseEvent* a_event,
                                                  RE::BSTEventSource<RE::TESGrabReleaseEvent>* a_eventSource) override;
};

