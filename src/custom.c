/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "custom.h"
#include "menu.h"
#include "stage.h"
#include "random.h"
#include "audio.h"
#include "trans.h"
#include "main.h"
#include "counter.h"

void BackPick()
{
    stage.backpick = RandomRange(0, 2);
}

static struct
{
    Gfx_Tex tex_back, tex_back2, tex_back3, tex_normal;
} asset;

Custom custom;

Counter counter;

static void Menu_DrawBack()
{
	RECT back_src = {0, 0, 255, 255};
	RECT back_dst = {0, - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};

	RECT back2_src = {0, 0, 255, 255};
	RECT back2_dst = {0, - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};

	RECT back3_src = {0, 0, 255, 255};
	RECT back3_dst = {0, - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
	
	if (stage.backpick == 0)
	{
	    Gfx_DrawTex(&asset.tex_back, &back_src, &back_dst);
	}
	else if (stage.backpick == 1)
	{
	    Gfx_DrawTex(&asset.tex_back2, &back2_src, &back2_dst);
	}
	else if (stage.backpick == 2 )
	{
	    Gfx_DrawTex(&asset.tex_back3, &back3_src, &back3_dst);
	}
}

void Custom_DrawNotes(void)
{
    RECT left_src = {0, 0, 32, 32};
    RECT down_src = {32, 0, 32, 32};
    RECT up_src = {64, 0, 32, 32};
    RECT right_src = {96, 0, 32, 32};

    RECT left_dst = {custom.leftX, custom.leftY, 32, 32};
    RECT down_dst = {custom.downX, custom.downY, 32, 32};
    RECT up_dst = {custom.upX, custom.upY, 32, 32};
    RECT right_dst = {custom.rightX, custom.rightY, 32, 32};

    Gfx_DrawTexCol(&asset.tex_normal, &left_src, &left_dst, custom.leftcol.red, custom.leftcol.green, custom.leftcol.blue);
    Gfx_DrawTex(&asset.tex_normal, &down_src, &down_dst);
    Gfx_DrawTex(&asset.tex_normal, &up_src, &up_dst);
    Gfx_DrawTex(&asset.tex_normal, &right_src, &right_dst);
}

void Custom_Load()
{
    //Load assets
    IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
    Gfx_LoadTex(&asset.tex_back,  Archive_Find(menu_arc, "back.tim"),  0);
	Gfx_LoadTex(&asset.tex_back2, Archive_Find(menu_arc, "back2.tim"), 0);
	Gfx_LoadTex(&asset.tex_back3, Archive_Find(menu_arc, "back3.tim"), 0);
    Mem_Free(menu_arc);

    IO_Data notes_arc = IO_Read("\\MENU\\NOTES.ARC;1");
    Gfx_LoadTex(&asset.tex_normal, Archive_Find(notes_arc, "normal.tim"), 0);
    Mem_Free(notes_arc);

    

    BackPick();

    Trans_Clear();

    custom.noteselect = 0;

    Audio_PlayXA_Track(XA_Ludum, 0x40, 0, 1);
	Audio_WaitPlayXA();

    custom.leftX = 100;
    custom.leftY = 100;
    custom.downX = 132;
    custom.downY = 100;
    custom.upX = 164;
    custom.upY = 100;
    custom.rightX = 196;
    custom.rightY = 100;
}

void Custom_Tick(void)
{

    switch (custom.CustomState)
    {
        case Main:
        {
            //Change option
			if (pad_state.press & PAD_LEFT && custom.noteselect > 0)
			{
				custom.noteselect--;
			}
            else if (pad_state.press & PAD_LEFT && custom.noteselect == 0)
            {
                custom.noteselect = 3;
            }
			if (pad_state.press & PAD_RIGHT && custom.noteselect < 3)
			{
				custom.noteselect++;
			}
            else if (pad_state.press & PAD_RIGHT && custom.noteselect == 3)
            {
                custom.noteselect = 0;
            }

            if (pad_state.press & PAD_CROSS)
            {

                if (custom.noteselect == 0)
                {
                    for (custom.leftY  <= 100; custom.leftY  > 80; custom.leftY   = custom.leftY  - 1);
                    for (custom.downY  >= 100; custom.downY  = -20; custom.downY  = custom.downY  + 1);
                    for (custom.upY    >= 100; custom.upY    = -20; custom.upY    = custom.upY    + 1);
                    for (custom.rightY >= 100; custom.rightY = -20; custom.rightY = custom.rightY + 1);
                    custom.CustomState = Color;
                }
            }

            switch (custom.noteselect)
            {
                case 0:
                {
                    for (custom.leftY  <= 100; custom.leftY  > 80; custom.leftY   = custom.leftY  - 1);
                    for (custom.downY  >= 100; custom.downY  < 100; custom.downY  = custom.downY  + 1);
                    for (custom.upY    >= 100; custom.upY    < 100; custom.upY    = custom.upY    + 1);
                    for (custom.rightY >= 100; custom.rightY < 100; custom.rightY = custom.rightY + 1);
                    break;
                }
                case 1:
                {
                    for (custom.leftY  >= 100; custom.leftY  < 100; custom.leftY  = custom.leftY  + 1);
                    for (custom.downY  <= 100; custom.downY  >  80; custom.downY  = custom.downY  - 1);
                    for (custom.upY    >= 100; custom.upY    < 100; custom.upY    = custom.upY    + 1);
                    for (custom.rightY >= 100; custom.rightY < 100; custom.rightY = custom.rightY + 1);
                    break;
                }
                case 2:
                {
                    for (custom.leftY  >= 100; custom.leftY  < 100; custom.leftY  = custom.leftY  + 1);
                    for (custom.downY  >= 100; custom.downY  < 100; custom.downY  = custom.downY  + 1);
                    for (custom.upY    <= 100; custom.upY    >  80; custom.upY    = custom.upY    - 1);
                    for (custom.rightY >= 100; custom.rightY < 100; custom.rightY = custom.rightY + 1);
                    break;
                }
                case 3:
                {
                    for (custom.leftY  >= 100; custom.leftY  < 100; custom.leftY  = custom.leftY  + 1);
                    for (custom.downY  >= 100; custom.downY  < 100; custom.downY  = custom.downY  + 1);
                    for (custom.upY    >= 100; custom.upY    < 100; custom.upY    = custom.upY    + 1);
                    for (custom.rightY <= 100; custom.rightY >  80; custom.rightY = custom.rightY - 1);
                    break;
                }
            }
            break;

        }
        case Color:
        {
            if (pad_state.press & PAD_CIRCLE)
            {
                custom.CustomState = Main;
            }

            switch (custom.noteselect)
            {
                case 0:
                {
                    break;
                }
            }
        }
    }

    if (pad_state.held & PAD_START)
    {
        Skidaddle();
    }

    Custom_DrawNotes();

    Menu_DrawBack();
}

void Skidaddle()
{
    Trans_Start();
    LoadScr_Start();
    gameloop = GameLoop_Menu;
    Menu_Load(MenuPage_Options);
    LoadScr_End();
}