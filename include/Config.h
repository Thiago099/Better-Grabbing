#pragma once

class Config {
private:
    static inline Config* singleton = nullptr;
public:
    static Config* GetSingleton() {
        if (!singleton) {
            singleton = new Config();
        }
        return singleton;
    }

    float MouseRotateXSensitivity = 0.005f;
    float MouseRotateYSensitivity = 0.005f;

    float MouseTranslateXSensitivity = 0.05f;
    float MouseTranslateYSensitivity = 0.05f;
    float MouseTranslateZSensitivity = 0.05f;


    float GamepadRotateXSensitivity = 0.1f;
    float GamepadRotateYSensitivity = 0.1f;

    float GamepadTranslateXSensitivity = 1.f;
    float GamepadTranslateYSensitivity = 1.f;
    float GamepadTranslateZSensitivity = 1.f;

    float ButtonRotateXSensitivity = 10.f;
    float ButtonRotateYSensitivity = 10.f;
    float ButtonTranslateXSensitivity = 10.f;
    float ButtonTranslateYSensitivity = 10.f;
    float ButtonTranslateZSensitivity = 10.f;

    float TranslateZMaxDistance = 1000.0f;
    float TranslateZMinDistance = 60.0f;
    float TranslateZMinDefaultDistance = 150.f;

    float DragMovementDamping = 1.0f;
    bool DisableCollisionWithItemsWhileGrabbing = true;
};