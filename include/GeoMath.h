#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <shared_mutex>
#include "Windows.h"
#include "DrawDebugExtension.h"
using glm::vec3;
using std::min;

#ifndef CONVERSION
    #define CONVERSION
namespace Conversion {
    inline RE::NiPoint3 ToSkyrimPoint(glm::vec3 point) { return RE::NiPoint3{point.x, point.y, point.z}; }
    inline glm::vec3 ToGlmPoint(RE::NiPoint3 point) { return glm::vec3{point.x, point.y, point.z}; }
}
#endif


namespace GeoMath {
    float isectSphere(const vec3& p0, const vec3& p1, const vec3& C, float R) {
        vec3 A = p0;
        vec3 B = glm::normalize(p1 - p0);
        vec3 oc = A - C;
        float a = glm::dot(B, B);
        float b = 2.0f * glm::dot(oc, B);
        float c = glm::dot(oc, oc) - R * R;
        float discriminant = b * b - 4 * a * c;
        if (discriminant > 0) {
            float t1 = (-b - std::sqrt(discriminant)) / (2 * a);
            float t2 = (-b + std::sqrt(discriminant)) / (2 * a);
            float t = min(t1, t2);
            return t >= 0.0f ? t : -1.0f;
        }
        return -1.0f;
    }

    float isectPlane(const vec3& p0, const vec3& p1, const vec3& PA, const vec3& PB, const vec3& PC,
                     vec3& intersection) {
        vec3 R0 = p0;
        vec3 D = glm::normalize(p1 - p0);
        vec3 NV = glm::normalize(glm::cross(PB - PA, PC - PA));
        float dist_isect = glm::dot(PA - R0, NV) / glm::dot(D, NV);
        intersection = R0 + D * dist_isect;
        return dist_isect;
    }

    bool PointInOrOn(const vec3& P1, const vec3& P2, const vec3& A, const vec3& B) {
        vec3 CP1 = glm::cross(B - A, P1 - A);
        vec3 CP2 = glm::cross(B - A, P2 - A);
        return glm::dot(CP1, CP2) >= 0.0f;
    }

    bool PointInOrOnTriangle(const vec3& P, const vec3& A, const vec3& B, const vec3& C) {
        return PointInOrOn(P, A, B, C) && PointInOrOn(P, B, C, A) && PointInOrOn(P, C, A, B);
    }

    float isectTriangle(const vec3& p0, const vec3& p1, const vec3& PA, const vec3& PB, const vec3& PC) {
        vec3 intersection;
        float t = isectPlane(p0, p1, PA, PB, PC, intersection);
        if (t >= 0 && PointInOrOnTriangle(intersection, PA, PB, PC)) {
            return t;
        }
        return -1.0f;
    }

    bool PointInOrOnQuad(const vec3& P, const vec3& A, const vec3& B, const vec3& C, const vec3& D) {
        return PointInOrOn(P, A, B, C) && PointInOrOn(P, B, C, D) && PointInOrOn(P, C, D, A) && PointInOrOn(P, D, A, B);
    }

    float isectQuad(const vec3& p0, const vec3& p1, const vec3& PA, const vec3& PB, const vec3& PC, const vec3& PD) {
        vec3 intersection;
        float t = isectPlane(p0, p1, PA, PB, PC, intersection);
        if (t >= 0 && PointInOrOnQuad(intersection, PA, PB, PC, PD)) {
            return t;
        }
        return -1.0f;
    }

    float isectCuboid(const vec3& p0, const vec3& p1, const vec3& pMin, const vec3& pMax) {
        float t = -1.0f;
        vec3 pl[] = {{pMin.x, pMin.y, pMin.z}, {pMax.x, pMin.y, pMin.z}, {pMax.x, pMax.y, pMin.z},
                     {pMin.x, pMax.y, pMin.z}, {pMin.x, pMin.y, pMax.z}, {pMax.x, pMin.y, pMax.z},
                     {pMax.x, pMax.y, pMax.z}, {pMin.x, pMax.y, pMax.z}};
        int il[][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {4, 0, 3, 7}, {1, 5, 6, 2}, {4, 3, 1, 0}, {3, 2, 6, 7}};
        for (auto& qi : il) {
            float ts = isectQuad(p0, p1, pl[qi[0]], pl[qi[1]], pl[qi[2]], pl[qi[3]]);
            if (ts >= 0 && (t < 0 || ts < t)) {
                t = ts;
            }
        }
        return t;
    }

    class Box {
        glm::vec3 position;
        glm::vec3 center;
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;
        glm::vec3 v4;
        glm::vec3 v5;
        glm::vec3 v6;
        glm::vec3 v7;
        glm::vec3 v8;
        RE::NiPoint3 euler;

        RE::NiPoint3 Rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles) {
            RE::NiMatrix3 R;
            R.SetEulerAnglesXYZ(angles);
            return R * A;
        }

