#include "Manager.h"
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

    auto c = glm::rotate(glm::mat4(1.0f), -cameraAngle.z, glm::vec3(1.0f, 0.0f, 0.0f));
    auto b = glm::rotate(glm::mat4(1.0f), yoffsetRotation, glm::vec3(0.0f, 0.0f, 1.0f));
    auto a = glm::rotate(glm::mat4(1.0f), xoffsetRoation, glm::vec3(1.0f, 0.0f, 0.0f));

    auto rotationMatrix = a * b * c;

    float newYaw = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);
    float newPitch = asin(-rotationMatrix[2][0]);
    float newRoll = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);

    float x = position.x * cos(-cameraAngle.z);
    float y = position.x * sin(-cameraAngle.z);
    float z = position.y;

    auto pos = rayPosition + RE::NiPoint3(x, y, z);

    //SetPosition(obj, );
    SetAngle(obj, RE::NiPoint3(newYaw, newPitch, newRoll));

    obj->Update3DPosition(true);

    auto object3D = obj->Get3D();
    if (!object3D) {
        return; 
    }

    logger::trace("not 2d");

    auto body = object3D->GetCollisionObject()->GetRigidBody();

    if (!body) {
        return;
    }

    logger::trace("body");

    glm::vec3 targetPoint(pos.x, pos.y, pos.z);  
    glm::vec3 currentPosition = {obj->GetPositionX(), obj->GetPositionY(),obj->GetPositionZ()};  
    glm::vec3 direction = targetPoint - currentPosition;

    RE::hkVector4 velocityVector = {direction.x, direction.y, direction.z, 0.0f};

    body->SetLinearVelocity(velocityVector);  
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
