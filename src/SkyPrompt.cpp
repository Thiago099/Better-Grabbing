#include "SkyPrompt.h"

#include "InputManager.h"
#include "SkyPrompt/API.hpp"

namespace {
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

            static_cast<void>(SkyPromptAPI::SendPrompt(this, clientID));
            visible = true;
        }

        void Hide() {
            if (!visible || clientID == 0) {
                return;
            }

            SkyPromptAPI::RemovePrompt(this, clientID);
            visible = false;
        }

        void ProcessEvent(SkyPromptAPI::PromptEvent) const override {}

        std::span<const SkyPromptAPI::Prompt> GetPrompts() const override {
            return prompts;
        }

    private:
        using Binding = std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>;

        struct Control {
            std::string_view action;
            std::string_view text;
        };

        void BuildPrompts() {
            constexpr std::array controls{
                Control{"Rotation", "Rotate"},
                Control{"Translation", "Move"},
                Control{"ZTranslation", "Adjust distance"},
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
                    0,
                    SkyPromptAPI::PromptType::kHint,
                    0,  // Keep the hint in the screen corner instead of attaching it to the grabbed reference.
                    bindings.back());
            }
        }

        SkyPromptAPI::ClientID clientID = 0;
        bool visible = false;
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
