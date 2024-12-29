#pragma once
#include <cmath>
#include "RayCollector.h"

namespace RayMath{
    inline float pointDistance(const RE::NiPoint3 a, const RE::NiPoint3 b) {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    inline RE::NiPoint3 angles2dir(const RE::NiPoint3& angles) {
        RE::NiPoint3 ans;

        const float sinx = sinf(angles.x);
        const float cosx = cosf(angles.x);
        const float sinz = sinf(angles.z);
        const float cosz = cosf(angles.z);

        ans.x = cosx * sinz;
        ans.y = cosx * cosz;
        ans.z = -sinx;

        return ans;
    }

    inline RE::NiPoint3 rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles) {
        RE::NiMatrix3 R;
        R.EulerAnglesToAxesZXY(angles);
        return R * A;
    }
    inline RE::NiPoint3 rotate(const float r, const RE::NiPoint3& angles) { return angles2dir(angles) * r; }

    inline [[maybe_unused]] RE::MagicTarget* FindPickTarget(RE::MagicCaster* caster, RE::NiPoint3& a_targetLocation,
                                                     RE::TESObjectCELL** a_targetCell, RE::bhkPickData& a_pickData) {
        using func_t = RE::MagicTarget*(RE::MagicCaster * caster, RE::NiPoint3 & a_targetLocation,
                                        RE::TESObjectCELL * *a_targetCell, RE::bhkPickData & a_pickData);
        const REL::Relocation<func_t> func{RELOCATION_ID(33676, 34456)};
        return func(caster, a_targetLocation, a_targetCell, a_pickData);
    }
}

struct RayCastResult;
class RayCast {
        static std::pair<RE::NiPoint3, RE::TESObjectREFR*> CastRay(
            RE::NiPoint3 angle, RE::NiPoint3 position,
            std::function<bool(RE::NiAVObject*)> const& evaluator, float raySize);
    public:
        static RE::NiPoint3 QuaternionToEuler(const RE::NiQuaternion& q);
        static std::pair<RE::NiPoint3, RE::NiPoint3> GetCameraData();
        static RayCastResult Cast(std::function<bool(RE::NiAVObject*)> const& evaluator, float raySize = 2000000000.f);


};

struct RayCastResult {
    RE::NiPoint3 position;
    RE::TESObjectREFR* object;
};