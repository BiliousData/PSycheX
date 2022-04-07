/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_CUSTOM_H
#define PSXF_GUARD_CUSTOM_H

#include "psx.h"
#include "pad.h"
#include "fixed.h"
#include "io.h"

//controller stuff

//custom stuff
typedef struct
{
    //colors
    struct
    {
        u8 red;
        u8 green;
        u8 blue;
    } leftcol, downcol, upcol, rightcol;

    //other stuff
    u8 noteskin; //will select note texture when implemented. 0 is the og arrows, 1 one and onward is... whatever I feel like adding.
    u8 colorable; //determines if noteskin is colorable.
    u8 noteselect;

    //selection
    u8 leftX;
    u8 leftY;
    u8 downX;
    u8 downY;
    u8 upX;
    u8 upY;
    u8 rightX;
    u8 rightY;

} Custom;

extern Custom custom;

typedef enum
{
    Main,
    Color,
    Skins,
} CustomState; //What menu the player is in

void Custom_Tick();
void Custom_Load();
void Custom_DrawNotes();
#endif