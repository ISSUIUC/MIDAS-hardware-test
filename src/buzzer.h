#pragma once
#include <stdint.h>

struct Sound {
public:
    uint32_t frequency;
    int duration_ms;
};

#define EIGHT 250
#define QUART 500
#define HALF 1000
#define WHOLE 2000


#define G4 static_cast<uint32_t>(392)
#define A4 static_cast<uint32_t>(440)
#define B4 static_cast<uint32_t>(494)
#define C5 static_cast<uint32_t>(523)
#define D5 static_cast<uint32_t>(587)
#define E5 static_cast<uint32_t>(659)
#define F5 static_cast<uint32_t>(698)

#define MERRY_CHRISTMAS_LENGTH 32


Sound merry_christmas[MERRY_CHRISTMAS_LENGTH] = {
    /*measure 1*/ Sound{G4, QUART}, Sound{C5, QUART}, Sound{C5, EIGHT}, Sound{D5, EIGHT}, Sound{C5, EIGHT}, Sound{B4, EIGHT},
    Sound{A4, QUART}, Sound{A4, QUART}, Sound{A4, QUART},
    /*measure 2*/ Sound{D5, QUART}, Sound{D5, EIGHT}, Sound{E5, EIGHT}, Sound{D5, EIGHT}, Sound{C5, EIGHT}, Sound{B4, QUART}, Sound{G4, QUART}, Sound{G4, QUART},
    /*measure 3*/ Sound{E5, QUART}, Sound{E5, EIGHT}, Sound{F5, EIGHT}, Sound{E5, EIGHT}, Sound{D5, EIGHT}, Sound{C5, QUART}, Sound{A4, QUART}, Sound{G4, EIGHT}, Sound{G4, EIGHT},
    /*measure 4*/ Sound{A4, QUART}, Sound{D5, QUART}, Sound{B4, QUART}, Sound{C5, QUART}
};
