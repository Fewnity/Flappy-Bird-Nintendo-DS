//Always include this
#include <nds.h>

//math function
#include <math.h>

//Graphic lib
#include <nf_lib.h>

//For random system
#include <time.h>

//Sound system
#include <maxmod9.h>
#include "soundbank.h"
#include "soundbank_bin.h"

//File system
#include <cstdio>

//All defines
#define DefaultBirdxPos 50
#define DefaultBirdyPos 80

#define BirdAnimationTimerCount 10
#define BirdAnimaitonCount 3

#define PipeGfxOffset 26
#define PipeCount 3

#define UiGfxOffset 10
#define MedalsGfxOffset 17

#define ScreenxSize 256
#define GameScreen 1
#define DebugScreen 0

//All structs
struct Pipe
{
	int xPos;
	int yPos;
};

struct VariablesToSave
{
	int HiScore;
} SaveFile;

//Background
byte BackGroundxPos = 0;

//Bird animation
byte BirdModel = 0;
byte CurrentBirdAnimation = 2;
byte BirdAnimationTimer = 0;
bool ReverseAnimation = false;

//Bird other
byte BirdxPos = ScreenxSize / 2 - 8;
byte DieSoundTimer = 0;
float BirdyPos = DefaultBirdyPos;
float BirdySpeed;
float BirdyOffset = 0;

//Pipes
Pipe AllPipes[PipeCount];

//Input
int pressedKey;

//Party
bool Lose = false;
bool WaitToStart = true;
bool FirstParty = true;
int Score = 0;
int HighScore = 0;

//File system
FILE *savefile;

//All functions
void ShowScore();
void ShowScoreInScoreScreen();
void ClearScore(bool Show);
void ShowScoreScreen(bool Show);
void LoseGame();
void DoBirdAnimation();
void DoBackgroundScroll();
void ShowTitle(int Title);
void ResetPipesPosition();
void DoMovePipes();

