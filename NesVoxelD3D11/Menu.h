#pragma once
#include <N3sConfig.hpp>
#include "resource.h"
#include <N3sApp.hpp>

void initMenu();
void updateMenu(HMENU hmenu, N3sApp * app);
void updateNesRegistry(UINT button);