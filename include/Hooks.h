#pragma once
#include "HookBuilder.h"
#include "Manager.h"


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

        static inline bool doRotate = false;
        static inline bool doTranslate = false;
        static inline bool doTranslateZ = false;

        static inline bool InputEvent(RE::InputEvent* event) {

            if (auto button = event->AsButtonEvent()) {
                if (button->device == RE::INPUT_DEVICE::kMouse) {
                    if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kLeftButton) {
                        if (button->IsDown()) {
                            doTranslate = true;
                        } else if (button->IsUp()) {
                            doTranslate = false;
                        }
                        return true;
                    }
                    if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kRightButton) {
                        if (button->IsDown()) {
                            doRotate = true;
                        } else if (button->IsUp()) {
                            doRotate = false;
                        }
                        return true;
                    }
                    if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kMiddleButton) {
       
                        if (button->IsDown()) {
                            doTranslateZ = true;
                        } else if (button->IsUp()) {
                            doTranslateZ = false;
                        }
                        return true;
                    }
                    if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kWheelUp) {
                        Manager::GetSingleton()->IncrementDistance(3.f);
                        return true;
                    } else if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kWheelDown) {
                        Manager::GetSingleton()->IncrementDistance(-3.f);
                        return true;
                    }
                }
            }
            if (auto move = event->AsMouseMoveEvent()) {
                bool block = false;
                if (doRotate) {
                    Manager::GetSingleton()->RotateX(move->mouseInputX * 0.005);
                    Manager::GetSingleton()->RotateY(move->mouseInputY * 0.005);
                    block = true;
                }
                if (doTranslateZ) {
                    Manager::GetSingleton()->IncrementDistance(-move->mouseInputY * 0.05);
                    block = true;
                }
                if (doTranslate) {
                    Manager::GetSingleton()->TranslateX(move->mouseInputX * 0.05);
                    Manager::GetSingleton()->TranslateY(-move->mouseInputY * 0.05);
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