int main(int argc, char **argv)
{
	//Init random
	srand(time(NULL));

	//Set NightFox game assets folder
	NF_SetRootFolder("NITROFS");

	//Init 2D engine
	NF_Set2D(GameScreen, 0);
	NF_Set2D(DebugScreen, 0);

	//Init sound system
	mmInitDefaultMem((mm_addr)soundbank_bin);

	//Load sound effects
	mmLoadEffect(SFX_DIE);
	mmLoadEffect(SFX_HIT);
	mmLoadEffect(SFX_POINT);
	mmLoadEffect(SFX_SWOOSH);
	mmLoadEffect(SFX_WING);

	//Read high score
	savefile = fopen("fat:/Flappy_Bird_Score.sav", "rb");
	fread(&SaveFile, 1, sizeof(SaveFile), savefile);
	fclose(savefile);

	//Set high score
	HighScore = SaveFile.HiScore;

	//Init backgrounds
	NF_InitTiledBgBuffers();
	NF_InitTiledBgSys(GameScreen);
	NF_InitTiledBgSys(DebugScreen);

	//Load and show bottom scrolling background
	NF_LoadTiledBg("backgrounds/ScrollingBg", "ScrollingBg", 256, 256);
	NF_CreateTiledBg(GameScreen, 2, "ScrollingBg");

	//Load and show trees/city background
	NF_LoadTiledBg("backgrounds/Bg1", "DayBg", 256, 256);
	NF_LoadTiledBg("backgrounds/Bg2", "NightBg", 256, 256);
	NF_CreateTiledBg(GameScreen, 3, "DayBg");

	//Scroll background
	NF_ScrollBg(GameScreen, 3, 0, 70);				  //70 to have trees and city in middle of the screen
	NF_ScrollBg(GameScreen, 2, BackGroundxPos, -150); //-136 for full scrolling bg, -150 for lower bg and better game screen size

	//Init sprite system
	NF_InitSpriteBuffers();
	NF_InitSpriteSys(GameScreen);

	//Load sprites and palettes
	NF_LoadSpriteGfx("sprites/LittleUI", 4, 32, 32);
	NF_LoadSpritePal("palettes/LittleUI", 4);

	NF_LoadSpriteGfx("sprites/ScoreScreenUI", 3, 64, 64);
	NF_LoadSpritePal("palettes/ScoreScreenUI", 3);

	NF_LoadSpriteGfx("sprites/NumberUI", 2, 16, 16);
	NF_LoadSpritePal("palettes/NumberUI", 2);

	NF_LoadSpriteGfx("sprites/Pipes", 1, 32, 64);
	NF_LoadSpritePal("palettes/Pipes", 1);

	NF_LoadSpriteGfx("sprites/Birds", 0, 16, 16);
	NF_LoadSpritePal("palettes/Birds", 0);

	//Set pallettes to sprites
	for (int i = 0; i < 5; i++)
		NF_VramSpritePal(GameScreen, i, i);

	/*GFX LIST :

	0 = bird

	[1;9] = Text ([1;3] in game score text, [4;9] score board texts)

	[UiGfxOffset;UiGfxOffset + 1] = Title (game name, Game Over & Get Ready!)
	UiGfxOffset + 2 = Tap Image
	[UiGfxOffset + 3;UiGfxOffset + 4] = Score screen
	UiGfxOffset + 5 = Play button
	UiGfxOffset + 6 = Copyright

	MedalsGfxOffset = medal
	MedalsGfxOffset + 1 = New high score

	[PipeGfxOffset; PipeGfxOffset + PipeCount * 2] = pipes
	*/

	//Bird GFX and pal
	NF_VramSpriteGfx(GameScreen, 0, 0, true);
	NF_SpriteLayer(GameScreen, 0, 3);

	//Score texts GFX
	for (int i = 0; i < 9; i++)
		NF_VramSpriteGfx(GameScreen, 2, 1 + i, true);

	//Some other UI
	for (int i = 0; i < 7; i++)
		NF_VramSpriteGfx(GameScreen, 3, UiGfxOffset + i, true);

	//Medal GFX & New high score GFX
	for (int i = 0; i < 2; i++)
		NF_VramSpriteGfx(GameScreen, 4, MedalsGfxOffset + i, true);

	//Pipes GFX and pal
	for (int i = 0; i < PipeCount * 2; i++)
		NF_VramSpriteGfx(GameScreen, 1, PipeGfxOffset + i, true);

	//TEXT
	/*NF_InitTextSys(DebugScreen);

	NF_LoadTextFont("fonts/font", "TopScreen", 256, 256, 0);
	NF_CreateTextLayer(DebugScreen, 0, 0, "TopScreen");
	NF_BgSetPalColor(DebugScreen, 0, 71, 31, 31, 31);

	NF_ClearTextLayer(DebugScreen, 0);
	char ScoreText[12];
	sprintf(ScoreText, "Debug:");
	NF_WriteText(DebugScreen, 0, 2, 2, ScoreText);
	NF_UpdateTextLayers();*/

	//Create bird sprite
	NF_CreateSprite(GameScreen, 0, 0, 0, BirdxPos, BirdyPos);
	NF_EnableSpriteRotScale(GameScreen, 0, 0, false);
	NF_SpriteFrame(GameScreen, 0, 2 + BirdModel * 3);

	//Create score text sprites
	for (int i = 0; i < 3; i++)
	{
		NF_CreateSprite(GameScreen, 1 + i, 1 + i, 2, 128 - 16, 40);
		NF_EnableSpriteRotScale(GameScreen, 1 + i, 1 + i, true);
		NF_SpriteRotScale(GameScreen, 1 + i, 0, 360, 360);

		if (i != 0)
			NF_ShowSprite(GameScreen, 1 + i, false);
	}

	//Create screen score text sprites
	for (int i = 0; i < 3; i++)
	{
		NF_CreateSprite(GameScreen, 4 + i, 4 + i, 2, 162 - i * 8, 78);
		NF_SpriteLayer(GameScreen, 4 + i, 0);
		NF_SpriteFrame(GameScreen, 4 + i, 10);
	}
	for (int i = 0; i < 3; i++)
	{
		NF_CreateSprite(GameScreen, 7 + i, 7 + i, 2, 162 - i * 8, 98);
		NF_SpriteLayer(GameScreen, 7 + i, 0);
		NF_SpriteFrame(GameScreen, 7 + i, 10);
	}

	//Create title sprites
	NF_CreateSprite(GameScreen, UiGfxOffset + 0, UiGfxOffset + 0, 3, 128 - 64, 20);
	NF_CreateSprite(GameScreen, UiGfxOffset + 1, UiGfxOffset + 1, 3, 128, 20);
	for (int i = 0; i < 2; i++)
	{
		NF_SpriteFrame(GameScreen, UiGfxOffset + i, 6 + i);
		NF_SpriteLayer(GameScreen, UiGfxOffset + i, 1);
	}

	//Create tap image sprite
	NF_CreateSprite(GameScreen, UiGfxOffset + 2, UiGfxOffset + 2, 3, 128 - 32, 60);
	NF_SpriteFrame(GameScreen, UiGfxOffset + 2, 8);
	NF_SpriteLayer(GameScreen, UiGfxOffset + 2, 1);
	NF_ShowSprite(GameScreen, UiGfxOffset + 2, false);

	//Create score screen sprites
	NF_CreateSprite(GameScreen, UiGfxOffset + 3, UiGfxOffset + 3, 3, 128 - 64, 60);
	NF_CreateSprite(GameScreen, UiGfxOffset + 4, UiGfxOffset + 4, 3, 128, 60);
	NF_SpriteFrame(GameScreen, UiGfxOffset + 4, 1);
	NF_SpriteLayer(GameScreen, UiGfxOffset + 3, 1);
	NF_SpriteLayer(GameScreen, UiGfxOffset + 4, 1);

	//Create play Button sprite
	NF_CreateSprite(GameScreen, UiGfxOffset + 5, UiGfxOffset + 5, 3, 128 - 32, 92);
	NF_SpriteFrame(GameScreen, UiGfxOffset + 5, 9);

	//Create copyright Button sprite
	NF_CreateSprite(GameScreen, UiGfxOffset + 6, UiGfxOffset + 6, 3, 128 - 32, 144);
	NF_SpriteFrame(GameScreen, UiGfxOffset + 6, 10);

	//Create medal sprite
	NF_CreateSprite(GameScreen, MedalsGfxOffset, MedalsGfxOffset, 4, 80, 80);
	NF_ShowSprite(GameScreen, MedalsGfxOffset, false);

	//Create new high score sprite
	NF_CreateSprite(GameScreen, MedalsGfxOffset + 1, MedalsGfxOffset + 1, 4, 128, 81);
	NF_SpriteFrame(GameScreen, MedalsGfxOffset + 1, 4);
	NF_ShowSprite(GameScreen, MedalsGfxOffset + 1, false);

	ResetPipesPosition();

	//Create pipes sprites
	for (int i = 0; i < PipeCount; i++)
	{
		//SPRITES POSITION
		NF_CreateSprite(GameScreen, PipeGfxOffset + i * 2, PipeGfxOffset + i * 2, 1, AllPipes[i].xPos, AllPipes[i].yPos);
		NF_CreateSprite(GameScreen, PipeGfxOffset + 1 + i * 2, PipeGfxOffset + 1 + i * 2, 1, AllPipes[i].xPos, AllPipes[i].yPos - 120);

		NF_SpriteFrame(GameScreen, PipeGfxOffset + 1 + i * 2, 3);

		NF_SpriteLayer(GameScreen, PipeGfxOffset + i * 2, 3);
		NF_SpriteLayer(GameScreen, PipeGfxOffset + 1 + i * 2, 3);
	}

	//Hide score and score screen
	ShowScoreScreen(false);
	ClearScore(false);

	/*while (1)
	{
		NF_SpriteOamSet(GameScreen);
		oamUpdate(&oamSub);
		swiWaitForVBlank();
	}*/

	while (1)
	{
		//Get key input
		scanKeys();
		pressedKey = keysDown(); // keys pressed this loop
		touchPosition touch;
		touchRead(&touch);

		//If game is waiting to start a new game
		if (WaitToStart)
		{
			DoBirdAnimation();
			DoBackgroundScroll();

			//Bird up and down waving animation
			BirdyOffset += 0.1f;
			if (BirdyOffset >= 3.1f * 2)
				BirdyOffset = 0;

			NF_MoveSprite(GameScreen, 0, BirdxPos, BirdyPos + sinf(BirdyOffset) * 5);

			//If this is his first party
			if (FirstParty)
			{
				//Check touch screen input position for clicking play button
				if ((touch.px <= 128 + 32 - 8 && touch.px >= 128 - 32 + 8 && touch.py <= 92 + 32 + 16 && touch.py >= 92 + 16) || (pressedKey != 0 && (pressedKey & KEY_TOUCH) != KEY_TOUCH))
				{
					FirstParty = false;
					BirdxPos = DefaultBirdxPos;
					//Hide play button and
					NF_ShowSprite(GameScreen, UiGfxOffset + 5, false);
					NF_ShowSprite(GameScreen, UiGfxOffset + 6, false);
					//Show Get Ready! screen
					ShowTitle(0);
					//Play Swoosh sound effect
					mmEffect(SFX_SWOOSH);
					//Change bird color
					BirdModel = rand() % 3;
					NF_SpriteFrame(GameScreen, 0, CurrentBirdAnimation + BirdModel * 3);
				}
			}
			else if (pressedKey != 0)
			{
				WaitToStart = false;
				BirdxPos = DefaultBirdxPos;
				//Add force to bird
				BirdySpeed = -3.2;
				//Show score to 0
				ClearScore(true);
				ShowScore();
				//Hide Get ready
				ShowTitle(-1);
				//Play wing sound effect
				mmEffect(SFX_WING);
			}
		}
		else if (Lose)
		{
			//Continue adding bird gravity
			BirdyPos += BirdySpeed;

			if (BirdyPos >= 138)
				BirdyPos = 138;
			else if (BirdySpeed < 4)
				BirdySpeed += 0.14;

			NF_MoveSprite(GameScreen, 0, BirdxPos, BirdyPos);
			NF_SpriteRotScale(GameScreen, 0, BirdySpeed * 31.5, 256, 256);

			//Check input to show Get ready! screen
			if (pressedKey != 0)
			{
				//Reset var
				Lose = false;
				Score = 0;
				BirdyPos = DefaultBirdyPos;
				BirdySpeed = 0;
				WaitToStart = true;
				//Change bird color
				BirdModel = rand() % 3;
				NF_SpriteFrame(GameScreen, 0, CurrentBirdAnimation + BirdModel * 3);
				NF_SpriteRotScale(GameScreen, 0, 0, 256, 256);
				//Hide score screen
				ShowScoreScreen(false);
				ClearScore(false);
				//Show Get Ready! screen
				ShowTitle(0);
				ResetPipesPosition();
				DoMovePipes();
				mmEffect(SFX_SWOOSH);

				//Change background
				if (rand() % 4 != 0) //More chance for day background
				{
					NF_DeleteTiledBg(GameScreen, 3);
					NF_CreateTiledBg(GameScreen, 3, "DayBg");
				}
				else
				{
					NF_DeleteTiledBg(GameScreen, 3);
					NF_CreateTiledBg(GameScreen, 3, "NightBg");
				}
				//Set background position
				NF_ScrollBg(GameScreen, 3, 0, 70);
			}

			//Wait before play die sound effect
			if (DieSoundTimer > 0)
			{
				DieSoundTimer--;

				if (DieSoundTimer == 1)
					mmEffect(SFX_DIE);
			}
		}
		else
		{
			//Move pipes
			DoMovePipes();

			//Background scroll
			DoBackgroundScroll();

			//Bird jump
			if (pressedKey != 0)
			{
				BirdySpeed = -3.1;
				mmEffect(SFX_WING);
			}

			//Add gravity to bird
			if (BirdySpeed < 4)
				BirdySpeed += 0.14;

			BirdyPos += BirdySpeed;

			//Check ground and top collision for bird
			if (BirdyPos >= 138)
			{
				BirdyPos = 138;
				LoseGame();
			}
			else if (BirdyPos <= -8)
			{
				LoseGame();
				DieSoundTimer = 12;
			}

			//Set bird position
			NF_MoveSprite(GameScreen, 0, BirdxPos, BirdyPos);
			NF_SpriteRotScale(GameScreen, 0, BirdySpeed * 31.5, 256, 256);

			//Bird animation
			DoBirdAnimation();

			//Collision
			for (int i = 0; i < PipeCount; i++)
				if (AllPipes[i].xPos - 11 <= BirdxPos && AllPipes[i].xPos + 26 >= BirdxPos)
					if (AllPipes[i].yPos <= BirdyPos + 16 || AllPipes[i].yPos >= BirdyPos + 57)
					{
						LoseGame();
						DieSoundTimer = 12;
					}

			//Add score
			for (int i = 0; i < PipeCount; i++)
				if (AllPipes[i].xPos + 16 == BirdxPos)
				{
					Score++;
					ShowScore();
					mmEffect(SFX_POINT);
				}
		}

		NF_SpriteOamSet(GameScreen);
		//oamUpdate(&oamMain);
		oamUpdate(&oamSub);
		swiWaitForVBlank();
	}

	return 0;
}

