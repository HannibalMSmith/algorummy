#pragma once
#include <map>
#include <memory>
#include "gamerummy.h"

#ifdef _DEBUG
#include <stdio.h>
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') + 1)
#define LOG(format, ...) printf("[debug]: %s:%d:%s: " format"\n", __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG(format, ...) {}
#endif

const char* scriptPrompt(int argc, char **argv);
bool init(const char* script, std::map<int, std::shared_ptr<Card>> &hand, Card &magic);
