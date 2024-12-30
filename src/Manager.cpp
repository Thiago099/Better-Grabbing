#include "Manager.h"
#include "DrawDebugExtension.h"
#include "GeoMath.h"

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

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

UINT GetBufferLength(RE::ID3D11Buffer* reBuffer) {
    auto buffer = reinterpret_cast<ID3D11Buffer*>(reBuffer);
    D3D11_BUFFER_DESC bufferDesc = {};
    buffer->GetDesc(&bufferDesc);
    return bufferDesc.ByteWidth;
}



void Manager::UpdateObjectTransform(RE::TESObjectREFR* obj, RayOutput& ray) const {
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

    auto pos = ray.position + RE::NiPoint3(x, y, z);

    const auto body = GetRigidBody(obj);

    if (!body) {
        return;
    }

    const auto config = Config::GetSingleton();


    SetAngle(obj, RE::NiPoint3(newYaw, newPitch, newRoll));

    #ifndef NDEBUG
    DrawDebug::Clean();
    #endif 

    auto box = GeoMath::Box(obj, pos);
        
    pos += box.GetPosition() - box.GetCenter();



    if (ray.hasHit) {
        box = GeoMath::Box(obj, pos);

        if (auto d3d = obj->Get3D()) {

            int i = 0;
            RE::BSGeometry* geo = nullptr;
            RE::BSVisit::TraverseScenegraphGeometries(d3d, [&](RE::BSGeometry* a_geometry) -> RE::BSVisit::BSVisitControl {
                geo = a_geometry;
                i++;
                return RE::BSVisit::BSVisitControl::kContinue;
            });

            logger::trace("num geo: {}", i);

            if (geo) {
                auto& model = geo->GetGeometryRuntimeData();
                if (auto triShape = model.rendererData) {
                    const uint8_t* vertexData = triShape->rawVertexData;

                    if (vertexData) {
                    
                        uint32_t stride = triShape->vertexDesc.GetSize();

                        auto numPoints = GetBufferLength(triShape->vertexBuffer);
                        auto numIndexes = GetBufferLength(triShape->indexBuffer) / sizeof(uint16_t);

                        RE::NiPoint3* positions = new RE::NiPoint3[numPoints / stride];
        
                        for (auto i = 0; i < numPoints; i += stride) {

                            const uint8_t* currentVertex = vertexData + i;

                            const float* position = reinterpret_cast<const float*>(
                                currentVertex + triShape->vertexDesc.GetAttributeOffset(
                                                    RE::BSGraphics::Vertex::Attribute::VA_POSITION));

                            auto pos = RE::NiPoint3{position[0], position[1], position[2]} + obj->GetPosition();
                            positions[i / stride] = pos;
                        }

                        logger::trace("num indexes {}", numIndexes);

                        for (auto i = 0; i < numIndexes; i += 3) {
                            const uint16_t* currentIndex = triShape->rawIndexData + i;
                            auto p1 = positions[currentIndex[0]];
                            auto p2 = positions[currentIndex[1]];
                            auto p3 = positions[currentIndex[2]];
                            DrawDebug::DrawLine(p1, p2);
                            DrawDebug::DrawLine(p2, p3);
                            DrawDebug::DrawLine(p3, p1);
                        }
                        delete[] positions;
                    } else {
                        logger::trace("missing vertex");
                    }
                } else {
                    logger::trace("missing shape");
                }
            } else {
                logger::trace("missing geo");
            }
        } else {
            logger::trace("missing 3d");
        }

        auto center = box.GetCenter();

        auto end = center + RE::NiPoint3(0, 0, 1) * 1000;

        auto ratio = GeoMath::isectBox(end, center, box);

        auto length = (center - end).Length();

        if (length != 0) {
            auto r = ratio / length;
            auto position = (center * r) + (end * (1 - r));

            pos -= box.GetCenter() - position;

            #ifndef NDEBUG

            DrawDebug::DrawLine(center, position, {1.0, 0.0, 1.0, 1.0});
            DrawDebug::DrawSphere(center, 1.0f, {1.0, 1.0, 0.0, 1.0});
            DrawDebug::DrawSphere(position, 1.0f, {0.0, 1.0, 1.0, 1.0});

            #endif 
        }
    }

        

    #ifndef NDEBUG
        box = GeoMath::Box(obj, pos);
        box.Draw(ray.hasHit ? DrawDebug::Color::Green : DrawDebug::Color::Red);
    #endif 



    SetPosition(obj, pos);
    obj->Update3DPosition(true);
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
        thirdPersonDistance = config->TranslateZMinDefaultDistance;
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
                        object3D->SetCollisionLayer(RE::COL_LAYER::kNonCollidable);
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

    auto[ cameraAngle,cameraPosition ] = RayCast::GetCameraData();

    auto player = RE::PlayerCharacter::GetSingleton();

    auto playerHeadPosition = player->GetPosition() + RE::NiPoint3{0, 0, player->GetHeight()};

    if (camera->currentState->id == RE::CameraState::kThirdPerson) {
        rayMaxDistance = thirdPersonDistance + (cameraPosition-playerHeadPosition).Length() * 2;
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

        auto ray = RayCast::Cast(evaluator, rayMaxDistance);

        UpdateObjectTransform(obj, ray);
    });
}

RE::BSContainer::ForEachResult ActiveEffectVisitor::Accept(RE::ActiveEffect* a_effect)
{
    if (!grabbed_obj) return RE::BSContainer::ForEachResult::kStop;
	if (!a_effect) return RE::BSContainer::ForEachResult::kContinue;
	const auto telekinesis_effect = skyrim_cast<RE::TelekinesisEffect*>(a_effect);
	if (!telekinesis_effect) return RE::BSContainer::ForEachResult::kContinue;
    const auto telekinesis_obj = telekinesis_effect->grabbedObject.get();
	if (!telekinesis_obj) return RE::BSContainer::ForEachResult::kContinue;
	if (telekinesis_obj->GetFormID() != grabbed_obj->GetFormID()) {
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
