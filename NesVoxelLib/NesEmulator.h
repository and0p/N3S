#pragma once

class NesEmulator {
public:
	static void Initialize(void);
	static void ExecuteFrame();
	static const void* getPixelData();
	static const void* getVRam();
};