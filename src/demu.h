/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_DEMU_H
#define PSXF_GUARD_DEMU_H

#include "stage.h"

//Demu enums
typedef enum
{
	DemuPage_Opening,
	DemuPage_Title,
	DemuPage_Main,
	DemuPage_Story,
	DemuPage_Freeplay,
	DemuPage_Credits,
	DemuPage_Options,
	
	#ifdef PSXF_NETWORK
		DemuPage_NetJoin,
		DemuPage_NetHost,
		DemuPage_NetConnect,
		DemuPage_NetFail,
		DemuPage_NetOpWait,
		DemuPage_NetOp,
		DemuPage_NetLobby,
		DemuPage_NetInitFail,
	#endif
	
	DemuPage_Stage, //Changes game loop
	DemuPage_Update,
} DemuPage;

//Demu functions
void Demu_Load(DemuPage page);
void Demu_Unload();
void Demu_ToStage(StageId id, StageDiff diff, boolean story);
void Demu_Tick();

#endif
