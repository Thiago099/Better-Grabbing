#include "Manager.h"


RE::bhkRigidBody* GetRigidBody(RE::TESObjectREFR* refr) {
    auto object3D = refr->Get3D();

    if (!object3D) {
        return NULL;
    }
    auto body = object3D->GetCollisionObject()->GetRigidBody();

    return body;
}


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


void Manager::UpdateObjectTransform(RE::TESObjectREFR* obj, RE::NiPoint3& rayPosition) {
    auto [cameraAngle, cameraPosition] = RayCast::GetCameraData();

    auto yoffsetRotation = angle.y;
    auto xoffsetRoation = angle.x;

    auto a = glm::rotate(glm::mat4(1.0f), xoffsetRoation, glm::vec3(1.0f, 0.0f, 0.0f));
    auto b = glm::rotate(glm::mat4(1.0f), yoffsetRotation, glm::vec3(0.0f, 0.0f, 1.0f));
    auto c = glm::rotate(glm::mat4(1.0f), -cameraAngle.z, glm::vec3(1.0f, 0.0f, 0.0f));

    auto rotationMatrix = a * b * c;


    float newYaw = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);
    float newPitch = asin(-rotationMatrix[2][0]);
    float newRoll = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);

    float x = position.x * cos(-cameraAngle.z);
    float y = position.x * sin(-cameraAngle.z);
    float z = position.y;

    auto pos = rayPosition + RE::NiPoint3(x, y, z);

    auto body = GetRigidBody(obj);

    if (!body) {
        return;
    }

    auto config = Config::GetSingleton();
        
    auto direction = (pos - obj->GetPosition()) * config->DragMovementDamping;

    RE::hkVector4 velocityVector(direction);

    SetAngle(obj, RE::NiPoint3(newYaw, newPitch, newRoll));
    obj->Update3DPosition(true);
    SKSE::GetTaskInterface()->AddTask([body, velocityVector]() {
        body->SetLinearVelocity(velocityVector);
    });
}

void Manager::SetGrabbing(bool value, RE::TESObjectREFRPtr ref) {
    if (value) {
        auto config = Config::GetSingleton();
        angle = {0, 0};
        distance = config->TranslateZMinDefaultDistance;
        position = {0, 0};
        if (ref) {
            if (auto ref2 = ref.get()) {
                auto [cameraAngle, cameraPosition] = RayCast::GetCameraData();
                auto objectAngle = ref2->GetAngle();
                angle = {-objectAngle.z + cameraAngle.z, 0};
            }
        }
    }
    isGrabbing = value;
}

void Manager::UpdatePosition(RE::TESObjectREFR* obj) {
    auto rayMaxDistance = distance;
    SKSE::GetTaskInterface()->AddTask([this, obj, rayMaxDistance]() {
        auto player3d = GetPlayer3d();

        const auto evaluator = [player3d, obj](RE::NiAVObject* mesh) {
            if (mesh == player3d) {
                return false;
            }
            if (mesh->GetUserData() == obj) {
                return false;
            }

            return true;
        };

        auto ray = RayCast::Cast(evaluator, rayMaxDistance);

        UpdateObjectTransform(obj, ray.position);
    });
}
