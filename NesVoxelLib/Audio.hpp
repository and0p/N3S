#pragma once
#include "mmreg.h"
#include <dsound.h>
#include <stdint.h>

typedef uint16_t u16;

// Stolen shamelessly from https ://github.com/visualboyadvance/vbam-libretro/blob/master/src/common/SoundDriver.h

class SoundDriver {
public:

	/**
	* Destructor. Free the resources allocated by the sound driver.
	*/
	virtual ~SoundDriver() { };

	/**
	* Initialize the sound driver.
	* @param sampleRate In Hertz
	*/
	virtual bool init(HWND hwnd, long sampleRate) = 0;

	/**
	* Tell the driver that the sound stream has paused
	*/
	virtual void pause() = 0;

	/**
	* Reset the sound driver
	*/
	virtual void reset() = 0;

	/**
	* Tell the driver that the sound stream has resumed
	*/
	virtual void resume() = 0;

	/**
	* Write length bytes of data from the finalWave buffer to the driver output buffer.
	*/
	virtual void write(u16 * finalWave, int length) = 0;

	virtual void setThrottle(unsigned short throttle) { };
};