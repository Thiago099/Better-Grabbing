#include "Manager.h"

namespace {
    RE::bhkRigidBody* GetRigidBody(const RE::TESObjectREFR* refr) {
        const auto object3D = refr->Get3D();

        if (!object3D) {
            return nullptr;
        }
        const auto collision = object3D->GetCollisionObject();

        if (!collision) {
            return nullptr;
        }

        const auto body = collision->GetRigidBody();

        return body;
    }

    void SetPosition(RE::TESObjectREFR* ref, const RE::NiPoint3& a_position) {
        if (!ref) {
            return;
        }
        using func_t = void(RE::TESObjectREFR*, const RE::NiPoint3&);
        const REL::Relocation<func_t> func{RELOCATION_ID(19363, 19790)};
        return func(ref, a_position);
    }

    void SetAngle(RE::TESObjectREFR* ref, const RE::NiPoint3& a_position) {
        if (!ref) {
            return;
        }
        using func_t = void(RE::TESObjectREFR*, const RE::NiPoint3&);
        const REL::Relocation<func_t> func{RELOCATION_ID(19359, 19786)};
        return func(ref, a_position);
    }

    RE::NiObject* GetPlayer3d() {
        const auto refr = RE::PlayerCharacter::GetSingleton();
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
}

void Manager::UpdateObjectTransform(RE::TESObjectREFR* obj, const RE::NiPoint3& rayPosition) const {
    auto [cameraAngle, cameraPosition] = RayCast::GetCameraData();

    const auto yoffsetRotation = angle.y;
    const auto xoffsetRoation = angle.x;

    const auto a = glm::rotate(glm::mat4(1.0f), xoffsetRoation, glm::vec3(1.0f, 0.0f, 0.0f));
    const auto b = glm::rotate(glm::mat4(1.0f), yoffsetRotation, glm::vec3(0.0f, 0.0f, 1.0f));
    const auto c = glm::rotate(glm::mat4(1.0f), -cameraAngle.z, glm::vec3(1.0f, 0.0f, 0.0f));

    auto rotationMatrix = a * b * c;

    const float newYaw = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);
    const float newPitch = asin(-rotationMatrix[2][0]);
    const float newRoll = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);

    const float x = position.x * cos(-cameraAngle.z);
    const float y = position.x * sin(-cameraAngle.z);
    const float z = position.y;

    const auto pos = rayPosition + RE::NiPoint3(x, y, z);

    const auto body = GetRigidBody(obj);

    if (!body) {
        return;
    }

    const auto config = Config::GetSingleton();

    if (config->EnablePhysicalBasedMovement) {
        auto direction = (pos - obj->GetPosition()) * config->DragMovementDamping;

        RE::hkVector4 velocityVector(direction);

        RE::hkVector4 havockPosition;
        body->GetPosition(havockPosition);
        float components[4];
        _mm_store_ps(components, havockPosition.quad);
        RE::NiPoint3 newPosition = {components[0], components[1], components[2]};
        constexpr auto havockToSkyrimConversionRate = 69.9915f;
        newPosition *= havockToSkyrimConversionRate;

        SetAngle(obj, RE::NiPoint3(newYaw, newPitch, newRoll));
        SetPosition(obj, newPosition);
        obj->Update3DPosition(true);
        body->SetLinearVelocity(velocityVector);
    } else {
        SetAngle(obj, RE::NiPoint3(newYaw, newPitch, newRoll));
        SetPosition(obj, pos);
        obj->Update3DPosition(true);
    }
}

float Manager::NormalizeAngle(float angle) {
    angle = glm::mod(angle + glm::pi<float>(), glm::two_pi<float>());
    if (angle < 0.0f) angle += glm::two_pi<float>();
    return angle - glm::pi<float>();
}

