/**
 * @brief      NID lookup system
 */
#ifndef TAI_MODULE_HEADER
#define TAI_MODULE_HEADER

#include "taihen.h"

/**
 * @defgroup   module NID Lookup Interface
 */
/** @{ */

int module_get_by_name_nid(EmuEnvState &emuenv, SceUID pid, const char *name, uint32_t nid, tai_module_info_t *info);
int module_get_offset(EmuEnvState &emuenv, SceUID pid, SceUID modid, int segidx, SceSize offset, Address *addr);
int module_get_export_func(EmuEnvState &emuenv, SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, Address *func);
int module_get_import_func(EmuEnvState &emuenv, SceUID pid, const char *modname, uint32_t target_libnid, uint32_t funcnid, Address *stub);

/** @} */

#endif // TAI_MODULE_HEADER
