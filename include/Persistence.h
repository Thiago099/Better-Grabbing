#pragma once

#include "Ini.h"
#include "InputManager.h"

namespace Persistence {
	inline void LoadConfiguration() {
		auto ini = new Ini::IniReader("Better Grabbing.ini");
        auto input = InputManager::GetSingleton();
        auto config = Config::GetSingleton();
        ini->GetKeyPair("keyboard", [input](const char * button,const char*  evt) {
			input->AddSource(evt, "keyboard", button);
        });
        ini->GetKeyPair("mouse", [input](const char* button,const char* evt) {
			input->AddSource(evt, "mouse", button);
        });
        ini->GetKeyPair("gamepad", [input](const char* button,const char* evt) {
			input->AddSource(evt, "gamepad", button);
        });
        ini->SetSection("Properties");

        config->MouseRotateXSensitivity = ini->GetFloat("MouseRotateXSensitivity", config->MouseRotateXSensitivity);
        config->MouseRotateYSensitivity = ini->GetFloat("MouseRotateYSensitivity", config->MouseRotateYSensitivity);
        config->MouseTranslateXSensitivity = ini->GetFloat("MouseTranslateXSensitivity", config->MouseTranslateXSensitivity);
        config->MouseTranslateYSensitivity = ini->GetFloat("MouseTranslateYSensitivity", config->MouseTranslateYSensitivity);
        config->MouseTranslateZSensitivity = ini->GetFloat("MouseTranslateZSensitivity", config->MouseTranslateZSensitivity);
        config->GamepadRotateXSensitivity = ini->GetFloat("GamepadRotateXSensitivity", config->GamepadRotateXSensitivity);
        config->GamepadRotateYSensitivity = ini->GetFloat("GamepadRotateYSensitivity", config->GamepadRotateYSensitivity);
        config->GamepadTranslateXSensitivity = ini->GetFloat("GamepadTranslateXSensitivity", config->GamepadTranslateXSensitivity);
        config->GamepadTranslateYSensitivity = ini->GetFloat("GamepadTranslateYSensitivity", config->GamepadTranslateYSensitivity);
        config->GamepadTranslateZSensitivity = ini->GetFloat("GamepadTranslateZSensitivity", config->GamepadTranslateZSensitivity);
        config->ButtonRotateXSensitivity = ini->GetFloat("ButtonRotateXSensitivity", config->ButtonRotateXSensitivity);
        config->ButtonRotateYSensitivity = ini->GetFloat("ButtonRotateYSensitivity", config->ButtonRotateYSensitivity);
        config->ButtonTranslateXSensitivity = ini->GetFloat("ButtonTranslateXSensitivity", config->ButtonTranslateXSensitivity);
        config->ButtonTranslateYSensitivity = ini->GetFloat("ButtonTranslateYSensitivity", config->ButtonTranslateYSensitivity);
        config->ButtonTranslateZSensitivity = ini->GetFloat("ButtonTranslateZSensitivity", config->ButtonTranslateZSensitivity);
        config->TranslateZMaxDistance = ini->GetFloat("TranslateZMaxDistance", config->TranslateZMaxDistance);
        config->TranslateZMinDistance = ini->GetFloat("TranslateZMinDistance", config->TranslateZMinDistance);
        config->TranslateZMinDefaultDistance = ini->GetFloat("TranslateZMinDefaultDistance", config->TranslateZMinDefaultDistance);
        config->DragMovementDamping = ini->GetFloat("DragMovementDamping", config->DragMovementDamping);


	}

}