#pragma once
#include <glm/glm.hpp>

class Geometry {
    std::vector<RE::NiPoint3> positions;
    std::vector<uint16_t> indexes;
    RE::TESObjectREFR* obj;

	void FetchVertexes(RE::BSGraphics::TriShape* triShape);
    void FetchIndexes(RE::BSGraphics::TriShape* triShape);

    RE::NiPoint3 Rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles);

public:

    ~Geometry();
    Geometry(RE::TESObjectREFR* obj);
    void DrawEdges(RE::NiPoint3 position, RE::NiPoint3 angle, float scale, glm::vec4 color = {1.0,0.0,0.0,1.0});
    std::pair<RE::NiPoint3, RE::NiPoint3> GetBoundingBox(RE::NiPoint3 angle, float scale);
};