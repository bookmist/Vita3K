#include <nids/functions.h>

#define VAR_NID(name, nid) extern const char name_##name[] = #name;
#define NID(name, nid) extern const char name_##name[] = #name;
#include <nids/nids.h>
#undef NID
#undef VAR_NID

const char *import_name(uint32_t nid) {
    switch (nid) {
#define VAR_NID(name, nid) \
    case nid:              \
        return name_##name;
#define NID(name, nid) \
    case nid:          \
        return name_##name;
#include <nids/nids.h>
#undef NID
#undef VAR_NID
    case 0x20A01112:
        return "SceMp4_20A01112_GetNextAU_any";
    case 0x32A15788:
        return "SceMp4_32A15788_GetNextAU_3";
    case 0x40351E1A:
        return "SceMp4_40351E1A_stop2";
    case 0x40C64DC0:
        return "SceMp4_40C64DC0_GetNextAU_4";
    case 0x609E57AD:
        return "SceMp4_609E57AD_EnableStream";
    case 0x6B69F900:
        return "SceMp4_6B69F900";
    case 0x7B4832FE:
        return "SceMp4_7B4832FE_destroy_file_info";
    case 0x920623C8:
        return "SceMp4_920623C8_CloseFile";
    case 0x94E1305D:
        return "SceMp4_94E1305D";
    case 0x96676BA0:
        return "SceMp4_96676BA0";
    case 0xA7C09746:
        return "SceMp4_A7C09746";
    case 0xA870587F:
        return "SceMp4_A870587F";
    case 0xC05DFF01:
        return "SceMp4_C05DFF01_Stop_l";
    case 0xCB91CEBC:
        return "SceMp4_CB91CEBC";
    case 0xD092A066:
        return "SceMp4_D092A066";
    case 0xD2871F1C:
        return "SceMp4_D2871F1C";
    case 0xD5B26179:
        return "SceMp4_D5B26179_GetStreamInfo";
    case 0xEFE299C9:
        return "SceMp4_EFE299C9";
    default:
        return "UNRECOGNISED";
    }
}
