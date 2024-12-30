#include "DrawDebugExtension.h"

RE::NiPoint3 Rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles) {
    RE::NiMatrix3 R;
    R.SetEulerAnglesXYZ(angles);
    return R * A;
}
void DrawDebug::DrawBoundingBox(RE::TESObjectREFR* refr, const glm::vec4& color) {
    if (!refr) {
        return;
    }

    auto center = refr->GetPosition();

    auto from = refr->GetBoundMin();
    auto to = refr->GetBoundMax();
    from = center + from;
    to = center + to;

    RE::NiPoint3 angle;

    if (refr->As<RE::Actor>()) {
        angle = {0, 0, refr->GetAngleZ()};
    } else {
        angle = refr->GetAngle();
    }

    DrawBox(from, to, center, angle, color);
}


void DrawDebug::DrawBox(RE::NiPoint3& from, RE::NiPoint3& to, RE::NiPoint3& center, RE::NiPoint3 euler,
                        const glm::vec4& color) {

    const auto v1 = Rotate(RE::NiPoint3(from.x, from.y, from.z) - center, euler) + center;
    const auto v2 = Rotate(RE::NiPoint3(to.x, from.y, from.z) - center, euler) + center;
    const auto v3 = Rotate(RE::NiPoint3(to.x, to.y, from.z) - center, euler) + center;
    const auto v4 = Rotate(RE::NiPoint3(from.x, to.y, from.z) - center, euler) + center;

    const auto v5 = Rotate(RE::NiPoint3(from.x, from.y, to.z) - center, euler) + center;
    const auto v6 = Rotate(RE::NiPoint3(to.x, from.y, to.z) - center, euler) + center;
    const auto v7 = Rotate(RE::NiPoint3(to.x, to.y, to.z) - center, euler) + center;
    const auto v8 = Rotate(RE::NiPoint3(from.x, to.y, to.z) - center, euler) + center;

    // Draw bottom face
    DrawLine(v1, v2, color);
    DrawLine(v2, v3, color);
    DrawLine(v3, v4, color);
    DrawLine(v4, v1, color);

    // Draw top face
    DrawLine(v5, v6, color);
    DrawLine(v6, v7, color);
    DrawLine(v7, v8, color);
    DrawLine(v8, v5, color);

    // Connect bottom and top faces
    DrawLine(v1, v5, color);
    DrawLine(v2, v6, color);
    DrawLine(v3, v7, color);
    DrawLine(v4, v8, color);
}

void DrawDebug::DrawCube(RE::NiPoint3& center, float radius, RE::NiPoint3 euler, const glm::vec4& color) {

    auto length = RE::NiPoint3(radius, radius, radius);
    auto from = center - length;
    auto to = center + length;
    DrawBox(from, to, from, euler, color);
}

void DrawDebug::DrawCube(RE::NiPoint3& center, float radius, const glm::vec4& color) {
    DrawCube(center, radius, {0, 0, 0}, color);
}
