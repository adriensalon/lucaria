#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <imgui.h>

#include <GLFW/glfw3.h>

#include <Backend.hpp>
#include <Fonts.hpp>
#include <ImGuiContext.hpp>
#include <RunQueue.hpp>

#include <profiler/TracyAchievements.hpp>
#include <profiler/TracyConfig.hpp>
#include <profiler/TracyFileselector.hpp>
#include <profiler/TracyMouse.hpp>
#include <profiler/TracyTexture.hpp>
#include <profiler/TracyView.hpp>

#include <misc/freetype/imgui_freetype.h>

#include <profiler/TracyEmbed.hpp>

#include "data/FontBold.hpp"
#include "data/FontBoldItalic.hpp"
#include "data/FontFixed.hpp"
#include "data/FontIcons.hpp"
#include "data/FontItalic.hpp"
#include "data/FontNormal.hpp"

#include <common/TracyVersion.hpp>


extern ImTextureID zigzagTex;

namespace lucaria {
namespace detail {

    namespace {

        static std::shared_ptr<EmbedData> s_fontFixed;
        static std::shared_ptr<EmbedData> s_fontIcons;
        static std::shared_ptr<EmbedData> s_fontNormal;
        static std::shared_ptr<EmbedData> s_fontBold;
        static std::shared_ptr<EmbedData> s_fontBoldItalic;
        static std::shared_ptr<EmbedData> s_fontItalic;

        static void LoadTracyFontsPersistent(float scale)
        {
            ImGuiIO& io = ImGui::GetIO();

            io.Fonts->Clear();

            s_fontFixed = Unembed(FontFixed);
            s_fontIcons = Unembed(FontIcons);
            s_fontNormal = Unembed(FontNormal);
            s_fontBold = Unembed(FontBold);
            s_fontBoldItalic = Unembed(FontBoldItalic);
            s_fontItalic = Unembed(FontItalic);

            ImFontConfig configBasic;
            configBasic.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
            configBasic.FontDataOwnedByAtlas = false;

            ImFontConfig configMerge;
            configMerge.MergeMode = true;
            configMerge.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
            configMerge.FontDataOwnedByAtlas = false;

            ImFontConfig configFixed;
            configFixed.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
            configFixed.GlyphExtraAdvanceX = -1;
            configFixed.FontDataOwnedByAtlas = false;

            g_fonts.normal = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontNormal->data(),
                (int)s_fontNormal->size(),
                roundf(15.0f * scale),
                &configBasic);

            io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontIcons->data(),
                (int)s_fontIcons->size(),
                roundf(14.0f * scale),
                &configMerge);

            g_fonts.mono = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontFixed->data(),
                (int)s_fontFixed->size(),
                roundf(15.0f * scale),
                &configFixed);