void DoMovePipes()
{
	for (int i = 0; i < PipeCount; i++)
	{
		AllPipes[i].xPos--;

		//If pipes are not offscreen (left)
		if (AllPipes[i].xPos <= 256)
		{
			//If pipes are offscreen (right)
			if (AllPipes[i].xPos <= -32)
			{
				//Reset pipes positon
				AllPipes[i].xPos = 256;
				AllPipes[i].yPos = 86 + rand() % 34; //Add some random
			}

			//Set pipes position
			NF_MoveSprite(GameScreen, PipeGfxOffset + i * 2, AllPipes[i].xPos, AllPipes[i].yPos);
			NF_MoveSprite(GameScreen, PipeGfxOffset + 1 + i * 2, AllPipes[i].xPos, AllPipes[i].yPos - 120);
		}
		else
		{
			//Set pipes position to off screen
			NF_MoveSprite(GameScreen, PipeGfxOffset + i * 2, 256, 0);
			NF_MoveSprite(GameScreen, PipeGfxOffset + 1 + i * 2, 256, 0);
		}
	}
}

void DoBackgroundScroll()
{
	BackGroundxPos++;

	//Reset background x position on full scroll cycle
	if (BackGroundxPos == 255)
		BackGroundxPos = 0;

	NF_ScrollBg(GameScreen, 2, BackGroundxPos, -150); //-136 for full bg, -150 for better game screen size
}