void Manager::SetGrabbing(const bool value, const RE::TESObjectREFRPtr& ref) {
    if (value) {
        const auto config = Config::GetSingleton();
        angle = {0, 0};
        fistPersonDistance = config->TranslateZMinDefaultDistance;
        thirdPersonDistance = config->TranslateZMinDefaultThirdPersonDistance;
        position = {0, 0};

        doRotate = false;
        doTranslate = false;
        doTranslateZ = false;

        if (ref) {
            if (const auto ref2 = ref.get()) {
                if (ref2->As<RE::Actor>()) {
                    return;
                }
                auto [cameraAngle, cameraPosition] = RayCast::GetCameraData();
                const auto objectAngle = ref2->GetAngle();
                angle = {-objectAngle.z + cameraAngle.z, 0};

                if (const auto body = GetRigidBody(ref2)) {
                    body->SetLinearVelocity(RE::hkVector4());
                    body->SetAngularVelocity(RE::hkVector4());
                }

                if (config->DisableCollisionWithItemsWhileGrabbing) {
                    if (const auto object3D = ref2->Get3D()) {
                        oldCollisionLayer = object3D->GetCollisionLayer();
                        object3D->SetCollisionLayer(RE::COL_LAYER::kCamera);
                    }
                }
            }
        }

    } else {
        if (ref) {
            if (const auto ref2 = ref.get()) {
                if (ref2->As<RE::Actor>()) {
                    return;
                }

                if (resetVelocityOnGrabEnd.load()) {
                    if (const auto body = GetRigidBody(ref2)) {
                        body->SetLinearVelocity({});
                    }
                }

                if (const auto config = Config::GetSingleton(); config->DisableCollisionWithItemsWhileGrabbing) {
                    if (const auto object3D = ref2->Get3D()) {
                        object3D->SetCollisionLayer(oldCollisionLayer);
                    }
                }
            }
        }
    }
    isGrabbing = value;
    resetVelocityOnGrabEnd.store(true);
}

void Manager::UpdatePosition(RE::TESObjectREFR* obj) const {
    auto rayMaxDistance = 0.f;

    const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();

    if (camera->currentState->id == RE::CameraState::kThirdPerson) {
        rayMaxDistance = thirdPersonDistance;
    } else {
        rayMaxDistance = fistPersonDistance;
    }

    SKSE::GetTaskInterface()->AddTask([this, obj, rayMaxDistance]() {
        auto player3d = GetPlayer3d();

        const auto evaluator = [player3d, obj](const RE::NiAVObject* mesh) {
            if (mesh == player3d) {
                return false;
            }
            if (mesh->GetUserData() == obj) {
                return false;
            }

            return true;
        };

        const auto ray = RayCast::Cast(evaluator, rayMaxDistance);

        UpdateObjectTransform(obj, ray.position);
    });
}

RE::BSContainer::ForEachResult ActiveEffectVisitor::Accept(RE::ActiveEffect* a_effect)
{
    if (!grabbed_obj) return RE::BSContainer::ForEachResult::kStop;
	if (!a_effect) return RE::BSContainer::ForEachResult::kContinue;
	const auto telekinesis_effect = skyrim_cast<RE::TelekinesisEffect*>(a_effect);
	if (!telekinesis_effect) return RE::BSContainer::ForEachResult::kContinue;
	if (telekinesis_effect->grabbedObject.get()->GetFormID() != grabbed_obj->GetFormID()) {
	    return RE::BSContainer::ForEachResult::kContinue;
	}
	is_using_telekinesis.store(true);
	return RE::BSContainer::ForEachResult::kStop;
}

void ActiveEffectVisitor::Reset(RE::TESObjectREFR* grabbed_object)
{
	this->grabbed_obj = grabbed_object;
	is_using_telekinesis.store(false);
}


bool Manager::IsTelekinesisObject(RE::TESObjectREFR* grabbed_ob)
{
    const auto visitor = ActiveEffectVisitor::GetSingleton();
	visitor->Reset(grabbed_ob);
	if (const auto magicTarget = RE::PlayerCharacter::GetSingleton()->AsMagicTarget()) {
		magicTarget->VisitEffects(*visitor);
	}
	return visitor->is_using_telekinesis.load();
}
