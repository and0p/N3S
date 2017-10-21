#pragma once

#include "Input.hpp"
#include "json.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

// TODO struct for various configs; video, audio, gamestate, etc

class N3sConfig
{
public:
	static void load();
	static void save();
};