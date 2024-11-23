#pragma once
#include "HookBuilder.h"
#include "Raycast.h"

void SetPosition(RE::TESObjectREFR* ref, const RE::NiPoint3& a_position) {
    if (!ref) {
        return;
    }
    using func_t = void(RE::TESObjectREFR*, const RE::NiPoint3&);
    REL::Relocation<func_t> func{RELOCATION_ID(19363, 19790)};
    return func(ref, a_position);
}

void SetAngle(RE::TESObjectREFR* ref, const RE::NiPoint3& a_position) {
    if (!ref) {
        return;
    }
    using func_t = void(RE::TESObjectREFR*, const RE::NiPoint3&);
    REL::Relocation<func_t> func{RELOCATION_ID(19359, 19786)};
    return func(ref, a_position);
}


RE::NiObject* GetPlayer3d() {
    auto refr = RE::PlayerCharacter::GetSingleton();
    if (!refr) {
        return nullptr;
    }
    if (!refr->loadedData) {
        return nullptr;
    }
    if (!refr->loadedData->data3D) {
        return nullptr;
    }
    return refr->loadedData->data3D.get();
}


inline void UpdateObjectTransform(RE::TESObjectREFR* obj,RE::NiPoint3& rayPosition) {
    auto [cameraAngle, cameraPosition] = RayCast::GetCameraData();

    auto yoffsetRotation = 0.f;
    auto xoffsetRoation = 0.f;

    auto c = glm::rotate(glm::mat4(1.0f), -cameraAngle.z, glm::vec3(1.0f, 0.0f, 0.0f));
    auto b = glm::rotate(glm::mat4(1.0f), yoffsetRotation, glm::vec3(0.0f, 0.0f, 1.0f));
    auto a = glm::rotate(glm::mat4(1.0f), xoffsetRoation, glm::vec3(1.0f, 0.0f, 0.0f));

    auto rotationMatrix = a * b * c;

    float newYaw = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);
    float newPitch = asin(-rotationMatrix[2][0]);
    float newRoll = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);

    float x = 0.f * cos(-cameraAngle.z);
    float y = 0.f * sin(-cameraAngle.z);
    float z = 0.f;

    SetPosition(obj, rayPosition + RE::NiPoint3(x, y, z));
    SetAngle(obj, RE::NiPoint3(newYaw, newPitch, newRoll));

    obj->Update3DPosition(true);
}



namespace Hooks {

    struct GrabHook {
        static inline bool thunk(RE::ObjectRefHandle* a1, RE::TESObjectREFRPtr* a2) { 
            auto obj = a1->get();
            if (obj) {
                auto obj2 = obj.get();
                if (obj2) {
                    if (obj2->As<RE::Actor>()) {
                        return originalFunction(a1, a2);
                    } else {

                          SKSE::GetTaskInterface()->AddTask([obj2]() {
                            auto player3d = GetPlayer3d();

                            const auto evaluator = [player3d, obj2](RE::NiAVObject* mesh) {
                                if (mesh == player3d) {
                                    return false;
                                }
                                if (mesh->GetUserData() == obj2) {
                                    return false;
                                }

                                return true;
                            };

                            auto ray = RayCast::Cast(evaluator,180.f);

                            UpdateObjectTransform(obj2, ray.position);
                        });

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
            return false;
        }
        static inline REL::Relocation<decltype(thunk)> originalFunction;
        inline static void Install(HookBuilder* builder) {
            //SE ID: 39479 SE Offset: 0xb85 (Heuristic)
            //AE ID: 40556 AE Offset: 0xaf0
            builder->AddCall<GrabHook2, 5, 14>(39479, 0xb85, 40556, 0xaf0);
        }
    };
    inline void Install() {
        auto builder = new HookBuilder();
        GrabHook::Install(builder);
        GrabHook2::Install(builder);

        // uint8_t data3[] = {0xeb, 0x23};  // allow disenchant anything // SkyrimSE.exe+9091ac //  0x90
        //REL::safe_write(REL::RelocationID(50440, 51344).address() + REL::Relocate(0x75, 0x7c), data3, sizeof(data3));

        //uint8_t data4[] = {0xeb};
        //REL::safe_write(REL::ID(40556).address() + 0x374, data4, sizeof(data4));

        builder->Install();
    }
}
