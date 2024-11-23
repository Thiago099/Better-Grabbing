#pragma once
#include "SKSEMenuFramework.h"
#include "Configuration.h"
namespace UI {
    void Register();
    namespace Example1 {
        inline RE::TESObjectSTAT* AddBoundObject = nullptr;
        void LookupForm();
        void __stdcall Render();
    }
};