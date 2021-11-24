#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <ostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

#ifdef RUI_PLATFORM_WINDOWS
	#define NOMINMAX
	#include <Windows.h>
#endif