#pragma once
#include <stdbool.h>

void otaInit();
void otaLoop();
bool otaCheckNow(bool force);
