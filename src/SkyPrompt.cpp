#include "SkyPrompt.h"

#include "InputManager.h"
#include "Manager.h"
#include "SkyPrompt/API.hpp"

#include <deque>

namespace {
    constexpr std::string_view betterGrabbingTheme = "BetterGrabbing";
    constexpr SkyPromptAPI::ActionID resetObjectTransformAction = 1;

    using Binding = std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>;

    class PromptEntrySink final : public SkyPromptAPI::PromptSink {
    public:
        PromptEntrySink(
            const std::string_view aText,
            const SkyPromptAPI::EventID aEventID,
            const SkyPromptAPI::ActionID aActionID,
            const SkyPromptAPI::PromptType aType,
            std::vector<Binding> aBindings)
            : text(aText),
              bindings(std::move(aBindings)),
              prompt(text, aEventID, aActionID, aType, 0, bindings),
              actionID(aActionID) {}

        PromptEntrySink(const PromptEntrySink&) = delete;
        PromptEntrySink& operator=(const PromptEntrySink&) = delete;
        PromptEntrySink(PromptEntrySink&&) = delete;
        PromptEntrySink& operator=(PromptEntrySink&&) = delete;

        bool Show(const SkyPromptAPI::ClientID aClientID) {
            clientID = aClientID;
            if (!queued) {
                queued = SkyPromptAPI::SendPrompt(this, clientID);
            }
            return queued;
        }

        void Hide() {
            if (queued && clientID != 0) {
                SkyPromptAPI::RemovePrompt(this, clientID);
            }
            queued = false;
        }

        void ProcessEvent(const SkyPromptAPI::PromptEvent event) const override {
            if (event.type == SkyPromptAPI::PromptEventType::kTimeout) {
                queued = false;
                if (clientID != 0 && Manager::GetSingleton()->GetIsGrabbing()) {
                    SkyPromptAPI::RemovePrompt(this, clientID);
                    queued = SkyPromptAPI::SendPrompt(this, clientID);
                }
                return;
            }

            if (event.type == SkyPromptAPI::PromptEventType::kAccepted &&
                actionID == resetObjectTransformAction) {
                Manager::GetSingleton()->ResetObjectTransform();
            }
        }

        std::span<const SkyPromptAPI::Prompt> GetPrompts() const override {
            return std::span<const SkyPromptAPI::Prompt>(&prompt, 1);
        }

        bool IsResetQueued() const {
            return queued && actionID == resetObjectTransformAction;
        }

    private:
        std::string text;
        std::vector<Binding> bindings;
        SkyPromptAPI::Prompt prompt;
        SkyPromptAPI::ActionID actionID;
        SkyPromptAPI::ClientID clientID = 0;
        mutable bool queued = false;
    };

    class GrabControlsPrompt final {
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
                if (clientID != 0 && !SkyPromptAPI::RequestTheme(clientID, betterGrabbingTheme)) {
                    logger::error("SkyPrompt could not load the Better Grabbing theme.");
                } else if (clientID != 0) {
                    logger::info("SkyPrompt loaded the Better Grabbing theme.");
                }
            }
            if (clientID == 0) {
                return;
            }

            std::size_t queuedCount = 0;
            nativeResetHold = false;
            for (auto& entry : prompts) {
                if (entry.Show(clientID)) {
                    ++queuedCount;
                    nativeResetHold |= entry.IsResetQueued();
                }
            }

            visible = queuedCount != 0;
            if (queuedCount != prompts.size()) {
                logger::warn("SkyPrompt queued {}/{} Better Grabbing prompts.", queuedCount, prompts.size());
            }
        }

        void Hide() {
            if (!visible || clientID == 0) {
                return;
            }

            for (auto& entry : prompts) {
                entry.Hide();
            }
            visible = false;
            nativeResetHold = false;
        }

        bool UsesNativeResetHold() const {
            return nativeResetHold;
        }

    private:
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

            const auto input = InputManager::GetSingleton();
            SkyPromptAPI::EventID eventID = 0;

            for (const auto& control : controls) {
                auto sources = input->GetSources(std::string(control.action));
                if (sources.empty()) {
                    continue;
                }

                prompts.emplace_back(
                    control.text,
                    eventID++,
                    control.actionID,
                    control.type,
                    std::vector<Binding>(sources.begin(), sources.end()));
            }

            const auto controlMap = RE::ControlMap::GetSingleton();
            const auto userEvents = RE::UserEvents::GetSingleton();
            if (!controlMap || !userEvents) {
                return;
            }

            std::vector<Binding> interactionBindings;
            for (const auto device : {RE::INPUT_DEVICE::kKeyboard, RE::INPUT_DEVICE::kMouse,
                     RE::INPUT_DEVICE::kGamepad}) {
                const auto interactionKey = controlMap->GetMappedKey(userEvents->activate, device);
                if (interactionKey != RE::ControlMap::kInvalid) {
                    interactionBindings.emplace_back(device, interactionKey);
                }
            }

            if (!interactionBindings.empty()) {
                prompts.emplace_back(
                    "Stop grabbing",
                    eventID++,
                    0,
                    SkyPromptAPI::PromptType::kHint,
                    std::move(interactionBindings));
            }
        }

        SkyPromptAPI::ClientID clientID = 0;
        bool visible = false;
        bool nativeResetHold = false;
        std::deque<PromptEntrySink> prompts;
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
