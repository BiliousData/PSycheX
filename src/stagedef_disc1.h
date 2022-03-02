	{ //StageId_1_1 (Psychic)
		//Characters
		{Char_BF_New,    FIXED_DEC(310,1),  FIXED_DEC(100,1)},
		{Char_Psychic_New, FIXED_DEC(10,1),  FIXED_DEC(70,1)},
		{NULL},
		
		//Stage background
		Back_Fireplace_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 1,
		XA_Bopeebo, 0,
		
		StageId_1_2, STAGE_LOAD_FLAG || STAGE_LOAD_PLAYER
	},
	{ //StageId_1_2 (Wilter)
		//Characters
		{Char_SenpaiB_New,    FIXED_DEC(310,1),  FIXED_DEC(100,1)},
		{Char_Psychic_New, FIXED_DEC(10,1),  FIXED_DEC(70,1)},
		{NULL},
		
		//Stage background
		Back_Fireplace_New,
		
		//Song info
		{FIXED_DEC(12,10),FIXED_DEC(2,1),FIXED_DEC(26,10)},
		1, 2,
		XA_Fresh, 2,
		
		StageId_1_6, STAGE_LOAD_FLAG | STAGE_LOAD_PLAYER | STAGE_LOAD_STAGE
	},
	{ //StageId_1_3 (Uproar)
		//Characters
		{Char_BFS_New,    FIXED_DEC(310,1),  FIXED_DEC(100,1)},
		{Char_Psychic_New, FIXED_DEC(10,1),  FIXED_DEC(70,1)},
		{NULL},
		
		//Stage background
		Back_Flames_New,
		
		//Song info
		{FIXED_DEC(17,10),FIXED_DEC(23,10),FIXED_DEC(3,1)},
		1, 3,
		XA_Dadbattle, 0,
		
		StageId_2_3, STAGE_LOAD_FLAG | STAGE_LOAD_PLAYER | STAGE_LOAD_STAGE | STAGE_LOAD_OPPONENT
	},
	{ //StageId_1_4 (Tutorial)
		//Characters
		{Char_BF_New, FIXED_DEC(60,1),  FIXED_DEC(100,1)},
		{Char_GF_New,  FIXED_DEC(0,1),  FIXED_DEC(-15,1)}, //TODO
		{NULL,           FIXED_DEC(0,1),  FIXED_DEC(-10,1)},
		
		//Stage background
		Back_Flames_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(1,1)},
		1, 4,
		XA_Tutorial, 2,
		
		StageId_1_4, 0
	},
	{ //StageId_1_5 (Bf Transformation cutscene)
		//Characters
		{Char_BF_New,    FIXED_DEC(310,1),  FIXED_DEC(100,1)},
		{Char_Psychic_New, FIXED_DEC(10,1),  FIXED_DEC(70,1)},
		{NULL},
		
		//Stage background
		Back_Fireplace_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 1,
		XA_BfTransform, 0,
		
		StageId_1_2, STAGE_LOAD_FLAG || STAGE_LOAD_PLAYER
	},
	{ //StageId_1_6 (Uproar Cutscene)
		//Characters
		{Char_Sendai_New,    FIXED_DEC(310,1),  FIXED_DEC(100,1)},
		{Char_Psychic_New, FIXED_DEC(10,1),  FIXED_DEC(70,1)},
		{NULL},
		
		//Stage background
		Back_Fireplace_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 6,
		XA_Sendai, 0,
		
		StageId_1_3, 0,
	},
	
	{ //StageId_2_1 (Spookeez)
		//Characters
		{Char_BF_New,      FIXED_DEC(310,1),   FIXED_DEC(100,1)},
		{Char_Psychic_New,  FIXED_DEC(10,1),   FIXED_DEC(70,1)},
		{Char_GF_New,       FIXED_DEC(205,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Fireplace_New,
		
		//Song info
		{FIXED_DEC(15,10),FIXED_DEC(2,1),FIXED_DEC(25,10)},
		1, 1,
		XA_Bopeebo, 0,
		
		StageId_2_1, 0
	},
	{ //StageId_2_2 (Late Drive)
		//Characters
		{Char_BF_New,      FIXED_DEC(310,1),   FIXED_DEC(100,1)},
		{Char_Psychic_New,  FIXED_DEC(10,1),   FIXED_DEC(70,1)},
		{Char_GF_New,       FIXED_DEC(205,1),  FIXED_DEC(-15,1)},
		
		//Stage background
		Back_Space_New,
		
		//Song info
		{FIXED_DEC(25,10),FIXED_DEC(25,10),FIXED_DEC(25,10)},
		2, 2,
		XA_South, 2,
		
		StageId_2_2, 0
	},
	{ //StageId_2_3 (Monster)
		//Characters
		{Char_End_New,  FIXED_DEC(310,1),   FIXED_DEC(-155,1)},
		{Char_Sendai_New,  FIXED_DEC(-90,1),   FIXED_DEC(85,1)},
		{NULL},
		
		//Stage background
		Back_Dummy_New,
		
		//Song info
		{FIXED_DEC(1,1),FIXED_DEC(1,1),FIXED_DEC(1,1)},
		2, 3,
		XA_Monster, 0,
		
		StageId_2_3, 0
	},
