#pragma once
#include "HookBuilder.h"
#include "Manager.h"
#include "InputManager.h"


namespace Hooks {
    inline bool CapturedSource = false;
    struct GrabHook {
        static inline bool thunk(RE::ObjectRefHandle* a1, RE::TESObjectREFRPtr* a2) { 
            auto obj = a1->get();
            CapturedSource = false;
            if (obj) {
                auto obj2 = obj.get();
                if (obj2) {
                    if (obj2->As<RE::Actor>()) {
                        return originalFunction(a1, a2);
                    } else {
                        auto manager = Manager::GetSingleton();
                        manager->UpdatePosition(obj2);
                        CapturedSource = true;
                    }
                }
            }
            return false;
        }
        static inline REL::Relocation<decltype(thunk)> originalFunction;
        inline static void Install(HookBuilder* builder) {
            //SE ID: 39479 SE Offset: 0x69
            //AE ID: 40556 AE Offset: 0x71 (Heuristic)
            builder->AddCall<GrabHook, 5, 14>(
                39479, 0x69, 
                40556, 0x71
            );

        }
    };
    struct GrabHook2 {
        static inline int64_t thunk(RE::PlayerCharacter* a1) {
            if (!CapturedSource) {
                return originalFunction(a1);
            }
            return false;
        }
        static inline REL::Relocation<decltype(thunk)> originalFunction;
        inline static void Install(HookBuilder* builder) {
            //SE ID: 39479 SE Offset: 0xb85 (Heuristic)
            //AE ID: 40556 AE Offset: 0xaf0
            builder->AddCall<GrabHook2, 5, 14>(39479, 0xb85, 40556, 0xaf0);
        }
    };

    struct ProcessInputQueueHook {



        static inline bool InputEvent(RE::InputEvent* event) {

            auto manager = Manager::GetSingleton();
            if (auto button = event->AsButtonEvent()) {
                if (InputManager::GetSingleton()->ProcessInput(button)) {
                    return true;
                }
            }
            if (auto move = event->AsMouseMoveEvent()) {
                bool block = false;
                if (manager->GetDoRotate()) {
                    Manager::GetSingleton()->RotateX(move->mouseInputX * 0.005);
                    Manager::GetSingleton()->RotateY(move->mouseInputY * 0.005);
                    block = true;
                }
                if (manager->GetTranslateZ()) {
                    Manager::GetSingleton()->IncrementDistance(-move->mouseInputY * 0.05);
                    block = true;
                }
                if (manager->GetDoTranslate()) {
                    Manager::GetSingleton()->TranslateX(move->mouseInputX * 0.05);
                    Manager::GetSingleton()->TranslateY(-move->mouseInputY * 0.05);
                    block = true;
                }
                return block;
            } 
            if (auto move = event->AsThumbstickEvent()) {
                bool block = false;
                if (manager->GetDoRotate()) {
                    Manager::GetSingleton()->RotateX(move->xValue * 0.1);
                    Manager::GetSingleton()->RotateY(move->yValue * 0.1);
                    block = true;
                }
                if (manager->GetTranslateZ()) {
                    Manager::GetSingleton()->IncrementDistance(move->yValue * 1.f);
                    block = true;
                }
                if (manager->GetDoTranslate()) {
                    Manager::GetSingleton()->TranslateX(move->xValue * 0.1);
                    Manager::GetSingleton()->TranslateY(move->yValue * 0.1);
                    block = true;
                }
                return block;
            }

            return false;
        }
        static inline void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent* const* a_event) {

            auto manager = Manager::GetSingleton();

            if (!manager->GetIsGrabbing()) {
                originalFunction(a_dispatcher, a_event);
                return;
            }

            auto first = *a_event;
            auto last = *a_event;
            size_t length = 0;

            for (auto current = *a_event; current; current = current->next) {

                bool suppress = InputEvent(current);

                if (suppress) {
                    if (current != last) {
                        last->next = current->next;
                    } else {
                        last = current->next;
                        first = current->next;
                    }
                } else {
                    last = current;
                    ++length;
                }
            }

            if (length == 0) {
                constexpr RE::InputEvent* const dummy[] = {nullptr};
                originalFunction(a_dispatcher, dummy);
            } else {
                RE::InputEvent* const e[] = {first};
                originalFunction(a_dispatcher, e);
            }


        }
        static inline REL::Relocation<decltype(thunk)> originalFunction;
        inline static void Install(HookBuilder* builder) {
            builder->AddCall<ProcessInputQueueHook, 5, 14>(
                67315, 0x7B, 
                68617, 0x7B//,
                //0xC519E0, 0x81
            );
        }
    };

    inline void Install() {
        auto builder = new HookBuilder();
        GrabHook::Install(builder);
        GrabHook2::Install(builder);
        ProcessInputQueueHook::Install(builder);

        // uint8_t data3[] = {0xeb, 0x23};  // allow disenchant anything // SkyrimSE.exe+9091ac //  0x90
        //REL::safe_write(REL::RelocationID(50440, 51344).address() + REL::Relocate(0x75, 0x7c), data3, sizeof(data3));

        //uint8_t data4[] = {0xeb};
        //REL::safe_write(REL::ID(40556).address() + 0x374, data4, sizeof(data4));

        builder->Install();
    }
}
