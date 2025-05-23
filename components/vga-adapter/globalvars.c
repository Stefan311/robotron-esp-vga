
#include "globalvars.h"

// Statische Werte vorinitialisiert
const struct SYSSTATIC _STATIC_SYS_VALS[] = {
	{ 
		.name = "A7100",
		.swap_colors = {0,1,2,3},
		.bits_per_sample = 8,
		.xres = 640,
		.yres = 400,
		.interleave_mask = 0,
		.default_pixel_abstand = 8950,
		.default_start_line = 32,
		.default_pixel_per_line = 73600,
		.default_vga_mode = 1,
		.accept_vga_modes = 0b0000000000001111,
	},
	{ 
		.name = "PC1715",
		.swap_colors = {2,1,0,3},
		.bits_per_sample = 4,
		.xres = 640,
		.yres = 299, /* 24 Zeilen (288 + Statuszeile) 299 im echten Leben*/
		.interleave_mask = 0,
		.default_pixel_abstand = 15532 /* schwankt bis auf 15588 */,
		.default_start_line = 24,
		.default_pixel_per_line = 86410,
		.default_vga_mode = 1,
		.accept_vga_modes = 0b0000000000001111,
	},
	{ 
		.name = "EC1834",
		.swap_colors = {0,1,2,3},
		.bits_per_sample = 8,
		.xres = 720,
		.yres = 350,
		.interleave_mask = 0,
		.default_pixel_abstand = 10717,
		.default_start_line = 18,
		.default_pixel_per_line = 86400,
		.default_vga_mode = 2,
		.accept_vga_modes = 0b0000000000001100,
	},
	{ 
		.name = "K7024",
		.swap_colors = {0,2,0,3},
		.bits_per_sample = 4,
		.xres = 640,
		.yres = 300,
		.interleave_mask = 0,
		.default_pixel_abstand = 15996,
		.default_start_line = 24,
		.default_pixel_per_line = 87200,
		.default_vga_mode = 1,
		.accept_vga_modes = 0b0000000000001111,
	},
	{ 
		.name = "VIDEO3",
		.swap_colors = {0,0,0,3},
		.bits_per_sample = 4,
		.xres = 640,
		.yres = 300,
		.interleave_mask = 0,
		.default_pixel_abstand = 20640,
		.default_start_line = 51,
		.default_pixel_per_line = 89580,
		.default_vga_mode = 1,
		.accept_vga_modes = 0b0000000000001111,
	},
	{
        .name = "VIS2A",
        .swap_colors = {2,1,0,3},
        .bits_per_sample = 4,
        .xres = 512,
        .yres = 256,
		.interleave_mask = 0,
        .default_pixel_abstand = 19186,
        .default_start_line = 35,
        .default_pixel_per_line = 80000,
		.default_vga_mode = 1,
		.accept_vga_modes = 0b0000000000001111,
    },    	
};

const struct COLORSTATIC _STATIC_COLOR_VALS[] = {
	{
		.name = {"Gr\x84n","green"},
		.colors = {0, 0b00000100, 0b00001000, 0b00001100}, // 0b--rrggbb
	},
	{
		.name = {"Weiss","white"},
		.colors = {0, 0b00010101, 0b00101010, 0b00111111}, // 0b--rrggbb
	},
	{
		.name = {"Orange","orange"},
		.colors = {0, 0b00010000, 0b00100100, 0b00111000}, // 0b--rrggbb
	}
};

const struct VGASTATIC _STATIC_VGA_VALS[] = {
	{
		.name="640x400x70",
		.hFront=16,
		.hSync=96,
		.hBack=48,
		.hRes=640,
		.vFront=12,
		.vSync=2,
		.vBack=35,
		.vRes=400,
		.frequency=25175000,
		.vPol=1,
		.hPol=1,
	},
	{
		.name="640x480x60",
		.hFront=16,
		.hSync=96,
		.hBack=48,
		.hRes=640,
		.vFront=10,
		.vSync=2,
		.vBack=33,
		.vRes=480,
		.frequency=25175000,
		.vPol=1,
		.hPol=1,
	},
	{
		.name="800x600x56",
		.hFront=24,
		.hSync=72,
		.hBack=128,
		.hRes=800,
		.vFront=1,
		.vSync=2,
		.vBack=22,
		.vRes=600,
		.frequency=36000000,
		.vPol=1,
		.hPol=1,
	},
	{
		.name="800x600x60",
		.hFront=40,
		.hSync=128,
		.hBack=88,
		.hRes=800,
		.vFront=1,
		.vSync=4,
		.vBack=23,
		.vRes=600,
		.frequency=40000000,
		.vPol=1,
		.hPol=1,
	},
};

// globale Variablen

// Aktives System
uint16_t ACTIVESYS = 0;
uint8_t ACTIVEVGA = 1;
nvs_handle_t sys_nvs_handle;
volatile uint32_t* ABG_DMALIST;
volatile uint32_t ABG_Scan_Line = 0;
volatile double ABG_PIXEL_PER_LINE;
volatile double BSYNC_PIXEL_ABSTAND;
volatile uint32_t ABG_START_LINE;
volatile bool ABG_RUN = false;
uint32_t ABG_Interleave_Mask = 0;
uint32_t ABG_Interleave = 0;
uint16_t ABG_XRes = 0;
uint16_t ABG_YRes = 0;
uint8_t ABG_Bits_per_sample = 0;

bool OSD_TRANSPARENT = false;
uint16_t OSD_TOP = 100;
uint16_t OSD_LEFT = 200;
const uint16_t OSD_HIGHT = 164;
const uint16_t OSD_WIDTH = 158;
uint8_t OSD_KEY_ROTATE = 0;

uint8_t* PIXEL_STEP_LIST;
uint8_t** VGA_BUF;
uint8_t** OSD_BUF;
volatile uint32_t bsyn_clock_diff = 0;
volatile uint32_t bsyn_clock_last = 0;
volatile uint32_t bsyn_clock_frame = 0;
volatile uint32_t BSYNC_SAMPLE_ABSTAND = 0;
uint8_t Current_Color_Scheme = 0;
uint8_t Current_Colors[4];
uint8_t Custom_Colors[4];

uint8_t this_app_id;
uint8_t next_app_id;

char* wlan_state = NULL;
char* wlan_ssid = NULL;
char* ap_ssid = NULL;
char* wlan_passwd = NULL;
uint8_t wlan_mode = 0;

uint8_t Language = 0;
