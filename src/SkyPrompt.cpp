#include "SkyPrompt.h"

#include "InputManager.h"
#include "Manager.h"
#include "SkyPrompt/API.hpp"

namespace {
    constexpr SkyPromptAPI::ActionID resetObjectTransformAction = 1;

    class GrabControlsPrompt final : public SkyPromptAPI::PromptSink {
    public:
        static GrabControlsPrompt* GetSingleton() {
            static GrabControlsPrompt singleton;
            return &singleton;
        }

        void Show() {
            if (visible) {
                return;
            }

            if (prompts.empty()) {
                BuildPrompts();
            }
            if (prompts.empty()) {
                return;
            }

            if (clientID == 0) {
                clientID = SkyPromptAPI::RequestClientID();
            }
            if (clientID == 0) {
                return;
            }

            visible = SkyPromptAPI::SendPrompt(this, clientID);
            nativeResetHold = visible && hasResetPrompt;
        }

        void Hide() {
            if (!visible || clientID == 0) {
                return;
            }

            SkyPromptAPI::RemovePrompt(this, clientID);
            visible = false;
            nativeResetHold = false;
        }

        void ProcessEvent(const SkyPromptAPI::PromptEvent event) const override {
            if (event.type == SkyPromptAPI::PromptEventType::kAccepted &&
                event.prompt.actionID == resetObjectTransformAction) {
                Manager::GetSingleton()->ResetObjectTransform();
            }
        }

        std::span<const SkyPromptAPI::Prompt> GetPrompts() const override {
            return prompts;
        }

        bool UsesNativeResetHold() const {
            return nativeResetHold;
        }

    private:
        using Binding = std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>;

        struct Control {
            std::string_view action;
            std::string_view text;
            SkyPromptAPI::PromptType type;
            SkyPromptAPI::ActionID actionID;
        };

        void BuildPrompts() {
            constexpr std::array controls{
                Control{"Rotation", "Rotate", SkyPromptAPI::PromptType::kHint, 0},
                Control{"Translation", "Move", SkyPromptAPI::PromptType::kHint, 0},
                Control{"ZTranslation", "Adjust distance", SkyPromptAPI::PromptType::kHint, 0},
                Control{"ResetObjectTransform", "Reset position and rotation", SkyPromptAPI::PromptType::kHoldAndKeep,
                        resetObjectTransformAction},
            };

            bindings.reserve(controls.size());
            prompts.reserve(controls.size());

            const auto input = InputManager::GetSingleton();
            SkyPromptAPI::EventID eventID = 0;
            for (const auto& control : controls) {
                auto sources = input->GetSources(std::string(control.action));
                if (sources.empty()) {
                    continue;
                }

                bindings.emplace_back(sources.begin(), sources.end());
                prompts.emplace_back(
                    control.text,
                    eventID++,
                    control.actionID,
                    control.type,
                    0,  // Keep the hint in the screen corner instead of attaching it to the grabbed reference.
                    bindings.back());

                if (control.actionID == resetObjectTransformAction) {
                    hasResetPrompt = true;
                }
            }
        }

        SkyPromptAPI::ClientID clientID = 0;
        bool visible = false;
        bool hasResetPrompt = false;
        bool nativeResetHold = false;
        std::vector<std::vector<Binding>> bindings;
        std::vector<SkyPromptAPI::Prompt> prompts;
    };
}

void SkyPrompt::ShowControls() {
    GrabControlsPrompt::GetSingleton()->Show();
}

void SkyPrompt::HideControls() {
    GrabControlsPrompt::GetSingleton()->Hide();
}

bool SkyPrompt::UsesNativeResetHold() {
    return GrabControlsPrompt::GetSingleton()->UsesNativeResetHold();
}
