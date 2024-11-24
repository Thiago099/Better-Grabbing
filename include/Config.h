#pragma once

class Config {
private:
    static inline Config* singleton = NULL;
public:
    static Config* GetSingleton() {
        if (!singleton) {
            singleton = new Config();
        }
        return singleton;
    }

    float MouseRotateXSensitivity = 0.005;
    float MouseRotateYSensitivity = 0.005;

    float MouseTranslateXSensitivity = 0.05;
    float MouseTranslateYSensitivity = 0.05;
    float MouseTranslateZSensitivity = 0.05;


    float GamepadRotateXSensitivity = 0.1;
    float GamepadRotateYSensitivity = 0.1;

    float GamepadTranslateXSensitivity = 1.0;
    float GamepadTranslateYSensitivity = 1.0;
    float GamepadTranslateZSensitivity = 1.0;

    float ButtonRotateXSensitivity = 10;
    float ButtonRotateYSensitivity = 10;
    float ButtonTranslateXSensitivity = 1;
    float ButtonTranslateYSensitivity = 1;
    float ButtonTranslateZSensitivity = 1;

    float TranslateZMaxDistance = 1000.0f;
    float TranslateZMinDistance = 60.0f;
    float TranslateZMinDefaultDistance = 100.f;

    float DragMovementDamping = 1.0f;
};