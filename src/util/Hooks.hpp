#pragma once
#include <cstdint>

#include "pl/Signature.h"
#include "pl/Hook.h"
#include "util/Logger.h"

#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>

bool VHOOK(const char*, int, void*, void**);
bool HOOK(const char*, void*, void**);