void DoBirdAnimation()
{
	BirdAnimationTimer++;
	//Wait before change bird sprite
	if (BirdAnimationTimer == BirdAnimationTimerCount)
	{
		BirdAnimationTimer = 0;

		//Check if animation ise reversed
		if (ReverseAnimation)
			CurrentBirdAnimation++;
		else
			CurrentBirdAnimation--;

		NF_SpriteFrame(GameScreen, 0, CurrentBirdAnimation + BirdModel * 3);

		if ((CurrentBirdAnimation == BirdAnimaitonCount - 1) || (CurrentBirdAnimation == 0))
			ReverseAnimation = !ReverseAnimation;
	}
}

void ShowScore()
{
	if (Score < 10) //If Score < 10, use one sprite
		NF_SpriteFrame(GameScreen, 1, Score);
	else if (Score < 100) //If Score < 100, use two sprites
	{
		//Show sprite
		NF_ShowSprite(GameScreen, 2, true);

		//Set sprites frame
		NF_SpriteFrame(GameScreen, 1, Score / 10);
		NF_SpriteFrame(GameScreen, 2, Score % 10);

		//Set sprites position
		NF_MoveSprite(GameScreen, 1, 128 - 16 - 11, 40);
		NF_MoveSprite(GameScreen, 2, 128 - 16 + 11, 40);
	}
	else if (Score < 1000) //If Score < 1000, use three sprites and If score is more than 999, score at screen will be 999
	{
		//Show sprites
		NF_ShowSprite(GameScreen, 2, true);
		NF_ShowSprite(GameScreen, 3, true);

		//Set sprites frame
		NF_SpriteFrame(GameScreen, 1, Score / 100);
		NF_SpriteFrame(GameScreen, 2, Score % 100 / 10);
		NF_SpriteFrame(GameScreen, 3, Score % 10);

		//Set sprites position
		NF_MoveSprite(GameScreen, 1, 128 - 16 - 22, 40);
		NF_MoveSprite(GameScreen, 2, 128 - 16, 40);
		NF_MoveSprite(GameScreen, 3, 128 - 16 + 22, 40);
	}
}

