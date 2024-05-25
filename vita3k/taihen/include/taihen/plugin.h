/**
 * @brief      Config and plugin loading
 */
#ifndef TAI_PLUGIN_HEADER
#define TAI_PLUGIN_HEADER

/**
 * @defgroup   plugin Plugin Loading
 * @brief      Loads config.txt and handles plugins
 *
 * @details    Plugins are loaded from the default location and then the
 *             fallback. Locks are used to ensure config does not change during
 *             parsing.
 */
/** @{ */

#include "kernel/cpu_protocol.h"

#include <emuenv/state.h>

int plugin_init(void);
void plugin_deinit(void);

int plugin_load_config(EmuEnvState &emuenv);
int plugin_free_config(EmuEnvState &emuenv);
int plugin_load_all(EmuEnvState &emuenv, SceUID pid, const char *titleid);
int plugin_delayed_load_config(EmuEnvState &emuenv, int load_kernel);

/** @} */

#endif // TAI_PLUGIN_HEADER
