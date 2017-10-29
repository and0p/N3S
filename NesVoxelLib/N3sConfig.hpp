#pragma once

#include "Input.hpp"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include "Common.hpp"
#include "PpuSnapshot.hpp"	// TODO don't need?

using json = nlohmann::json;

#define NO_OVERRIDE		-1
#define OVERRIDE_FALSE	0
#define OVERRIDE_TRUE	1

class N3sConfig
{
public:
	static void init();
	static bool anyRegistersOveridden();
	static void load();
	static void save();
	static void update(shared_ptr<PpuSnapshot> snapshot);
	static bool options[N3S_OPTION_SIZE];
	static int registers[REGISTER_OPTION_SIZE];
	static UINT nesRegisters[REGISTER_OPTION_SIZE];
	static bool getOption(N3sOption o);
	static void setOption(N3sOption o, bool val);
	static void toggleOption(N3sOption o);
	static UINT getRegisterOverride(RegisterOption o);
	static bool isRegisterActive(RegisterOption o);
	static void setRegisterOverride(RegisterOption o, int val);
	static void resetRegisterOverrides();
	static bool anyRegisterOverridden();
};