void ShowScoreInScoreScreen()
{
	for (int i = 0; i < 2; i++)
	{
		int ScoreToShow = 0;

		//First loop is for score and second loop is for high score
		if (i == 0)
			ScoreToShow = Score;
		else
			ScoreToShow = HighScore;

		if (ScoreToShow < 10) //If Score < 10, use one sprite
			NF_SpriteFrame(GameScreen, 4 + i * 3, 10 + ScoreToShow);
		else if (ScoreToShow < 100) //If Score < 100, use two sprites
		{
			//Show sprite
			NF_ShowSprite(GameScreen, 5 + i * 3, true);

			//Set sprites frame
			NF_SpriteFrame(GameScreen, 4 + i * 3, 10 + ScoreToShow % 10);
			NF_SpriteFrame(GameScreen, 5 + i * 3, 10 + ScoreToShow / 10);
		}
		else if (ScoreToShow < 1000) //If Score < 1000, use three sprites and If score is more than 999, score at screen will be 999
		{
			//Show sprites
			NF_ShowSprite(GameScreen, 5 + i * 3, true);
			NF_ShowSprite(GameScreen, 6 + i * 3, true);

			//Set sprites frame
			NF_SpriteFrame(GameScreen, 4 + i * 3, 10 + ScoreToShow % 10);
			NF_SpriteFrame(GameScreen, 5 + i * 3, 10 + ScoreToShow % 100 / 10);
			NF_SpriteFrame(GameScreen, 6 + i * 3, 10 + ScoreToShow / 100);
		}
	}

	//Show medal
	NF_ShowSprite(GameScreen, MedalsGfxOffset, true);
	if (Score >= 40)
		NF_SpriteFrame(GameScreen, MedalsGfxOffset, 3);
	else if (Score >= 30)
		NF_SpriteFrame(GameScreen, MedalsGfxOffset, 2);
	else if (Score >= 20)
		NF_SpriteFrame(GameScreen, MedalsGfxOffset, 1);
	else if (Score >= 10)
		NF_SpriteFrame(GameScreen, MedalsGfxOffset, 0);
	else //Or hide medal
		NF_ShowSprite(GameScreen, MedalsGfxOffset, false);
}

