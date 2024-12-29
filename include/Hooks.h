#pragma once
#include "HookBuilder.h"
#include "Manager.h"
#include "InputManager.h"
#include "Config.h"

namespace Hooks {
    inline bool OriginalDraggingBehavior = false;
    struct GrabHook {
        static bool thunk(RE::ObjectRefHandle* a1, RE::TESObjectREFRPtr* a2) {
            const auto obj = a1->get();
            OriginalDraggingBehavior = false;
            if (obj) {
                if (const auto obj2 = obj.get()) {
                    if (UseDefaultGrabbingBehavior(obj2)) {
                        OriginalDraggingBehavior = true;
                        return originalFunction(a1, a2);
                    }
                    const auto manager = Manager::GetSingleton();
                    if (!manager->GetIsGrabbing()) {
                        manager->SetGrabbing(true, obj);
                    }
                    manager->UpdatePosition(obj2);

                }
            }
            return false;
        }

        static bool UseDefaultGrabbingBehavior(RE::TESObjectREFR* obj2) {

            if (auto base = obj2->GetBaseObject()) {
                if (base->As<RE::TESFlora>()) {
                    return true;
                }
            }

            return 
                obj2->As<RE::Actor>() ||
                Manager::IsTelekinesisObject(obj2);
        }

        static inline REL::Relocation<decltype(thunk)> originalFunction;
        static void Install(HookBuilder* builder) {
            //SE ID: 39479 SE Offset: 0x69
            //AE ID: 40556 AE Offset: 0x71 (Heuristic)
            builder->AddCall<GrabHook, 5, 14>(
                39479, 0x69, 
                40556, 0x71
            );

        }
    };
    struct GrabHook2 {
        static int64_t thunk(RE::PlayerCharacter* a1) {
            const auto manager = Manager::GetSingleton();
            if (OriginalDraggingBehavior || !manager->GetIsGrabbing()) {
                manager->SetGrabbing(false, nullptr);
                return originalFunction(a1);
            }
            return false;
        }
        static inline REL::Relocation<decltype(thunk)> originalFunction;
        static void Install(HookBuilder* builder) {
            //SE ID: 39479 SE Offset: 0xb85 (Heuristic)
            //AE ID: 40556 AE Offset: 0xaf0
            builder->AddCall<GrabHook2, 5, 14>(39479, 0xb85, 40556, 0xaf0);
        }
    };

    struct ProcessInputQueueHook {



        static bool InputEvent(RE::InputEvent* event) {

            const auto config = Config::GetSingleton();
            if (const auto button = event->AsButtonEvent()) {
                if (InputManager::GetSingleton()->ProcessInput(button)) {
                    return true;
                }
            }
            if (event->device == RE::INPUT_DEVICE::kMouse){
                if (const auto move = event->AsMouseMoveEvent()) {
                    bool block = false;
                    if (Manager::GetDoRotate()) {
                        Manager::GetSingleton()->RotateX(move->mouseInputX * config->MouseRotateXSensitivity);
                        Manager::GetSingleton()->RotateY(move->mouseInputY * config->MouseRotateYSensitivity);
                        block = true;
                    }
                    if (Manager::GetTranslateZ()) {
                        Manager::GetSingleton()->TranslateZ(-move->mouseInputY * config->MouseTranslateZSensitivity);
                        block = true;
                    }
                    if (Manager::GetDoTranslate()) {
                        Manager::GetSingleton()->TranslateX(move->mouseInputX * config->MouseTranslateXSensitivity);
                        Manager::GetSingleton()->TranslateY(-move->mouseInputY * config->MouseTranslateYSensitivity);
                        block = true;
                    }
                    return block;
                } 
            }
            if (event->device == RE::INPUT_DEVICE::kGamepad) {
                if (const auto move = event->AsThumbstickEvent()) {
                    bool block = false;
                    if (Manager::GetDoRotate()) {
                        Manager::GetSingleton()->RotateX(move->xValue * config->GamepadRotateXSensitivity);
                        Manager::GetSingleton()->RotateY(-move->yValue * config->GamepadRotateYSensitivity);
                        block = true;
                    }
                    if (Manager::GetTranslateZ()) {
                        Manager::GetSingleton()->TranslateZ(move->yValue * config->GamepadTranslateZSensitivity);
                        block = true;
                    }
                    if (Manager::GetDoTranslate()) {
                        Manager::GetSingleton()->TranslateX(move->xValue * config->GamepadTranslateXSensitivity);
                        Manager::GetSingleton()->TranslateY(move->yValue * config->GamepadTranslateYSensitivity);
                        block = true;
                    }
                    return block;
                }
            }

            return false;
        }
        static void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent* const* a_event) {
            auto manager = Manager::GetSingleton();
            auto player = RE::PlayerCharacter::GetSingleton();

            if (player->GetGrabbedRef() == nullptr) {
                manager->SetGrabbing(false, nullptr);
            }

            if (!manager->GetIsGrabbing()) {
                originalFunction(a_dispatcher, a_event);
                return;
            }

            const auto ui = RE::UI::GetSingleton();
            if (ui->IsApplicationMenuOpen() || ui->IsItemMenuOpen() || ui->IsModalMenuOpen() || ui->GameIsPaused()) {
                originalFunction(a_dispatcher, a_event);
                return;
            }
            if (is_grab_n_throw_installed && a_event && *a_event) {
                if (const auto button = (*a_event)->AsButtonEvent(); button && button->IsUp()) {
					const auto device = button->GetDevice();
				    if (device == RE::INPUT_DEVICE::kKeyboard || device == RE::INPUT_DEVICE::kGamepad) {
            	        const auto controlMap = RE::ControlMap::GetSingleton();
				        const auto userEvents = RE::UserEvents::GetSingleton();
                        const auto readyweap_button = controlMap->GetMappedKey(userEvents->readyWeapon, device,
                                                       RE::UserEvents::INPUT_CONTEXT_ID::kGameplay);
					    if (button->GetIDCode() == readyweap_button) {
							Manager::GetSingleton()->resetVelocityOnGrabEnd.store(false);
						    return originalFunction(a_dispatcher, a_event);
					    }
                    }
                }
            }

            auto first = *a_event;
            auto last = *a_event;
            size_t length = 0;

            for (auto current = *a_event; current; current = current->next) {
                if (const bool suppress = InputEvent(current)) {
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

        static void Install(HookBuilder* builder) {
            builder->AddCall<ProcessInputQueueHook, 5, 14>(
                67315, 0x7B, 
                68617, 0x7B//,
                //0xC519E0, 0x81
            );
        }
    };

    inline void Install() {
        const auto builder = new HookBuilder();
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
