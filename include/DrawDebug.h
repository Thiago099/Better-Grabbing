#pragma once
#include <glm/glm.hpp>
//https://github.com/fenix31415/UselessFenixUtils/blob/master/include/UselessDebugRenderUtils.h

namespace DrawDebug {
    void OnMessage(SKSE::MessagingInterface::Message* message);

    namespace Color {
        static constexpr glm::vec4 Red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        static constexpr glm::vec4 Green = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        static constexpr glm::vec4 Blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }

    void DrawLine(const RE::NiPoint3& start, const RE::NiPoint3& end, glm::vec4 color = Color::Red);
    void DrawSphere(const RE::NiPoint3& a_center, float a_radius, glm::vec4 a_color); 
    void Clean();
}
