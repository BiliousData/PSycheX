#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(17000)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(3840)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(11800)}, //XA_Bopeebo
	{XA_Week1A, XA_LENGTH(11400)}, //XA_Fresh
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(12000)}, //XA_Dadbattle
	{XA_Week1B, XA_LENGTH(6800)}, //XA_Tutorial
	//WEEK2A.XA
	{XA_Week2A, XA_LENGTH(11800)}, //XA_Spookeez
	{XA_Week2A, XA_LENGTH(11100)}, //XA_South
	//WEEK2B.XA
	{XA_Week2B, XA_LENGTH(15400)}, //XA_Monster
	{XA_Week2B, XA_LENGTH(11000)}, //XA_Clucked
	//CUT.XA
	{XA_CutA, XA_LENGTH(1600)},
	{XA_CutB, XA_LENGTH(800)},
	//EXTRA.XA
	{XA_Extra, XA_LENGTH(11295)},  //XA_Freeky
	{XA_Extra, XA_LENGTH(1800)},  //XA_Bftdemo
	//CUSTOM.XA
	{XA_Custom, XA_LENGTH(3700)}, //XA_Ludum
	//DIALOG.XA
	{XA_Dialog, XA_LENGTH(4800)}, //XA_Psy
	{XA_Dialog, XA_LENGTH(6400)}, //XA_LoveIsAFlower
	{XA_Dialog, XA_LENGTH(7000)}, //XA_SpiritualUnrest
	{XA_Dialog, XA_LENGTH(2800)}, //XA_Voyage
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
	"\\MUSIC\\WEEK2A.XA;1", //XA_Week2A
	"\\MUSIC\\WEEK2B.XA;1", //XA_Week2B
	"\\MUSIC\\CUTA.XA;1",
	"\\MUSIC\\CUTB.XA;1",
	"\\MUSIC\\EXTRA.XA;1",
	"\\MUSIC\\CUSTOM.XA;1",
	"\\MUSIC\\DIALOG.XA;1",
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"bopeebo", true}, //XA_Bopeebo
	{"fresh", true},   //XA_Fresh
	//WEEK1B.XA
	{"dadbattle", true}, //XA_Dadbattle
	{"tutorial", false}, //XA_Tutorial
	//WEEK2A.XA
	{"spookeez", true}, //XA_Spookeez
	{"south", true},    //XA_South
	//WEEK2B.XA
	{"monster", true}, //XA_Monster
	{"clucked", true}, //XA_Clucked
	//CUT.XA
	{"bftransform", false},
	{"sendai", false},
	//EXTRA.XA
	{"freeky", false},
	{"bftdemo", false},
	//CUSTOM.XA
	{"ludum", false},
	//DIALOG.XA
	{"psy", false},
	{"loveisaflower", false},
	{"spiritualunrest", false},
	{"voyage", false},
	
	{NULL, false}
};
