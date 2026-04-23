// Vita3K emulator project
// Copyright (C) 2026 Vita3K team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#pragma once

#include <array>
#include <util/system.h>
#include <util/warning.h>

enum ModulesMode {
    AUTOMATIC,
    AUTO_MANUAL,
    MANUAL
};

enum PerformanceOverlayDetail {
    MINIMUM,
    LOW,
    MEDIUM,
    MAXIMUM,
};

enum PerformanceOverlayPosition {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
};

enum ScreenshotFormat {
    None,
    JPEG,
    PNG,
};

template <size_t N>
consteval std::array<const char, N> make_option_name(const char (&input)[N]) {
    std::array<char, N> result{};

    for (size_t i = 0; i < N; ++i) {
        result[i] = (input[i] == '_') ? '-' : input[i];
    }

    return std::array<const char, N>{ const_cast<const char &>(result[0]) };
}

__pragma(warning(disable : 4002))

#define CODE_NOTHING(option_type, option_name, option_default, member_name, ...)

#define CONFIG_VITA_IF_HELPER(...) CONFIG_VITA_IF_SELECT(__VA_ARGS__)
#define CONFIG_VITA_IF_SELECT(condition, ...) CONFIG_VITA_IF_##condition

#define CONFIG_VITA_IF_1(true_case, false_case) true_case
#define CONFIG_VITA_IF_0(true_case, false_case) false_case

// if code_custom is empty then use code_all else use code_custom or code_no_custom based on is_custom
#define CONFIG_VITA_IF(is_custom, code_custom, code_no_custom) \
    CONFIG_VITA_IF_HELPER(is_custom)(code_custom, code_no_custom)

// New CODE macro: allow the last parameter to be optional (node name) and treat it as a token.
// When the optional parameter is present, invoke `code_custom` with an extra argument
// (the node name). When absent, invoke `code_no_custom`.

//  Detect whether an extra variadic argument was provided. Expands to 1 if provided, 0 otherwise.
#define PP_HAS_ARG_IMPL(_0, _1, N, ...) N
#define PP_HAS_ARG(...) PP_HAS_ARG_IMPL(, ##__VA_ARGS__, 1, 0)

