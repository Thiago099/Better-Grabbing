#include "Geometry.h"
#include "DrawDebugExtension.h"
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include "GeoMath.h"
UINT GetBufferLength(RE::ID3D11Buffer* reBuffer) {
    auto buffer = reinterpret_cast<ID3D11Buffer*>(reBuffer);
    D3D11_BUFFER_DESC bufferDesc = {};
    buffer->GetDesc(&bufferDesc);
    return bufferDesc.ByteWidth;
}


void EachGeometry(RE::TESObjectREFR* obj, std::function<void(RE::BSGeometry* o3d, RE::BSGraphics::TriShape*)> callback) {
    if (auto d3d = obj->Get3D()) {
        int i = 0;

        RE::BSVisit::TraverseScenegraphGeometries(d3d, [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {

            auto& model = a_geometry->GetGeometryRuntimeData();

            if (auto triShape = model.rendererData) {

                callback(a_geometry, triShape);
            }

            return RE::BSVisit::BSVisitControl::kContinue;
        });


    }
}
bool ToYawPitchRoll(RE::NiMatrix3& angle) {
    float yaw = 0;
    float pitch = 0;
    float roll = 0;
    pitch = std::asin(-angle.entry[2][0]);
    if (std::abs(angle.entry[2][0]) < 0.9999f) {
        yaw = std::atan2(angle.entry[2][1], angle.entry[2][2]);
        roll = std::atan2(angle.entry[1][0], angle.entry[0][0]);
    } else {
        yaw = 0.0f;
        roll = std::atan2(-angle.entry[0][1], angle.entry[1][1]);
    }
    return true;
}
void Geometry::FetchVertexes(RE::BSGeometry* o3d, RE::BSGraphics::TriShape* triShape) {
    if (const uint8_t* vertexData = triShape->rawVertexData) {
        uint32_t stride = triShape->vertexDesc.GetSize();
        auto numPoints = GetBufferLength(triShape->vertexBuffer);
        auto numPositions = numPoints / stride;
        positions.reserve(positions.size() + numPositions);
        logger::trace("NP:{}", numPositions);
        for (auto i = 0; i < numPoints; i += stride) {
            const uint8_t* currentVertex = vertexData + i;

            const float* position =
                reinterpret_cast<const float*>(currentVertex + triShape->vertexDesc.GetAttributeOffset(
                                                                   RE::BSGraphics::Vertex::Attribute::VA_POSITION));

            auto pos = RE::NiPoint3{position[0], position[1], position[2]};
            pos *= o3d->local.scale;
            pos += o3d->local.translate;
            pos = o3d->local.rotate * pos;
            positions.push_back(pos);
        }
    }
}
void Geometry::FetchIndexes(RE::BSGraphics::TriShape* triShape) {
    auto numIndexes = GetBufferLength(triShape->indexBuffer) / sizeof(uint16_t);
    logger::trace("NI:{}", numIndexes);

    auto offset = indexes.size(); 
    indexes.reserve(indexes.size() + numIndexes);

    for (auto i = 0; i < numIndexes; i++) {
        const uint16_t* currentIndex = triShape->rawIndexData + i;
        indexes.push_back(currentIndex[0] + offset);
    }
}

RE::NiPoint3 Geometry::Rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles) {
    RE::NiMatrix3 R;
    R.SetEulerAnglesXYZ(angles);
    return R * A;
}




Geometry::~Geometry() {
}

Geometry::Geometry(RE::TESObjectREFR* obj) {
        this->obj = obj;
    EachGeometry(obj, [this](RE::BSGeometry* o3d, RE::BSGraphics::TriShape* triShape) -> void {
        FetchVertexes(o3d, triShape);
        FetchIndexes(triShape);
    });

}

void Geometry::DrawEdges(RE::NiPoint3 position, RE::NiPoint3 angle, float scale, glm::vec4 color) {
    if (this->obj) {

        for (auto i = 0; i < indexes.size(); i += 3) {

            if (i + 2 >= indexes.size()) {
                break;
            }

            auto i0 = indexes[i];
            auto i1 = indexes[i+1];
            auto i2 = indexes[i+2];

            if (i0 >= positions.size()) {
                continue;
            }

            if (i1 >= positions.size()) {
                continue;
            }

            if (i2 >= positions.size()) {
                continue;
            }

            auto p1 = Rotate(positions[i0] * obj->GetScale(), angle) + position;
            auto p2 = Rotate(positions[i1] * obj->GetScale(), angle) + position;
            auto p3 = Rotate(positions[i2] * obj->GetScale(), angle) + position;

            DrawDebug::DrawLine(p1, p2, color);
            DrawDebug::DrawLine(p2, p3, color);
            DrawDebug::DrawLine(p3, p1, color);
        }
    }
}


std::pair<RE::NiPoint3, RE::NiPoint3> Geometry::GetBoundingBox(RE::NiPoint3 angle, float scale) {
    auto min = RE::NiPoint3{0, 0, 0};
    auto max = RE::NiPoint3{0, 0, 0};

    for (auto i = 0; i < positions.size(); i++) {
        auto p1 = Rotate(positions[i] * obj->GetScale(), angle);

        if (p1.x < min.x) {
            min.x = p1.x;
        }
        if (p1.x > max.x) {
            max.x = p1.x;
        }
        if (p1.y < min.y) {
            min.y = p1.y;
        }
        if (p1.y > max.y) {
            max.y = p1.y;
        }
        if (p1.z < min.z) {
            min.z = p1.z;
        }
        if (p1.z > max.z) {
            max.z = p1.z;
        }
    }

    return std::pair(min, max);

}
