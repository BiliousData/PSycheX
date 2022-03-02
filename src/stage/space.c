/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "space.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back3; //Hallway floor left
	Gfx_Tex tex_back4; //Hallway floor right
	
} Back_Space;





void Back_Space_DrawFG(StageBack *back)
{
	Back_Space *this = (Back_Space*)back;
	
	fixed_t fx, fy;
	        
	
}

void Back_Space_DrawBG(StageBack *back)
{
	Back_Space *this = (Back_Space*)back;
	

	
	fixed_t fx, fy;
	
	
	
	//Draw sunset
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT halll_src = {0, 0, 256, 256};
	RECT_FIXED halll_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(340 + SCREEN_WIDEOADD,1),
		FIXED_DEC(260,1)
	};

	RECT hallr_src = {0, 0, 256, 256};
	RECT_FIXED hallr_dst = {
		FIXED_DEC(173 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(340 + SCREEN_WIDEOADD,1),
		FIXED_DEC(260,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &halll_src, &halll_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back4, &hallr_src, &hallr_dst, stage.camera.bzoom);
}

void Back_Space_Free(StageBack *back)
{
	Back_Space *this = (Back_Space*)back;
	
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Space_New(void)
{
	//Allocate background structure
	Back_Space *this = (Back_Space*)Mem_Alloc(sizeof(Back_Space));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Space_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Space_DrawBG;
	this->back.free = Back_Space_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\PSYCHE\\SPACE.ARC;1");
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Gfx_LoadTex(&this->tex_back4, Archive_Find(arc_back, "back4.tim"), 0);
	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);
	
	
	
	return (StageBack*)this;
}
