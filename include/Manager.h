#pragma once
#include "Raycast.h"
#include "InputManager.h"
#include "Config.h"

#define M_PI 3.14159265358979323846f


class Manager {
    bool isGrabbing = false;
    float fistPersonDistance = 100.f;
    float thirdPersonDistance = 500.f;
    RE::NiPoint2 angle = {0, 0};
    RE::NiPoint2 position = {0, 0};
    RE::COL_LAYER oldCollisionLayer;
    inline static Manager* singleton = NULL;

    static inline bool doRotate = false;
    static inline bool doTranslate = false;
    static inline bool doTranslateZ = false;

    void UpdateObjectTransform(RE::TESObjectREFR* obj, RE::NiPoint3& rayPosition);
    float NormalizeAngle(float angle);

public:

    bool GetDoRotate() {
        return doRotate;
    }

    bool GetDoTranslate() {
        return doTranslate ;
    }

    bool GetTranslateZ() {
        return doTranslateZ;
    }

    Manager() { 
        auto input = InputManager::GetSingleton();
        input->AddSink("Rotation", [](RE::ButtonEvent* button) { 
            if (button->IsDown()) {
                doRotate = true;
            } else if (button->IsUp()) {
                doRotate = false;
            }
        });

        input->AddSink("Translation", [](RE::ButtonEvent* button) {
            if (button->IsDown()) {
                doTranslate = true;
            } else if (button->IsUp()) {
                doTranslate = false;
            }
        });

        input->AddSink("ZTranslation", [](RE::ButtonEvent* button) {
            if (button->IsDown()) {
                doTranslateZ = true;
            } else if (button->IsUp()) {
                doTranslateZ = false;
            }
        });

        input->AddSink("MoveObjectCloser", [](RE::ButtonEvent* button) {
            Manager::GetSingleton()->TranslateZ(3.f);
        });

        input->AddSink("MoveObjectFruther", [](RE::ButtonEvent* button) {
            Manager::GetSingleton()->TranslateZ(-3.f);
        });

        input->AddSink("RotateXPlus",
        [this](RE::ButtonEvent* button) { 
            auto config = Config::GetSingleton();
            RotateX(config->ButtonRotateXSensitivity / 360 * M_PI);
        });

        input->AddSink("RotateXMinus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            RotateX(-config->ButtonRotateXSensitivity / 360 * M_PI);
        });

        input->AddSink("RotateYPlus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            RotateY(-config->ButtonRotateYSensitivity / 360 * M_PI);
        });

        input->AddSink("RotateYMinus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            RotateY(config->ButtonRotateYSensitivity / 360 * M_PI);
        });

        input->AddSink("TranslateXPlus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            TranslateX(config->ButtonTranslateXSensitivity);
        });

        input->AddSink("TranslateXMinus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            TranslateX(-config->ButtonTranslateXSensitivity);
        });

        input->AddSink("TranslateYPlus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            TranslateY(config->ButtonTranslateYSensitivity);
        });

        input->AddSink("TranslateYMinus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            TranslateY(-config->ButtonTranslateYSensitivity);
        });

        input->AddSink("TranslateZPlus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            TranslateZ(config->ButtonTranslateZSensitivity);
        });

        input->AddSink("TranslateZMinus", [this](RE::ButtonEvent* button) {
            auto config = Config::GetSingleton();
            TranslateZ(-config->ButtonTranslateZSensitivity);
        });
    }
    static Manager* GetSingleton() {
        if (!singleton) {
            singleton = new Manager();
        }
        return singleton;
    }
    void SetGrabbing(bool value, RE::TESObjectREFRPtr ref);
    void RotateX(float x) {
        if (glm::abs(angle.y) > glm::half_pi<float>()) {
            angle.x = NormalizeAngle(angle.x - x);
        } else {
            angle.x = NormalizeAngle(angle.x + x);
        }
    }
    void RotateY(float y) {
        angle.y = NormalizeAngle(angle.y + y);
    }
    void TranslateX(float x) {
        position.x += x;
    }
    void TranslateY(float y) {
        position.y += y;
    }
    void TranslateZ(float value) {
        RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();

        if (camera->currentState.get()->id == RE::CameraState::kThirdPerson) {

            auto candidate = thirdPersonDistance + value;
            auto config = Config::GetSingleton();

            if (candidate < config->TranslateZMinDistance) {
                candidate = config->TranslateZMinDistance;
            }

            if (candidate > config->TranslateZMaxDistance) {
                candidate = config->TranslateZMaxDistance;
            }

            thirdPersonDistance = candidate;

        } else {
            
            auto candidate = fistPersonDistance + value;
            auto config = Config::GetSingleton();

            if (candidate < config->TranslateZMinDistance) {
                candidate = config->TranslateZMinDistance;
            }

            if (candidate > config->TranslateZMaxDistance) {
                candidate = config->TranslateZMaxDistance;
            }

            fistPersonDistance = candidate;
        }

    }
    bool GetIsGrabbing() {
        return isGrabbing;
    }
    void UpdatePosition(RE::TESObjectREFR* obj);
};