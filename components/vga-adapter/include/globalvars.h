#pragma once

#include <limits.h>
#include "nvs_flash.h"
#include "nvs.h"

// Statische Struktur - Systemkonstanten
struct SYSSTATIC {
	char* name;
	uint8_t swap_colors[4];
	uint8_t bits_per_sample; // 4 oder 8
	uint16_t xres; // 640 oder 720
	uint16_t yres;
	uint8_t interleave_mask; // 	0=1:1	1=1:2	3=1:4
	uint32_t default_pixel_abstand;
	uint32_t default_start_line;
	uint32_t default_pixel_per_line;
	uint8_t default_vga_mode;
	uint32_t accept_vga_modes;
};

// Statische Struktur - Farben
struct COLORSTATIC {
	char* shortname;
	char* longname;
	uint8_t colors[4];
};

// Statische Struktur - VGA-Modus
struct VGASTATIC {
	char* shortname;
	char* longname;
	uint16_t hFront;
	uint16_t hSync;
	uint16_t hBack;
	uint16_t hRes;
	uint16_t vFront;
	uint16_t vSync;
	uint16_t vBack;
	uint16_t vRes;
	uint32_t frequency;
	uint16_t vPol;
	uint16_t hPol;
};

// Statische Werte vorinitialisiert
extern const struct SYSSTATIC _STATIC_SYS_VALS[];
extern const struct COLORSTATIC _STATIC_COLOR_VALS[];
extern const struct VGASTATIC _STATIC_VGA_VALS[];

// Bezeichner für NVS KEY - max 15 Zeichen!
#define _NVS_SETTING_MODE	"SMODE"
#define _NVS_SETTING_PIXEL_ABSTAND "SPIXABST(%d)"
#define _NVS_SETTING_START_LINE	"SSTARTLINE(%d)"
#define _NVS_SETTING_PIXEL_PER_LINE "SPIXPERLINE(%d)"
#define _NVS_SETTING_WPS_MODE	"WPSMODE"
#define _NVS_SETTING_COLORSCHEMA	"COLORSCHEMA"
#define _NVS_SETTING_CUSTOMCOLORS	"CUSTOMCOLORS"
#define _NVS_SETTING_VGAMODE	"VGAMODE(%d)"

#define _SETTINGS_COUNT 6 // Anzahl unterstützter Computer (A7100,PC1715,EC1835,K7024,VIDEO3,VIS2A)
#define _COLORSCHEME_COUNT 3 // Anzahl unterstützter Farbschema (+custom)
#define _VGAMODE_COUNT 4 // Anzahl VGA-Modes (640x400x70, 640x480x60, 800x600x56, 800x600x60)

// globale Variablen

// Aktives System
extern uint16_t ACTIVESYS;
extern uint8_t ACTIVEVGA;

extern nvs_handle_t sys_nvs_handle;

extern volatile uint32_t* ABG_DMALIST;
extern volatile uint32_t ABG_Scan_Line;
extern volatile double ABG_PIXEL_PER_LINE;
extern volatile double BSYNC_PIXEL_ABSTAND;
extern volatile uint32_t ABG_START_LINE;
extern volatile bool ABG_RUN;
extern uint32_t ABG_Interleave_Mask;
extern uint32_t ABG_Interleave;
extern uint16_t ABG_XRes;
extern uint16_t ABG_YRes;
extern uint8_t ABG_Bits_per_sample;
extern uint8_t OSD_Height;

extern uint8_t* PIXEL_STEP_LIST;
extern uint8_t** VGA_BUF;
extern uint8_t** OSD_BUF;
extern volatile uint32_t bsyn_clock_diff;
extern volatile uint32_t bsyn_clock_last;
extern volatile uint32_t bsyn_clock_frame;
extern volatile uint32_t BSYNC_SAMPLE_ABSTAND;
extern int8_t Current_Color_Scheme;
extern uint8_t Current_Colors[4];
extern uint8_t Custom_Colors[4];