            io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontIcons->data(),
                (int)s_fontIcons->size(),
                roundf(14.0f * scale),
                &configMerge);

            g_fonts.bold = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontBold->data(),
                (int)s_fontBold->size(),
                roundf(15.0f * scale),
                &configBasic);

            io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontIcons->data(),
                (int)s_fontIcons->size(),
                roundf(14.0f * scale),
                &configMerge);

            g_fonts.boldItalic = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontBoldItalic->data(),
                (int)s_fontBoldItalic->size(),
                roundf(15.0f * scale),
                &configBasic);

            io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontIcons->data(),
                (int)s_fontIcons->size(),
                roundf(14.0f * scale),
                &configMerge);

            g_fonts.italic = io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontItalic->data(),
                (int)s_fontItalic->size(),
                roundf(15.0f * scale),
                &configBasic);

            io.Fonts->AddFontFromMemoryTTF(
                (void*)s_fontIcons->data(),
                (int)s_fontIcons->size(),
                roundf(14.0f * scale),
                &configMerge);

            FontNormal = roundf(scale * 15.f);
            FontSmall = roundf(scale * 15.f * 2.f / 3.f);
            FontBig = roundf(scale * 15.f * 1.4f);
        }

        struct TracyProfilerState {
            char title[128] = {};
            Backend* backend = nullptr;

            RunQueue mainThreadTasks;

            std::unique_ptr<tracy::View> view;
            std::unique_ptr<ImGuiTracyContext> imguiContext;

            tracy::AchievementsMgr achievements;

            float dpiScale = 1.0f;
            bool running = true;
            bool readyToDraw = false;
        };

        static TracyProfilerState* g_state = nullptr;
        static bool g_insideDraw = false;

        static void RunOnMainThread(const std::function<void()>& cb, bool forceDelay = false)
        {
            if (g_state) {
                g_state->mainThreadTasks.Queue(cb, forceDelay);
            }
        }

        static void SetWindowTitleCallback(const char* title)
        {
            if (!g_state || !g_state->backend)
                return;

            char tmp[1024];
            std::snprintf(
                tmp,
                sizeof(tmp),
                "%s - Tracy Profiler %i.%i.%i",
                title,
                tracy::Version::Major,
                tracy::Version::Minor,
                tracy::Version::Patch);

            g_state->backend->SetTitle(tmp);
        }

        static void AttentionCallback()
        {
            if (g_state && g_state->backend) {
                g_state->backend->Attention();
            }
        }

        static void SetupScaleCallback(float scale)
        {
            if (!g_state)
                return;

            g_state->dpiScale = scale;
            tracy::s_config.userScale = scale;

            LoadTracyFontsPersistent(scale);
        }

        static void ScaleChangedCallback(float scale)
        {
            if (!g_state)
                return;

            g_state->dpiScale = scale;
            LoadTracyFontsPersistent(scale);
        }

        static int IsBusyCallback()
        {
            if (!g_state || !g_state->view)
                return 0;

            return g_state->view->IsBackgroundDone() ? 0 : 1;
        }

        static void DrawContents()
        {
            if (!g_state || !g_state->backend || !g_state->readyToDraw)
                return;

            if (g_insideDraw)
                return;

            struct DrawGuard {
                bool& flag;
                explicit DrawGuard(bool& flag)
                    : flag(flag)
                {
                    flag = true;
                }
                ~DrawGuard() { flag = false; }
            } guard(g_insideDraw);

            int displayW = 0;
            int displayH = 0;

            g_state->backend->NewFrame(displayW, displayH);

            ImGui::NewFrame();
            tracy::MouseFrame();

            if (g_state->view) {
                g_state->view->NotifyRootWindowSize(displayW, displayH);

                if (!g_state->view->Draw()) {
                    g_state->view.reset();
                }
            } else {
                ImGui::SetNextWindowSize(ImVec2(480, 160), ImGuiCond_FirstUseEver);

                ImGui::Begin("Lucaria Tracy Profiler");
                ImGui::TextUnformatted("No Tracy view is active.");
                ImGui::TextUnformatted("The profiler window can be closed.");
                ImGui::End();
            }

            g_state->backend->EndFrame();
        }

    }

    int run_tracy_profiler_window(const char* address, uint16_t port)
    {
        TracyProfilerState state;
        g_state = &state;

        std::snprintf(
            state.title,
            sizeof(state.title),
            "Tracy Profiler %i.%i.%i",
            tracy::Version::Major,
            tracy::Version::Minor,
            tracy::Version::Patch);

        tracy::LoadConfig();

        state.imguiContext = std::make_unique<ImGuiTracyContext>();

        Backend backend(
            state.title,
            DrawContents,
            ScaleChangedCallback,
            IsBusyCallback,
            &state.mainThreadTasks);

        state.backend = &backend;

        tracy::InitTexture();
        // zigzagTex = tracy::MakeTexture(true);
        zigzagTex = {};

        LoadTracyFontsPersistent(state.dpiScale);

        tracy::Fileselector::Init();

        state.view = std::make_unique<tracy::View>(
            RunOnMainThread,
            address,
            port,
            SetWindowTitleCallback,
            SetupScaleCallback,
            AttentionCallback,
            &state.achievements);

        state.readyToDraw = true;

        backend.Show();
        backend.Run();

        state.view.reset();

        tracy::Fileselector::Shutdown();

        g_state = nullptr;

        return 0;
    }

}
}