#define CODE(code_custom, code_no_custom, type, name, def_value, ...) \
    CONFIG_VITA_IF(PP_HAS_ARG(__VA_ARGS__), code_custom, code_no_custom)(type, (&(make_option_name(#name)[0])), def_value, name, ##__VA_ARGS__)

// clang-format off
// Singular options produced in config file
// Order is CODE(code_custom, code_no_custom, option_type, option_default, member_name, option_can_be_set_for_individual_games(0/1))
// calls are code(option_type, "member-name", option_default, member_name)
// When adding in a new macro for generation, ALL options must be stated.
// All member names starting with "keyboard_" will be considered as key input (See controls_dialog.cpp)
#define CONFIG_INDIVIDUAL_EXT(code_custom, code_no_custom) \
    CODE(code_custom, code_no_custom, bool,            initial_setup,                          false, ) \
    CODE(code_custom, code_no_custom, bool,            gdbstub,                                false, ) \
    CODE(code_custom, code_no_custom, bool,            log_active_shaders,                     false, ) \
    CODE(code_custom, code_no_custom, bool,            log_uniforms,                           false, ) \
    CODE(code_custom, code_no_custom, bool,            log_compat_warn,                        false, ) \
    CODE(code_custom, code_no_custom, bool,            validation_layer,                       true,  ) \
    CODE(code_custom, code_no_custom, bool,            pstv_mode,                              false, system) \
    CODE(code_custom, code_no_custom, bool,            show_mode,                              false, ) \
    CODE(code_custom, code_no_custom, bool,            demo_mode,                              false, ) \
    CODE(code_custom, code_no_custom, bool,            show_gui,                               false, ) \
    CODE(code_custom, code_no_custom, bool,            show_info_bar,                          false, ) \
    CODE(code_custom, code_no_custom, bool,            apps_list_grid,                         false, ) \
    CODE(code_custom, code_no_custom, bool,            display_system_apps,                    true,  ) \
    CODE(code_custom, code_no_custom, bool,            stretch_the_display_area,               false, gpu) \
    CODE(code_custom, code_no_custom, bool,            fullscreen_hd_res_pixel_perfect,        false, gpu) \
    CODE(code_custom, code_no_custom, bool,            show_live_area_screen,                  true,  ) \
    CODE(code_custom, code_no_custom, int,             icon_size,                              64,    ) \
    CODE(code_custom, code_no_custom, bool,            archive_log,                            false, ) \
    CODE(code_custom, code_no_custom, std::string,     backend_renderer,                       "Vulkan", gpu) \
    CODE(code_custom, code_no_custom, std::string,     custom_driver_name,                     "",     gpu) \
    CODE(code_custom, code_no_custom, bool,            turbo_mode,                             false, ) \
    CODE(code_custom, code_no_custom, int,             gpu_idx,                                0,      gpu) \
    CODE(code_custom, code_no_custom, bool,            high_accuracy,                          false, gpu) \
    CODE(code_custom, code_no_custom, float,           resolution_multiplier,                  1.0f,   gpu) \
    CODE(code_custom, code_no_custom, bool,            disable_surface_sync,                   true,   gpu) \
    CODE(code_custom, code_no_custom, std::string,     screen_filter,                          "Bilinear", gpu) \
    CODE(code_custom, code_no_custom, bool,            v_sync,                                 true,   gpu) \
    CODE(code_custom, code_no_custom, int,             anisotropic_filtering,                   1,      gpu) \
    CODE(code_custom, code_no_custom, bool,            texture_cache,                          true,   ) \
    CODE(code_custom, code_no_custom, bool,            async_pipeline_compilation,             true,   gpu) \
    CODE(code_custom, code_no_custom, bool,            show_compile_shaders,                   true,   ) \
    CODE(code_custom, code_no_custom, bool,            hashless_texture_cache,                 false,  ) \
    CODE(code_custom, code_no_custom, bool,            import_textures,                        false,  gpu) \
    CODE(code_custom, code_no_custom, bool,            export_textures,                        false,  gpu) \
    CODE(code_custom, code_no_custom, bool,            export_as_png,                          true,   gpu) \
    CODE(code_custom, code_no_custom, std::string,     memory_mapping,                         "double-buffer", gpu) \
    CODE(code_custom, code_no_custom, bool,            boot_apps_full_screen,                  false,  ) \
    CODE(code_custom, code_no_custom, std::string,     audio_backend,                          "SDL", audio) \
    CODE(code_custom, code_no_custom, int,             audio_volume,                           100,    audio) \
    CODE(code_custom, code_no_custom, bool,            ngs_enable,                             true,   audio) \
    CODE(code_custom, code_no_custom, int,             bgm_volume,                             65,     ) \
    CODE(code_custom, code_no_custom, int,             sys_button,                             static_cast<int>(SCE_SYSTEM_PARAM_ENTER_BUTTON_CROSS), ) \
    CODE(code_custom, code_no_custom, int,             sys_lang,                               static_cast<int>(SCE_SYSTEM_PARAM_LANG_ENGLISH_US), ) \
    CODE(code_custom, code_no_custom, int,             sys_date_format,                        (int)SCE_SYSTEM_PARAM_DATE_FORMAT_MMDDYYYY, ) \
    CODE(code_custom, code_no_custom, int,             sys_time_format,                        (int)SCE_SYSTEM_PARAM_TIME_FORMAT_12HOUR, ) \
    CODE(code_custom, code_no_custom, int,             cpu_pool_size,                          10,     ) \
    CODE(code_custom, code_no_custom, int,             modules_mode,                           static_cast<int>(ModulesMode::AUTOMATIC), core) \
    CODE(code_custom, code_no_custom, int,             delay_background,                       4,      ) \
    CODE(code_custom, code_no_custom, int,             delay_start,                            30,     ) \
    CODE(code_custom, code_no_custom, float,           background_alpha,                       .300f,  ) \
    CODE(code_custom, code_no_custom, int,             log_level,                              0 /*SPDLOG_LEVEL_TRACE*/, ) \
    CODE(code_custom, code_no_custom, bool,            cpu_opt,                                true,   cpu) \
    CODE(code_custom, code_no_custom, std::string,     pref_path,                              std::string{}, ) \
    CODE(code_custom, code_no_custom, bool,            discord_rich_presence,                  true,   ) \
    CODE(code_custom, code_no_custom, bool,            wait_for_debugger,                      false,  ) \
    CODE(code_custom, code_no_custom, bool,            color_surface_debug,                    false,  ) \
    CODE(code_custom, code_no_custom, bool,            show_touchpad_cursor,                   true,   emulator) \
    CODE(code_custom, code_no_custom, bool,            performance_overlay,                    false,  ) \
    CODE(code_custom, code_no_custom, int,             performance_overlay_detail,             static_cast<int>(MINIMUM), ) \
    CODE(code_custom, code_no_custom, int,             performance_overlay_position,           static_cast<int>(TOP_LEFT), ) \
    CODE(code_custom, code_no_custom, bool,            enable_gamepad_overlay,                 true,   ) \
    CODE(code_custom, code_no_custom, bool,            overlay_show_touch_switch,              false,  ) \
    CODE(code_custom, code_no_custom, float,           overlay_scale,                          1.0f,   ) \
    CODE(code_custom, code_no_custom, int,             overlay_opacity,                        100,    ) \
    CODE(code_custom, code_no_custom, int,             screenshot_format,                      static_cast<int>(JPEG), ) \
    CODE(code_custom, code_no_custom, bool,            disable_motion,                         false,  ) \
    CODE(code_custom, code_no_custom, float,           controller_analog_multiplier,           1.0f,   ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_select,                  229,   ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_start,                   40,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_up,                      82,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_right,                   79,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_down,                    81,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_left,                    80,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_l1,                      20,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_r1,                      8,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_l2,                      24,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_r2,                      18,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_l3,                      9,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_r3,                      11,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_triangle,                25,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_circle,                  6,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_cross,                   27,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_square,                  29,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_left,                 4,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_right,                7,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_up,                   26,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_down,                 22,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_left,                13,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_right,               15,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_up,                  12,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_down,                14,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_psbutton,                19,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_gui_toggle_gui,                 10,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_gui_fullscreen,                 68,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_gui_toggle_touch,               23,    ) \
    CODE(code_custom, code_no_custom, int,             keyboard_toggle_texture_replacement,     0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_take_screenshot,                0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_pinch_modifier,                 0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_alternate_pinch_in,             0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_alternate_pinch_out,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_select_alt,              0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_start_alt,               0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_up_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_right_alt,               0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_down_alt,                0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_left_alt,                0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_l1_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_r1_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_l2_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_r2_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_l3_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_r3_alt,                  0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_triangle_alt,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_circle_alt,              0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_cross_alt,               0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_square_alt,              0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_left_alt,             0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_right_alt,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_up_alt,               0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_leftstick_down_alt,             0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_left_alt,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_right_alt,           0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_up_alt,              0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_rightstick_down_alt,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_button_psbutton_alt,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_gui_toggle_gui_alt,             0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_gui_fullscreen_alt,             0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_gui_toggle_touch_alt,           0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_toggle_texture_replacement_alt, 0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_take_screenshot_alt,            0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_pinch_modifier_alt,             0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_alternate_pinch_in_alt,         0,     ) \
    CODE(code_custom, code_no_custom, int,             keyboard_alternate_pinch_out_alt,        0,     ) \
    CODE(code_custom, code_no_custom, std::string,     user_id,                                 std::string{}, ) \
    CODE(code_custom, code_no_custom, bool,            auto_user_login,                         false, ) \
    CODE(code_custom, code_no_custom, std::string,     user_lang,                               std::string{}, ) \
    CODE(code_custom, code_no_custom, bool,            display_info_message,                    false, ) \
    CODE(code_custom, code_no_custom, bool,            show_welcome,                            true,  ) \
    CODE(code_custom, code_no_custom, bool,            check_for_updates,                       true,  ) \
    CODE(code_custom, code_no_custom, int,             file_loading_delay,                      0,     emulator) \
    CODE(code_custom, code_no_custom, bool,            asia_font_support,                       false, ) \
    CODE(code_custom, code_no_custom, bool,            shader_cache,                            true,  ) \
    CODE(code_custom, code_no_custom, bool,            spirv_shader,                            false, ) \
    CODE(code_custom, code_no_custom, bool,            fps_hack,                                false, gpu) \
    CODE(code_custom, code_no_custom, uint64_t,        current_ime_lang,                        4,     ) \
    CODE(code_custom, code_no_custom, bool,            psn_signed_in,                           false, network) \
    CODE(code_custom, code_no_custom, bool,            http_enable,                             true,  ) \
    CODE(code_custom, code_no_custom, int,             http_timeout_attempts,                   50,    ) \
    CODE(code_custom, code_no_custom, int,             http_timeout_sleep_ms,                   100,   ) \
    CODE(code_custom, code_no_custom, int,             http_read_end_attempts,                  10,    ) \
    CODE(code_custom, code_no_custom, int,             http_read_end_sleep_ms,                  250,   ) \
    CODE(code_custom, code_no_custom, int,             adhoc_addr,                              0,     ) \
    CODE(code_custom, code_no_custom, int,             front_camera_type,                       2,     ) \
    CODE(code_custom, code_no_custom, std::string,     front_camera_id,                         std::string{}, ) \
    CODE(code_custom, code_no_custom, std::string,     front_camera_image,                      std::string{}, ) \
    CODE(code_custom, code_no_custom, uint32_t,        front_camera_color,                      0,     ) \
    CODE(code_custom, code_no_custom, int,             back_camera_type,                        2,     ) \
    CODE(code_custom, code_no_custom, std::string,     back_camera_id,                          std::string{}, ) \
    CODE(code_custom, code_no_custom, std::string,     back_camera_image,                       std::string{}, ) \
    CODE(code_custom, code_no_custom, uint32_t,        back_camera_color,                       0,     ) \
    CODE(code_custom, code_no_custom, bool,            tracy_primitive_impl,                    false, )

// Vector members produced in the config file
// Order is code(option_type, option_name, default_value)
// If you are going to implement a dynamic list in the YAML, add it here instead
// When adding in a new macro for generation, ALL options must be stated.
#define CONFIG_VECTOR_EXT(code_custom, code_no_custom)                                                                             \
    CODE(code_custom, code_no_custom, std::vector<short>, controller_binds, std::vector<short>{},)                \
    CODE(code_custom, code_no_custom, std::vector<int>, controller_led_color, std::vector<int>{},)            \
    CODE(code_custom, code_no_custom, std::vector<std::string>, lle_modules, std::vector<std::string>{},core)              \
    CODE(code_custom, code_no_custom, std::vector<uint64_t>, ime_langs, std::vector<uint64_t>{4},)                       \
    CODE(code_custom, code_no_custom, std::vector<std::string>, tracy_advanced_profiling_modules, std::vector<std::string>{},)

DISABLE_WARNING_END
// clang-format on
#define CONFIG_INDIVIDUAL(code) \
    CONFIG_INDIVIDUAL_EXT(code, code)

#define CONFIG_INDIVIDUAL_CUSTOM(code) \
    CONFIG_INDIVIDUAL_EXT(code, CODE_NOTHING)

#define CONFIG_VECTOR(code) \
    CONFIG_VECTOR_EXT(code, code)

#define CONFIG_VECTOR_CUSTOM(code) \
    CONFIG_VECTOR_EXT(code, CODE_NOTHING)

// clang-format off
// Parent macro for easier generation
#define CONFIG_LIST(code)                                                                               \
    DISABLE_WARNING_BEGIN(4002,"")           \
    CONFIG_INDIVIDUAL(code)                                                                             \
    CONFIG_VECTOR(code)    \
    DISABLE_WARNING_END

#define CONFIG_LIST_CUSTOM(code)                                                                               \
    CONFIG_INDIVIDUAL_CUSTOM(code)                                                                             \
    CONFIG_VECTOR_CUSTOM(code)
    // clang-format on
