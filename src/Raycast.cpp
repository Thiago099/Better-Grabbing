#include "Raycast.h"



enum class LineOfSightLocation : uint32_t { kNone, kEyes, kHead, kTorso, kFeet };

RE::NiPoint3 RayCast::QuaternionToEuler(const RE::NiQuaternion& q) {
    RE::NiPoint3 euler;

    const double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    const double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    euler.x = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    if (const double sinp = 2 * (q.w * q.y - q.z * q.x); std::abs(sinp) >= 1)
        euler.y = std::copysign(glm::pi<float>() / 2, sinp);
    else
        euler.y = std::asin(sinp);

    // Yaw (z-axis rotation)
    const double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    const double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    euler.z = std::atan2(siny_cosp, cosy_cosp);

    euler.x = euler.x * -1;
    //euler.y = euler.y;
    euler.z = euler.z * -1;

    return euler;
}

std::pair<RE::NiPoint3, RE::NiPoint3> RayCast::GetCameraData() {
    const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();
    const auto thirdPerson =
        reinterpret_cast<RE::ThirdPersonState*>(camera->cameraStates[RE::CameraState::kThirdPerson].get());
    const auto firstPerson =
        reinterpret_cast<RE::FirstPersonState*>(camera->cameraStates[RE::CameraState::kFirstPerson].get());

    RE::NiQuaternion rotation;
    RE::NiPoint3 translation;
    if (camera->currentState->id == RE::CameraState::kFirstPerson) {
        firstPerson->GetRotation(rotation);
        firstPerson->GetTranslation(translation);
        translation += firstPerson->dampeningOffset;
    } else if (camera->currentState->id == RE::CameraState::kThirdPerson) {
        rotation = thirdPerson->rotation;
        translation = thirdPerson->translation;
    } else {
        return {};
    }
    return {QuaternionToEuler(rotation), translation};
}

RayCastResult RayCast::Cast(std::function<bool(RE::NiAVObject*)> const& evaluator, const float raySize) {

    auto [camera_rotation, camera_position] = GetCameraData();

    auto [ray_position, ray_object] = CastRay(camera_rotation, camera_position, evaluator, raySize);
    return RayCastResult(ray_position, ray_object);
}

std::pair<RE::NiPoint3, RE::TESObjectREFR*> RayCast::CastRay(
    RE::NiPoint3 angle, RE::NiPoint3 position,
    std::function<bool(RE::NiAVObject*)> const& evaluator, float raySize) {
    using namespace RayMath;
    auto havokWorldScale = RE::bhkWorld::GetWorldScale();
    RE::bhkPickData pick_data;
    RE::NiPoint3 ray_start, ray_end;

    ray_start = position;
    ray_end = ray_start + rotate(raySize, angle);
    pick_data.rayInput.from = ray_start * havokWorldScale;
    pick_data.rayInput.to = ray_end * havokWorldScale;

    auto dif = ray_start - ray_end;

    auto collector = RayCollector(evaluator);
    collector.Reset();
    pick_data.rayHitCollectorA8 = reinterpret_cast<RE::hkpClosestRayHitCollector*>(&collector);

    const auto ply = RE::PlayerCharacter::GetSingleton();
    if (!ply->parentCell) return {};

    if (auto physicsWorld = ply->parentCell->GetbhkWorld()) {
        physicsWorld->PickObject(pick_data);
    }

    RayCollector::HitResult best = {};
    best.hitFraction = 1.0f;
    RE::NiPoint3 bestPos = {};

    for (auto& hit : collector.GetHits()) {
        const auto pos = (dif * hit.hitFraction) + ray_start;
        if (best.body == nullptr) {
            best = hit;
            bestPos = pos;
            continue;
        }

        if (hit.hitFraction < best.hitFraction) {
            best = hit;
            bestPos = pos;
        }
    }

    if (!best.body) {
        return std::pair(ray_end, nullptr);
    }

    auto hitpos = ray_start + (ray_end - ray_start) * best.hitFraction;

    if (auto av = best.getAVObject()) {
        auto ref = av->GetUserData();

        return {hitpos, ref};
    }
    return std::pair(hitpos, nullptr);
}
