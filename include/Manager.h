#pragma once
#include <algorithm>

#include "Raycast.h"
#include "InputManager.h"
#include "Config.h"
#include "SkyPrompt.h"

#define M_PI 3.14159265358979323846f


class Manager {
    bool isGrabbing = false;
    float fistPersonDistance = 100.f;
    float thirdPersonDistance = 100.f;
    RE::NiPoint2 rotationDelta = {0, 0};
    RE::NiMatrix3 initialOrientation;
    RE::NiMatrix3 currentOrientation;
    float initialCameraYaw = 0.0f;
    float appliedHorizontalAngle = 0.0f;
    RE::NiPoint2 position = {0, 0};
    bool resetHoldTriggered = false;
    RE::COL_LAYER oldCollisionLayer;
    inline static Manager* singleton = nullptr;

    static inline bool doRotate = false;
    static inline bool doTranslate = false;
    static inline bool doTranslateZ = false;

    void UpdateObjectTransform(RE::TESObjectREFR* obj, RayOutput& ray);
    void HandleResetObjectTransform(RE::ButtonEvent* button);
    static float NormalizeAngle(float angle);

    std::atomic<bool> isTryingToThrow = false;

public:

    static bool GetIsTryingToThrow() {
        return is_grab_n_throw_installed && Manager::GetSingleton()->isTryingToThrow.load();
	}

    static void SetIsTryingToThrow(bool value) {
		if (is_grab_n_throw_installed) {
            Manager::GetSingleton()->isTryingToThrow.store(value);
		}
	}

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

        input->AddSinkWithResult("ResetObjectTransform", [this](RE::ButtonEvent* button) {
            HandleResetObjectTransform(button);
            return !SkyPrompt::UsesNativeResetHold();
        });
    }
    static Manager* GetSingleton() {
        if (!singleton) {
            singleton = new Manager();
        }
        return singleton;
    }
    void SetGrabbing(bool value, const RE::TESObjectREFRPtr& ref);
    void ResetObjectTransform();
    void RotateX(const float x) {
        rotationDelta.x = NormalizeAngle(rotationDelta.x + x);
    }
    void RotateY(const float y) {
        rotationDelta.y = NormalizeAngle(rotationDelta.y + y);
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
    void UpdatePosition(RE::TESObjectREFR* obj);
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
