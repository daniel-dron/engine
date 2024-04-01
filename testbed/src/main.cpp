#include <stdio.h>
#include "main.h"
#include <engine.h>

extern "C" void init_game() {
    printf("hello world!");
    engine_api();
}