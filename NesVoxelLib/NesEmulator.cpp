#include "libretro.h"
#include "stdafx.h"

#include <stdio.h>
#include <windows.h>
#include "libretro.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>

#include "NesEmulator.h"

static const void *pixelData;
static retro_game_info *info;

static struct {
	HMODULE handle;
	bool initialized;
	void(*retro_init)(void);
	void(*retro_deinit)(void);
	unsigned(*retro_api_version)(void);
	void(*retro_get_system_info)(struct retro_system_info *info);
	void(*retro_get_system_av_info)(struct retro_system_av_info *info);
	void(*retro_set_controller_port_device)(unsigned port, unsigned device);
	void(*retro_reset)(void);
	void(*retro_run)(void);
	void*(*retro_get_memory_data)(unsigned id);
	//void *(retro_get_memory_data)(unsigned id);
	//	size_t retro_serialize_size(void);
	//	bool retro_serialize(void *data, size_t size);
	//	bool retro_unserialize(const void *data, size_t size);
	//	void retro_cheat_reset(void);
	//	void retro_cheat_set(unsigned index, bool enabled, const char *code);
	bool(*retro_load_game)(const struct retro_game_info *game);
	//	bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info);
	void(*retro_unload_game)(void);
	//	unsigned retro_get_region(void);
	// void *retro_get_memory_data(unsigned id);
	//	size_t retro_get_memory_size(unsigned id);
} g_retro;

static struct {
	unsigned int tex_id;
	unsigned int pitch;
	int tex_w, tex_h;
	unsigned int clip_w, clip_h;

	unsigned int pixfmt;
	unsigned int pixtype;
	unsigned int bpp;
} g_video = { 0 };

static void core_log(enum retro_log_level level, const char *fmt, ...) {
	char buffer[4096] = { 0 };
	static const char * levelstr[] = { "dbg", "inf", "wrn", "err" };
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	if (level == 0)
		return;

	fprintf(stderr, "[%s] %s", levelstr[level], buffer);
	fflush(stderr);

	if (level == RETRO_LOG_ERROR)
		exit(EXIT_FAILURE);
}

static bool core_environment(unsigned cmd, void *data) {
	bool *bval;

	switch (cmd) {
	case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
		struct retro_log_callback *cb = (struct retro_log_callback *)data;
		cb->log = core_log;
		break;
	}
	case RETRO_ENVIRONMENT_GET_CAN_DUPE:
		bval = (bool*)data;
		*bval = true;
		break;
	case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
		const enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;

		if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
			return false;

		return true;
	}
	case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
		return false;
	}
	case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL: {
		return false;
	}
	case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
		return true;
	}
	case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
		// TODO: Give this a proper home I guess?
		data = GetCurrentDirectory;
		return true;
	}
	default:
		core_log(RETRO_LOG_DEBUG, "Unhandled env #%u", cmd);
		return false;
	}

	return true;
}

static void video_configure(const struct retro_game_geometry *geom) {
	g_video.tex_id = 0;

	g_video.pixfmt = 0;

	g_video.tex_w = geom->max_width;
	g_video.tex_h = geom->max_height;
	g_video.clip_w = geom->base_width;
	g_video.clip_h = geom->base_height;
}

static void core_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
	if (data)
		pixelData = data;
}

struct keymap {
	unsigned k;
	unsigned rk;
};

struct keymap g_binds[] = {
	{ 0, RETRO_DEVICE_ID_JOYPAD_A },
	{ 1, RETRO_DEVICE_ID_JOYPAD_B },
	{ 2, RETRO_DEVICE_ID_JOYPAD_Y },
	{ 3, RETRO_DEVICE_ID_JOYPAD_X },
	{ 4, RETRO_DEVICE_ID_JOYPAD_UP },
	{ 5, RETRO_DEVICE_ID_JOYPAD_DOWN },
	{ 6, RETRO_DEVICE_ID_JOYPAD_LEFT },
	{ 7, RETRO_DEVICE_ID_JOYPAD_RIGHT },
	{ 8, RETRO_DEVICE_ID_JOYPAD_START },
	{ 9, RETRO_DEVICE_ID_JOYPAD_SELECT },

	{ 0, 0 }
};

static unsigned g_joy[RETRO_DEVICE_ID_JOYPAD_R3 + 1] = { 0 };

static void core_input_poll(void) {
	int i;
	for (i = 0; g_binds[i].k || g_binds[i].rk; ++i)
		g_joy[g_binds[i].rk] = 0;
}

static int16_t core_input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
	if (port || index || device != RETRO_DEVICE_JOYPAD)
		return 0;

	return g_joy[id];
}

