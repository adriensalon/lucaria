// window platform select
#if defined(LUCARIA_DISABLE_ANDROID)
constexpr int lucaria_has_android = 0;
#else
constexpr int lucaria_has_android = 1;
#endif
#if defined(LUCARIA_DISABLE_GLFW)
constexpr int lucaria_has_glfw = 0;
#else
constexpr int lucaria_has_glfw = 1;
#endif
#if defined(LUCARIA_DISABLE_PSP)
constexpr int lucaria_has_psp = 0;
#else
constexpr int lucaria_has_psp = 1;
#endif
// static_assert(lucaria_has_android + lucaria_has_glfw + lucaria_has_psp == 1, "Only one window platform can be selected")

// rendering backend select

