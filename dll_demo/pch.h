// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <Windows.h>
#include <array>
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include <flashgui.h>

#pragma comment(lib, "flashgui.lib")
#endif //PCH_H