//static void core_audio_sample(int16_t left, int16_t right) {
//	int16_t buf[2] = { left, right };
//	audio_write(buf, 1);
//}
//
//static size_t core_audio_sample_batch(const int16_t *data, size_t frames) {
//	return audio_write(data, frames);
//}
//
//static size_t audio_write(const void *buf, unsigned frames) {
//	int written = snd_pcm_writei(g_pcm, buf, frames);
//
//	if (written < 0) {
//		printf("Alsa warning/error #%i: ", -written);
//		snd_pcm_recover(g_pcm, written, 0);
//
//		return 0;
//	}
//
//	return written;
//}

#define load_sym(V, S) do {\
	if (!((*(void**)&V) = GetProcAddress(g_retro.handle, #S))) \
		exit(0); \
	} while (0)
#define load_retro_sym(S) load_sym(g_retro.S, S)

static void core_load() {
	void(*set_environment)(retro_environment_t) = NULL;
	void(*set_video_refresh)(retro_video_refresh_t) = NULL;
	void(*set_input_poll)(retro_input_poll_t) = NULL;
	void(*set_input_state)(retro_input_state_t) = NULL;
	void(*set_audio_sample)(retro_audio_sample_t) = NULL;
	void(*set_audio_sample_batch)(retro_audio_sample_batch_t) = NULL;

	memset(&g_retro, 0, sizeof(g_retro));
	g_retro.handle = LoadLibrary(L"msvc-2010.dll");

	load_retro_sym(retro_init);
	load_retro_sym(retro_deinit);
	load_retro_sym(retro_api_version);
	load_retro_sym(retro_get_system_info);
	load_retro_sym(retro_get_system_av_info);
	load_retro_sym(retro_set_controller_port_device);
	load_retro_sym(retro_reset);
	load_retro_sym(retro_run);
	load_retro_sym(retro_load_game);
	load_retro_sym(retro_unload_game);
	load_retro_sym(retro_get_memory_data);
	//g_retro.retro_get_memory_data = GetProcAddress(g_retro.handle, (LPCSTR)retro_get_memory_data);

	load_sym(set_environment, retro_set_environment);
	load_sym(set_video_refresh, retro_set_video_refresh);
	load_sym(set_input_poll, retro_set_input_poll);
	load_sym(set_input_state, retro_set_input_state);
	load_sym(set_audio_sample, retro_set_audio_sample);
	load_sym(set_audio_sample_batch, retro_set_audio_sample_batch);

	set_environment(core_environment);
	set_video_refresh(core_video_refresh);
	set_input_poll(core_input_poll);
	set_input_state(core_input_state);
	// set_audio_sample(core_audio_sample);
	// set_audio_sample_batch(core_audio_sample_batch);

	g_retro.retro_init();
	g_retro.initialized = true;

	puts("Core loaded");
}

static void core_load_game(const char *filename) {
	struct retro_system_av_info av = { 0 };
	struct retro_system_info system = { 0 };
	printf(filename);
	info = new retro_game_info { filename, 0 };
	FILE *file;
	fopen_s(&file, filename, "rb");

	if (!file)
		goto libc_error;

	fseek(file, 0, SEEK_END);
	info->size = ftell(file);
	rewind(file);

	g_retro.retro_get_system_info(&system);

	if (!system.need_fullpath) {
		info->data = malloc(info->size);

		if (!info->data || !fread((void*)info->data, info->size, 1, file))
			goto libc_error;
	}

	if (!g_retro.retro_load_game(info))
		exit(0);

	g_retro.retro_get_system_av_info(&av);

	video_configure(&av.geometry);
	// audio_init(av.timing.sample_rate);

	return;

libc_error:
	exit(0);
}

void NesEmulator::Initialize(void) {
	core_load();
	core_load_game("c:\\mario.nes");
}

void NesEmulator::ExecuteFrame() {
	g_retro.retro_run();
}

const void* NesEmulator::getPixelData() {
	return pixelData;
}

const void* NesEmulator::getVRam() {
	return g_retro.retro_get_memory_data(RETRO_MEMORY_VIDEO_RAM);
}

retro_game_info *NesEmulator::getGameInfo()
{
	return info;
}

//int main()
//{
//	core_load();
//	core_load_game("c:\\mario.nes");
//	g_retro.retro_run();
//	for (int i = 0; i < 10000; i++) {
//		g_retro.retro_run();
//		byte* vRam = (byte*)g_retro.retro_get_memory_data(RETRO_MEMORY_VIDEO_RAM);
//		int palette[32];
//		for (int v = 0; v < 32; v++)
//			palette[v] = (int)vRam[v];
//		g_retro.retro_get_memory_data(RETRO_MEMORY_VIDEO_RAM);
//		//printf(palette);
//		//printf("\n");
//	}
//	return 0;
//}