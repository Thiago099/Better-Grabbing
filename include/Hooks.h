#pragma once
#include "HookBuilder.h"
#include "Manager.h"
#include "InputManager.h"
#include "Config.h"

namespace Hooks {
    inline bool OriginalDraggingBehavior = false;
    struct GrabHook {
        static inline bool thunk(RE::ObjectRefHandle* a1, RE::TESObjectREFRPtr* a2) { 
            auto obj = a1->get();
            OriginalDraggingBehavior = false;
            if (obj) {
                auto obj2 = obj.get();
                if (obj2) {
                    if (obj2->As<RE::Actor>()) {
                        OriginalDraggingBehavior = true;
                        return originalFunction(a1, a2);
                    } else {
                        auto manager = Manager::GetSingleton();
                        manager->UpdatePosition(obj2);
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
            if (OriginalDraggingBehavior) {
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
            auto config = Config::GetSingleton();
            if (auto button = event->AsButtonEvent()) {
                if (InputManager::GetSingleton()->ProcessInput(button)) {
                    return true;
                }
            }
            if (event->device == RE::INPUT_DEVICE::kMouse){
                if (auto move = event->AsMouseMoveEvent()) {
                    bool block = false;
                    if (manager->GetDoRotate()) {
                        Manager::GetSingleton()->RotateX(move->mouseInputX * config->MouseRotateXSensitivity);
                        Manager::GetSingleton()->RotateY(move->mouseInputY * config->MouseRotateYSensitivity);
                        block = true;
                    }
                    if (manager->GetTranslateZ()) {
                        Manager::GetSingleton()->TranslateZ(-move->mouseInputY * config->MouseTranslateZSensitivity);
                        block = true;
                    }
                    if (manager->GetDoTranslate()) {
                        Manager::GetSingleton()->TranslateX(move->mouseInputX * config->MouseTranslateXSensitivity);
                        Manager::GetSingleton()->TranslateY(-move->mouseInputY * config->MouseTranslateYSensitivity);
                        block = true;
                    }
                    return block;
                } 
            }
            if (event->device == RE::INPUT_DEVICE::kGamepad) {
                if (auto move = event->AsThumbstickEvent()) {
                    bool block = false;
                    if (manager->GetDoRotate()) {
                        Manager::GetSingleton()->RotateX(move->xValue * config->GamepadRotateXSensitivity);
                        Manager::GetSingleton()->RotateY(-move->yValue * config->GamepadRotateYSensitivity);
                        block = true;
                    }
                    if (manager->GetTranslateZ()) {
                        Manager::GetSingleton()->TranslateZ(move->yValue * config->GamepadTranslateZSensitivity);
                        block = true;
                    }
                    if (manager->GetDoTranslate()) {
                        Manager::GetSingleton()->TranslateX(move->xValue * config->GamepadTranslateXSensitivity);
                        Manager::GetSingleton()->TranslateY(move->yValue * config->GamepadTranslateYSensitivity);
                        block = true;
                    }
                    return block;
                }
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
