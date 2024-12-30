#pragma once
#include <algorithm>

#include "Raycast.h"
#include "InputManager.h"
#include "Config.h"

#define M_PI 3.14159265358979323846f


class Manager {
    bool isGrabbing = false;
    float fistPersonDistance = 100.f;
    float thirdPersonDistance = 100.f;
    RE::NiPoint2 angle = {0, 0};
    RE::NiPoint2 position = {0, 0};
    RE::COL_LAYER oldCollisionLayer;
    inline static Manager* singleton = nullptr;

    static inline bool doRotate = false;
    static inline bool doTranslate = false;
    static inline bool doTranslateZ = false;

    void UpdateObjectTransform(RE::TESObjectREFR* obj, RayOutput& ray) const;
    static float NormalizeAngle(float angle);

public:

	std::atomic<bool> resetVelocityOnGrabEnd = true;

    static bool GetDoRotate() {
        return doRotate;
    }

    static bool GetDoTranslate() {
        return doTranslate ;
    }

    static bool GetTranslateZ() {
        return doTranslateZ;
    }

    Manager() {
        const auto input = InputManager::GetSingleton();
        input->AddSink("Rotation", [](const RE::ButtonEvent* button) { 
            if (button->IsDown()) {
                doRotate = true;
            } else if (button->IsUp()) {
                doRotate = false;
            }
        });

        input->AddSink("Translation", [](const RE::ButtonEvent* button) {
            if (button->IsDown()) {
                doTranslate = true;
            } else if (button->IsUp()) {
                doTranslate = false;
            }
        });

        input->AddSink("ZTranslation", [](const RE::ButtonEvent* button) {
            if (button->IsDown()) {
                doTranslateZ = true;
            } else if (button->IsUp()) {
                doTranslateZ = false;
            }
        });

        input->AddSink("MoveObjectCloser", [](RE::ButtonEvent*) {
            Manager::GetSingleton()->TranslateZ(3.f);
        });

        input->AddSink("MoveObjectFurther", [](RE::ButtonEvent*) {
            Manager::GetSingleton()->TranslateZ(-3.f);
        });

        input->AddSink("RotateXPlus",
        [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            RotateX(config->ButtonRotateXSensitivity / 360 * M_PI);
        });

        input->AddSink("RotateXMinus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            RotateX(-config->ButtonRotateXSensitivity / 360 * M_PI);
        });

        input->AddSink("RotateYPlus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            RotateY(-config->ButtonRotateYSensitivity / 360 * M_PI);
        });

        input->AddSink("RotateYMinus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            RotateY(config->ButtonRotateYSensitivity / 360 * M_PI);
        });

        input->AddSink("TranslateXPlus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            TranslateX(config->ButtonTranslateXSensitivity);
        });

        input->AddSink("TranslateXMinus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            TranslateX(-config->ButtonTranslateXSensitivity);
        });

        input->AddSink("TranslateYPlus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            TranslateY(config->ButtonTranslateYSensitivity);
        });

        input->AddSink("TranslateYMinus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            TranslateY(-config->ButtonTranslateYSensitivity);
        });

        input->AddSink("TranslateZPlus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            TranslateZ(config->ButtonTranslateZSensitivity);
        });

        input->AddSink("TranslateZMinus", [this](RE::ButtonEvent*) {
            const auto config = Config::GetSingleton();
            TranslateZ(-config->ButtonTranslateZSensitivity);
        });
    }
    static Manager* GetSingleton() {
        if (!singleton) {
            singleton = new Manager();
        }
        return singleton;
    }
    void SetGrabbing(bool value, const RE::TESObjectREFRPtr& ref);
    void RotateX(const float x) {
        if (glm::abs(angle.y) > glm::half_pi<float>()) {
            angle.x = NormalizeAngle(angle.x - x);
        } else {
            angle.x = NormalizeAngle(angle.x + x);
        }
    }
    void RotateY(const float y) {
        angle.y = NormalizeAngle(angle.y + y);
    }
    void TranslateX(const float x) {
        position.x += x;
    }
    void TranslateY(const float y) {
        position.y += y;
    }
    void TranslateZ(const float value) {
        const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();

        if (camera->currentState->id == RE::CameraState::kThirdPerson) {

            auto candidate = thirdPersonDistance + value;
            const auto config = Config::GetSingleton();

            candidate = std::max(candidate, config->TranslateZMinDistance);

            candidate = std::min(candidate, config->TranslateZMaxDistance);

            thirdPersonDistance = candidate;

        } else {
            
            auto candidate = fistPersonDistance + value;
            const auto config = Config::GetSingleton();

            candidate = std::max(candidate, config->TranslateZMinDistance);

            candidate = std::min(candidate, config->TranslateZMaxDistance);

            fistPersonDistance = candidate;
        }

    }

    bool GetIsGrabbing() const {
        return isGrabbing;
    }
    void UpdatePosition(RE::TESObjectREFR* obj) const;
    static bool IsTelekinesisObject(RE::TESObjectREFR* grabbed_ob);
};

class ActiveEffectVisitor final : public RE::MagicTarget::ForEachActiveEffectVisitor {
	RE::TESObjectREFR* grabbed_obj = nullptr;
public:
	RE::BSContainer::ForEachResult Accept(RE::ActiveEffect* a_effect) override;

    static ActiveEffectVisitor* GetSingleton() {
		static ActiveEffectVisitor singleton;
		return &singleton;
	}

	std::atomic<bool> is_using_telekinesis = false;

	void Reset(RE::TESObjectREFR* grabbed_object);
};