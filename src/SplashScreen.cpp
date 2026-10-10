#include "SplashScreen.h"

namespace WitcherNotes::Splash {

Art PickArt() {
    return {"", "Notes"};
}

void Render(GLFWwindow*, ImVec2, const std::string&, float,
            GLuint, GLuint, const Fonts&,
            const std::string&, const WitcherNotes::Settings::ThemeColors&) {}

void RunStep(const Step& step, int, int, GLFWwindow*, ImVec2,
             GLuint, GLuint, const Fonts&,
             const std::string&, const WitcherNotes::Settings::ThemeColors&) {
    if (step.task) step.task();
}

void WaitForUserConfirmation(GLFWwindow*, ImVec2,
                             GLuint, GLuint, const Fonts&,
                             const std::string&, const WitcherNotes::Settings::ThemeColors&) {}

} // namespace WitcherNotes::Splash
