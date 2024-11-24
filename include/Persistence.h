#pragma once

#include "Ini.h"
#include "InputManager.h"

namespace Persistence {
	inline void LoadConfiguration() {
		auto ini = new Ini::IniReader("Better Grabbing.ini");
        auto input = InputManager::GetSingleton();
        ini->GetKeyPair("keyboard", [input](const char * button,const char*  evt) {
			input->AddSource(evt, "keyboard", button);
        });
        ini->GetKeyPair("mouse", [input](const char* button,const char* evt) {
			input->AddSource(evt, "mouse", button);
        });
        ini->GetKeyPair("gamepad", [input](const char* button,const char* evt) {
			input->AddSource(evt, "gamepad", button);
        });
	}

}