void ClearScore(bool Show)
{
	//Hide some score sprite and reset first sprite position
	NF_MoveSprite(GameScreen, 1, 128 - 16, 40);
	NF_ShowSprite(GameScreen, 1, Show);
	NF_ShowSprite(GameScreen, 2, false);
	NF_ShowSprite(GameScreen, 3, false);
}

void ShowScoreScreen(bool Show)
{
	//Show or hide
	NF_ShowSprite(GameScreen, UiGfxOffset + 3, Show);
	NF_ShowSprite(GameScreen, UiGfxOffset + 4, Show);
	NF_ShowSprite(GameScreen, MedalsGfxOffset, false);

	if (!Show)
		NF_ShowSprite(GameScreen, MedalsGfxOffset + 1, false);

	for (int i = 0; i < 2; i++)
	{
		NF_ShowSprite(GameScreen, 4 + i * 3, Show);
		NF_ShowSprite(GameScreen, 5 + i * 3, false);
		NF_ShowSprite(GameScreen, 6 + i * 3, false);
	}
}

void LoseGame()
{
	//Save high score
	if (Score > HighScore)
	{
		//Show new high score sprite
		NF_ShowSprite(GameScreen, MedalsGfxOffset + 1, true);
		HighScore = Score;
		SaveFile.HiScore = HighScore;
		savefile = fopen("fat:/Flappy_Bird_Score.sav", "wb");
		fwrite(&SaveFile, 1, sizeof(SaveFile), savefile);
		fclose(savefile);
	}
	else
		NF_ShowSprite(GameScreen, MedalsGfxOffset + 1, false); //Hide new high score sprite

	Lose = true;
	ShowScoreScreen(true);
	ShowScoreInScoreScreen();
	ClearScore(false);
	//Show game over screen
	ShowTitle(1);
	//Play hit sound effect
	mmEffect(SFX_HIT);
}

void ResetPipesPosition()
{
	for (int i = 0; i < PipeCount; i++)
	{
		AllPipes[i].xPos = 256 + i * (256 + 32) / PipeCount;
		AllPipes[i].yPos = 86 + rand() % 34;
	}
}

void ShowTitle(int Title)
{
	//Hide or show title
	NF_ShowSprite(GameScreen, UiGfxOffset, Title != -1);
	NF_ShowSprite(GameScreen, UiGfxOffset + 1, Title != -1);

	if (Title == 0)
	{
		//Show Get ready! screen
		NF_SpriteFrame(GameScreen, UiGfxOffset, 2);
		NF_SpriteFrame(GameScreen, UiGfxOffset + 1, 3);
		NF_MoveSprite(GameScreen, UiGfxOffset, 128 - 64, -6);
		NF_MoveSprite(GameScreen, UiGfxOffset + 1, 128, -6);
		NF_ShowSprite(GameScreen, UiGfxOffset + 2, true);
	}
	else if (Title == 1)
	{
		//Show game over screen
		NF_SpriteFrame(GameScreen, UiGfxOffset, 4);
		NF_SpriteFrame(GameScreen, UiGfxOffset + 1, 5);
		NF_MoveSprite(GameScreen, UiGfxOffset, 128 - 64, 14);
		NF_MoveSprite(GameScreen, UiGfxOffset + 1, 128, 14);
	}
	else
		NF_ShowSprite(GameScreen, UiGfxOffset + 2, false); //Hide tap image
}
