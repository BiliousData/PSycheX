/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "chop.h"

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
	Gfx_Tex tex_back0; //Wall left
	Gfx_Tex tex_back1; //Wall right

	Gfx_Tex tex_floor; //Floor

	Gfx_Tex tex_junk; //random objects

	Gfx_Tex tex_fireplace; //fireplace

	u8 fire_frame, fire_tex_id;
	Animatable fire_animatable;
	
} Back_Chop;

static const CharFrame fire_frame[4] = {
	{0, {169,   0,  59, 51}, {0,  0}},
	{0, {105,  79,  55, 51}, {-1, 0}},
	{0, {160,  79,  59, 64}, {0, 12}},
	{0, {105, 143,  57, 59}, {-1, 8}},
};

static const Animation fire_anim[1] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_REPEAT}},
};

void Chop_Fire_Draw(Back_Chop *this, fixed_t x, fixed_t y)
{
	//Draw animated object
	const CharFrame *cframe = &fire_frame[this->fire_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_junk, &src, &dst, stage.camera.bzoom);
}

void Chop_Fire_SetFrame(void *user, u8 frame)
{
	Back_Chop *this = (Back_Chop*)user;
	
	//Check if this is a new frame
	if (frame != this->fire_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &fire_frame[this->fire_frame = frame];
	}
}




void Back_Chop_DrawFG(StageBack *back)
{
	Back_Chop *this = (Back_Chop*)back;
	
	fixed_t fx, fy;
	
	
}

void Back_Chop_DrawBG(StageBack *back)
{
	Back_Chop *this = (Back_Chop*)back;
	

	
	fixed_t fx, fy;

	Animatable_Animate(&this->fire_animatable, (void*)this, Chop_Fire_SetFrame);
	
	
	
	//Draw room
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT halll_src = {0, 0, 255, 256};
	RECT_FIXED halll_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT hallr_src = {0, 0, 256, 256};
	RECT_FIXED hallr_dst = {
		FIXED_DEC(90 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT floorl_src = {0, 0, 255, 125};
	RECT_FIXED floorl_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(76,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT floorr_src = {0, 125, 255, 125};
	RECT_FIXED floorr_dst = {
		FIXED_DEC(91 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(74,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT chair_src = {0, 0, 105, 175};
	RECT_FIXED chair_dst = {
		FIXED_DEC(-107 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-52,1) - fy,
		FIXED_DEC(105 + SCREEN_WIDEOADD,1),
		FIXED_DEC(175,1)
	};

	RECT table_src = {105, 0, 64, 79};
	RECT_FIXED table_dst = {
		FIXED_DEC(245 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(42,1) - fy,
		FIXED_DEC(64 + SCREEN_WIDEOADD,1),
		FIXED_DEC(79,1)
	};

	RECT fireplace_src = {0, 0, 213, 256};
	RECT_FIXED fireplace_dst = {
		FIXED_DEC(8 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-145,1) - fy,
		FIXED_DEC(213 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	Chop_Fire_Draw(this, FIXED_DEC(85,1) - fx, FIXED_DEC(31,1) - fy);
	
	Stage_DrawTex(&this->tex_fireplace, &fireplace_src, &fireplace_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_junk, &table_src, &table_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_junk, &chair_src, &chair_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_floor, &floorl_src, &floorl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_floor, &floorr_src, &floorr_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back0, &halll_src, &halll_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &hallr_src, &hallr_dst, stage.camera.bzoom);
}

void Back_Chop_Free(StageBack *back)
{
	Back_Chop *this = (Back_Chop*)back;
	
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Chop_New(void)
{
	//Allocate background structure
	Back_Chop *this = (Back_Chop*)Mem_Alloc(sizeof(Back_Chop));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Chop_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Chop_DrawBG;
	this->back.free = Back_Chop_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\PSYCHE\\COOL.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_floor, Archive_Find(arc_back, "floor.tim"), 0);
	Gfx_LoadTex(&this->tex_junk, Archive_Find(arc_back, "junk.tim"), 0);
	Gfx_LoadTex(&this->tex_fireplace, Archive_Find(arc_back, "fplace.tim"), 0);
	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);

	Animatable_Init(&this->fire_animatable, fire_anim);
	Animatable_SetAnim(&this->fire_animatable, 0);
	
	
	
	return (StageBack*)this;
}