        void FromP1AndP2(glm::vec3 from, glm::vec3 to, glm::vec3 eulerAngles) {
            using namespace Conversion;

            auto center = ToSkyrimPoint(position);
            euler = ToSkyrimPoint(eulerAngles);

            v1 = ToGlmPoint(Rotate(RE::NiPoint3(from.x, from.y, from.z) - center, euler) + center);
            v2 = ToGlmPoint(Rotate(RE::NiPoint3(to.x, from.y, from.z) - center, euler) + center);
            v3 = ToGlmPoint(Rotate(RE::NiPoint3(to.x, to.y, from.z) - center, euler) + center);
            v4 = ToGlmPoint(Rotate(RE::NiPoint3(from.x, to.y, from.z) - center, euler) + center);

            v5 = ToGlmPoint(Rotate(RE::NiPoint3(from.x, from.y, to.z) - center, euler) + center);
            v6 = ToGlmPoint(Rotate(RE::NiPoint3(to.x, from.y, to.z) - center, euler) + center);
            v7 = ToGlmPoint(Rotate(RE::NiPoint3(to.x, to.y, to.z) - center, euler) + center);
            v8 = ToGlmPoint(Rotate(RE::NiPoint3(from.x, to.y, to.z) - center, euler) + center);

        }
    public:
        Box(RE::TESObjectREFR* refr, RE::NiPoint3 position) {
            using namespace Conversion;

            auto from = refr->GetBoundMin();
            auto to = refr->GetBoundMax();

            if ((to - from).Length() < 1) {
                from = {-10, -10, 0};
                to = {10, 10, 20};
            }

            from = position + from;
            to = position + to;

            RE::NiPoint3 angle;
            if (refr->As<RE::Actor>()) {
                angle = {0, 0, refr->GetAngleZ()};
            } else {
                angle = refr->GetAngle();
            }
            this->position = ToGlmPoint(position);
            center = ToGlmPoint((from + to) / 2);
            FromP1AndP2(ToGlmPoint(from), ToGlmPoint(to), ToGlmPoint(angle));
        }
        void Draw(glm::vec4 color = DrawDebug::Color::Red) {
            using namespace Conversion;
            DrawDebug::DrawLine(ToSkyrimPoint(v1), ToSkyrimPoint(v2), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v2), ToSkyrimPoint(v3), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v3), ToSkyrimPoint(v4), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v4), ToSkyrimPoint(v1), color);

            // Draw top face
            DrawDebug::DrawLine(ToSkyrimPoint(v5), ToSkyrimPoint(v6), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v6), ToSkyrimPoint(v7), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v7), ToSkyrimPoint(v8), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v8), ToSkyrimPoint(v5), color);

            // Connect bottom and top faces
            DrawDebug::DrawLine(ToSkyrimPoint(v1), ToSkyrimPoint(v5), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v2), ToSkyrimPoint(v6), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v3), ToSkyrimPoint(v7), color);
            DrawDebug::DrawLine(ToSkyrimPoint(v4), ToSkyrimPoint(v8), color);
        }
        std::vector<std::vector<glm::vec3>> getFaces() {
            std::vector<std::vector<glm::vec3>> faces = {
                {v1, v2, v3, v4},
                {v5, v6, v7, v8},
                {v1, v5, v8, v4},
                {v2, v6, v7, v3},
                {v1, v2, v6, v5},
                {v4, v3, v7, v8}
            };
            return faces;
        }
        RE::NiPoint3 GetCenter() {
            using namespace Conversion;
            return Conversion::ToSkyrimPoint((v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8) / 8.0f);
        }
        RE::NiPoint3 GetPosition() {
            using namespace Conversion;
            return Conversion::ToSkyrimPoint(position);
        }
        RE::NiPoint3 GetDifference() {
            using namespace Conversion;

            glm::mat4 rotationMatrix = glm::yawPitchRoll(euler.x, euler.y, euler.z);

            auto vec = glm::vec4(position - center, 1.0);

            glm::vec3 positionAroundCenter = glm::vec3(rotationMatrix * vec);

            return ToSkyrimPoint(positionAroundCenter);
        }
    };


    float isectBox(const vec3& p0, const vec3& p1, Box& box) {
        float t = -1.0f;
        auto pl = box.getFaces();
        for (auto& qi : pl) {
            float ts = isectQuad(p0, p1, qi[0], qi[1], qi[2], qi[3]);
            if (ts >= 0 && (t < 0 || ts < t)) {
                t = ts;
            }
        }
        return t;
    }
    float isectBox(const RE::NiPoint3& p0, const RE::NiPoint3& p1, Box& box) {
        using namespace Conversion;
        return isectBox(ToGlmPoint(p0), ToGlmPoint(p1), box);
    }
}


