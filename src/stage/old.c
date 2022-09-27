/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "old.h"

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

	//fire
	u8 fire_frame, fire_tex_id;
	Animatable fire_animatable;

	//wick
	u8 wick_frame, wick_tex_id;
	Animatable wick_animatable;
	
} Back_Old;

static const CharFrame fire_frame[4] = {
	{0, {169,   0,  59, 51}, {0,  0}},
	{0, {105,  79,  55, 51}, {-1, 0}},
	{0, {160,  79,  59, 64}, {0, 12}},
	{0, {105, 143,  57, 59}, {-1, 8}},
};

static const Animation fire_anim[1] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_REPEAT}},
};

void Old_Fire_Draw(Back_Old *this, fixed_t x, fixed_t y)
{
	//Draw animated object
	const CharFrame *cframe = &fire_frame[this->fire_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_junk, &src, &dst, stage.camera.bzoom);
}

void Old_Fire_SetFrame(void *user, u8 frame)
{
	Back_Old *this = (Back_Old*)user;
	
	//Check if this is a new frame
	if (frame != this->fire_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &fire_frame[this->fire_frame = frame];
	}
}

static const CharFrame wick_frame[4] = {
	{0, {  0, 175,  4,  8}, {0,  0}},
	{0, {  4, 175,  4,  9}, {0,  1}},
	{0, {  8, 175,  4,  7}, {0, -1}},
	{0, { 12, 175,  5,  7}, {0, -1}},
};

static const Animation wick_anim[1] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_REPEAT}},
};

void Old_Wick_Draw(Back_Old *this, fixed_t x, fixed_t y)
{
	//Draw animated object
	const CharFrame *cframe = &wick_frame[this->wick_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_junk, &src, &dst, stage.camera.bzoom);
}

void Old_Wick_SetFrame(void *user, u8 frame)
{
	Back_Old *this = (Back_Old*)user;
	
	//Check if this is a new frame
	if (frame != this->wick_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &fire_frame[this->wick_frame = frame];
	}
}

void Back_Old_DrawBG(StageBack *back)
{
	Back_Old *this = (Back_Old*)back;
	

	
	fixed_t fx, fy;

	//Animatable_Animate(&this->fire_animatable, (void*)this, Old_Fire_SetFrame);
	//Animatable_Animate(&this->steam_animatable, (void*)this, Old_Steam_SetFrame);
	//Animatable_Animate(&this->wick_animatable, (void*)this, Old_Wick_SetFrame);
	
	
	
	//Draw room
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT halll_src = {0, 0, 255, 256};
	RECT_FIXED halll_dst = {
		FIXED_DEC(-125 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT hallr_src = {0, 0, 256, 256};
	RECT_FIXED hallr_dst = {
		FIXED_DEC(120 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT floorl_src = {0, 0, 256, 116};
	RECT_FIXED floorl_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(76,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT floorr_src = {0, 116, 256, 116};
	RECT_FIXED floorr_dst = {
		FIXED_DEC(90 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(76,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT chair_src = {0, 0, 111, 163};
	RECT_FIXED chair_dst = {
		FIXED_DEC(-124 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-27,1) - fy,
		FIXED_DEC(111 + SCREEN_WIDEOADD,1),
		FIXED_DEC(163,1)
	};

	RECT fireplace_src = {0, 0, 166, 256};
	RECT_FIXED fireplace_dst = {
		FIXED_DEC(17 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-156,1) - fy,
		FIXED_DEC(166 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	//Old_Fire_Draw(this, FIXED_DEC(85,1) - fx, FIXED_DEC(31,1) - fy);
	//Old_Wick_Draw(this, FIXED_DEC(41,1) - fx, FIXED_DEC(-26,1) - fy);
	//Old_Wick_Draw(this, FIXED_DEC(55,1) - fx, FIXED_DEC(-21,1) - fy);
	//Old_Wick_Draw(this, FIXED_DEC(184,1) - fx, FIXED_DEC(-26,1) - fy);
	
	Stage_DrawTex(&this->tex_fireplace, &fireplace_src, &fireplace_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_junk, &chair_src, &chair_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_floor, &floorl_src, &floorl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_floor, &floorr_src, &floorr_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back0, &halll_src, &halll_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &hallr_src, &hallr_dst, stage.camera.bzoom);
}

void Back_Old_Free(StageBack *back)
{
	Back_Old *this = (Back_Old*)back;
	
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Old_New(void)
{
	//Allocate background structure
	Back_Old *this = (Back_Old*)Mem_Alloc(sizeof(Back_Old));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Old_DrawBG;
	this->back.free = Back_Old_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\PSYCHE\\OLD.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_floor, Archive_Find(arc_back, "floor.tim"), 0);
	Gfx_LoadTex(&this->tex_junk, Archive_Find(arc_back, "junk.tim"), 0);
	Gfx_LoadTex(&this->tex_fireplace, Archive_Find(arc_back, "fplace.tim"), 0);
	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);

	//Animatable_Init(&this->fire_animatable, fire_anim);
	//Animatable_SetAnim(&this->fire_animatable, 0);

	//Animatable_Init(&this->steam_animatable, steam_anim);
	//Animatable_SetAnim(&this->steam_animatable, 0);

	//Animatable_Init(&this->wick_animatable, wick_anim);
	//Animatable_SetAnim(&this->wick_animatable, 0);
	
	
	
	return (StageBack*)this;
}
