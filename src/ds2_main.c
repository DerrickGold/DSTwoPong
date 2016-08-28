//ds2_main.c

#include <BAGLib.h>

#define VERSION 1.01

#define StartBallSpeed 2
#define MaxBallSpeed 16
#define MaxBallVSpeed 8
#define MaxPaddleSpeed 4
#define MinReaction 150000
#define MaxReaction 230000
#define HitsToSpeedUp 6

typedef enum
{
	Top_Screen,
	Bottom_Screen,
}ScreenInfos;

struct Ball_Sprite
{
	struct BMPOBJ Gfx;
	struct SndFile Snd;
	short X,Y;
	short VX,VY;
	short Time, FinalY, Hits, Angle,Speed, half, VYcount, lastVY;
};

struct Paddle_Sprite
{
	//resources
	struct BMPOBJ Gfx;
	struct SndFile Snd;
	
	short X,Y;
	long ReactionTime;
	short VX,VY, Speed, Reaction;
	short Timer;
	char is_Bot, player, dir;
};


	
//sprites
struct Ball_Sprite Ball;
struct Paddle_Sprite Player1, AI;
//graphics
struct BMPOBJ  BackGround, ScoreNums[2], Win, Lose;
//sounds
struct SndFile wall, BGM;
//font
struct FT_FONT Menu_fnt;

//keeps track of the score
char Score[2], Enable_BGM = 0, Collided[2], Skin_Loaded = 0, Skin_Selected = 0;

struct Directory{
	char Name[512][256];
	int Num_Of_Folds;
};

//for skins folders
struct Directory Skins;
char PongDir[256];

void Get_Data_dir(void)
{
	FILE *testopen;
	
	testopen = fopen("/_dstwoplug/DS2Pong.plg", "r");
	if(testopen)
	{
		fclose(testopen);
		DIR * dir = opendir("/_dstwoplug/ds2pong/");
		if(dir)
		{
			closedir(dir);
			sprintf(PongDir, "%s","/_dstwoplug/ds2pong/");
			return;
		}
	}
	
	testopen = fopen("/_Imenu/_plg/DS2Pong.plg", "r");
	if(testopen)
	{
		fclose(testopen);
		DIR * dir = opendir("/_Imenu/_plg/ds2pong/");
		if(dir)
		{
			closedir(dir);
			sprintf(PongDir, "%s","/_Imenu/_plg/ds2pong/");
			return;
		}
	}	
	
	Debug_Sys_Msg("failed to locate DS2Pong folder");
}


/*===============================================================
	General Code
===============================================================*/
//Generates a random value between 2 values
static inline unsigned int RandMinMax(unsigned int min,unsigned int max){
	return ((rand()%((max + 1)-min)) + min);
} 


//ball and paddle collision
char Ball_Col(struct Paddle_Sprite  *paddle, struct Ball_Sprite *ball)
{
  short w1=paddle->Gfx.FrameWd;
  short h1= paddle->Gfx.FrameHt;
  short x1= paddle->X + (paddle->Gfx.FrameWd>>1);
  short y1= paddle->Y + (paddle->Gfx.FrameHt>>1);
  
  short w2= ball->Gfx.FrameWd;
  short h2= ball->Gfx.FrameHt;
  short x2= ball->X + (ball->Gfx.FrameWd>>1);
  short y2 = ball->Y + (ball->Gfx.FrameHt>>1);
  
  return ((x2 >= x1 - ((w1 + w2)>>1)) && (x2 <= x1 + ((w1 + w2)>>1)) && (y2 >= y1 - ((h1 + h2)>>1)) && (y2 <= y1 + ((h1 + h2)>>1)));
}

/*===============================================================
	AI Code
===============================================================*/
void Calculate_AISpeed(struct Paddle_Sprite *ai);

void AI_Reaction(struct Paddle_Sprite *ai)
{
	if(ai->is_Bot)
	{
		ai->Reaction=1;
		Calculate_AISpeed(ai);
	}
}

void Reset_AI_Reaction(struct Paddle_Sprite *ai)
{
	stopTimer(ai->Timer);
	if(ai->is_Bot)
	{
		resetTimer(ai->Timer);
		ai->Reaction = 0;
	}
}

void Run_AI_Reaction(struct Paddle_Sprite *ai)
{
	if(ai->is_Bot)
	{
		ai->ReactionTime = RandMinMax(MinReaction,MaxReaction);
		initTimer(ai->Timer,ai->ReactionTime , AI_Reaction, ai);
		runTimer(ai->Timer);
	}
}

char Check_AI_Speed(struct Paddle_Sprite *ai, char speed,char dir)
{
	if(ai->is_Bot)
	{
		if(ai->player == 1)
			Ball.Time = abs(Ball.X - (ai->X + ai->Gfx.FrameWd)) / abs(Ball.VX);
		else if(ai->player == 2)
			Ball.Time = abs((SCREEN_WIDTH -(ai->X + ai->Gfx.FrameWd)) - Ball.X+Ball.Gfx.FrameWd) / abs(Ball.VX);
		
		if(Ball.Time <= 0) 
			Ball.Time = 1;
		switch(dir)
		{
			default: break;
			case -1: //up
				 if(ai->Y  - (Ball.Time*speed) <= Ball.FinalY)		
					return 1;
			break;
			case 1://down
				 if(ai->Y + ai->Gfx.FrameHt  + (Ball.Time*speed) >= Ball.FinalY+ Ball.Gfx.FrameHt)
					return 1;
			break;
		}
	}
	return 0;
}

void Update_AI_Speed ( struct Paddle_Sprite *ai)
{
	if(ai->is_Bot)
	{
		int i=0;
		ai->Speed = 0;
		for(i=0; i< MaxPaddleSpeed; i++)
		{
			if(Check_AI_Speed(ai, i, ai->dir))
			{
				ai->Speed = i;
				break;
			}
		}	
		if(i == MaxPaddleSpeed)
			ai->Speed = i;	
	}
}


void Calculate_AISpeed(struct Paddle_Sprite *ai)
{
	if(ai->is_Bot)
	{
		int i = 0;
		ai->dir = 0;
		//calculate position percentage
		int hitpercent = RandMinMax(0,100);
		int finalHP = (hitpercent<<8)/100;
		//now to get the final y position based on its velocity
		//find out what y value the ball will be at at the edge of the screen

		int TempTime = 0, TempY = Ball.Y, TempX = Ball.X, TempVY = Ball.VY;
		while (TempX > 16 && TempX < SCREEN_WIDTH - 16 - Ball.Gfx.FrameWd)
		{
			TempY +=TempVY;
			if(TempY<=0)
				TempVY = -TempVY;
			else if(TempY >= SCREEN_HEIGHT)
				TempVY = -TempVY;
			
			TempX +=Ball.VX;
			TempTime++;
		}
		
		Ball.FinalY = TempY;
		
		if(Ball.FinalY  <  ai->Y)
			ai->dir = -1;
		if(Ball.FinalY + Ball.Gfx.FrameHt  > ai->Y + ai->Gfx.FrameHt)
			ai->dir = 1;	
			
			
		Ball.FinalY = TempY + ( ai->Gfx.FrameHt *(finalHP>>8) * ai->dir);	
			
		Update_AI_Speed (ai);
	}
}



void CPUControls(struct Paddle_Sprite *ai)
{
	bool MovingTowards = 0;
	//if ball is moving toward the AI
	if(ai->is_Bot)
	{
		switch(ai->player)
		{
			case 1:
				if(Ball.VX < 0 && ai->Reaction)
					MovingTowards= 1;		
			break;
			case 2:
				if(Ball.VX > 0 && ai->Reaction)
					MovingTowards= 1;
			break;
		}
		
		if(MovingTowards)
		{
			if(Ball.FinalY < ai->Y)
			{
				ai->Y -= ai->Speed;
				ai->VY = -ai->Speed;	
			}
			else if(Ball.FinalY > ai->Y + ai->Gfx.FrameHt)
			{
				ai->Y += ai->Speed;
				ai->VY = ai->Speed;	
			}
		}
		//ball is not moving towards AI, so re-center the paddle
		else
		{
			if(ai->Y+16 < SCREEN_HEIGHT>>1)
				ai->Y++;
			else if(ai->Y + 16 > SCREEN_HEIGHT>>1)
				ai->Y--;
			else
				ai->VY = 0;
		}

		//prevent cpu from going off screen
		if(ai->Y < 0) 
		{
			ai->Y = 0;
			ai->VY = 0;
		}
		if(ai->Y + ai->Gfx.FrameHt > SCREEN_HEIGHT) 
		{
			ai->Y = SCREEN_HEIGHT - ai->Gfx.FrameHt;
			ai->VY = 0;
		}	
		Update_AI_Speed (ai);
	}
}

/*===============================================================
	Player code
===============================================================*/
void PlayerOneControls(struct Paddle_Sprite *p1)
{
	//set the actual player position
	p1->Y += (Pad.Held.Down - Pad.Held.Up) * p1->Speed;
	//set the velocity of the player
	p1->VY = (Pad.Held.Down - Pad.Held.Up) * p1->Speed;
	
	if(p1->Y < 0) 
	{
		p1->Y = 0;
		p1->VY = 0;
	}
	if(p1->Y + p1->Gfx.FrameHt > SCREEN_HEIGHT) 
	{
		p1->Y = SCREEN_HEIGHT - p1->Gfx.FrameHt;
		p1->VY = 0;
	}	
	
}


/*===============================================================
	Game Code
===============================================================*/
//Reset the ball and send it in a random direction
void ResetBall(struct Paddle_Sprite *p1, struct Paddle_Sprite *p2)
{
	Ball.X=(256>>1) - (Ball.Gfx.FrameWd>>1);
	Ball.Y=(192>>1) - (Ball.Gfx.FrameHt>>1);
	Ball.Angle = 0;
	Ball.VY = 0;
	Ball.VX = 0;
	Ball.VYcount = 0;
	Collided[0] = 0;
	Collided[1] = 0;
	p1->Reaction = 0;
	p2->Reaction = 0;
	Reset_AI_Reaction(p1);
	Reset_AI_Reaction(p2);
	Ball.Hits = 0;
	Ball.Speed= StartBallSpeed;//ball goes up
	while(Ball.VY == 0 || Ball.VX == 0)
	{
		Ball.Angle = rand() % 512;
	
		//reset speed and velocity
		Ball.VX = -(_Cos(Ball.Angle)*Ball.Speed)>>8;
		Ball.VY = -(_Sin(Ball.Angle)*Ball.Speed)>>8;	
	}
	
	//ball is heading to AI so start the reaction timing
	if(Ball.VX > 0)
	{
		if(p2->is_Bot)
		{
			Calculate_AISpeed(p2);
			Run_AI_Reaction(p2);
		}
	}
	else
	{
		if(p1->is_Bot)
		{
			Calculate_AISpeed(p1);
			Run_AI_Reaction(p1);
		}
		if(p2->is_Bot)
			Reset_AI_Reaction(p2);
	}
}

void Paddle_Collision(struct Paddle_Sprite *p)
{
	Stop_Wave(&p->Snd);
	Play_Wave(&p->Snd,  p->player - 1);
	
	Ball.Angle = GetAngle(Ball.Gfx.X+(Ball.Gfx.FrameWd>>1) ,Ball.Gfx.Y+(Ball.Gfx.FrameHt>>1),p->Gfx.X+(p->Gfx.FrameWd>>1), p->Gfx.Y+(p->Gfx.FrameHt>>1)); 
	Ball.VX = -(_Cos(Ball.Angle)*Ball.Speed)>>8;
	Ball.VY += p->VY>>1;
	if(abs(Ball.VX) < StartBallSpeed)
		Ball.VX = Ball.Speed;
		
	if(p->player == 1)
	{
		Collided[0]++;
		Collided[1]=0;
	}
	else
	{
		Collided[1]++;
		Collided[0]=0;	 
	}
	
	Ball.Hits++;
	
	if(Ball.VY != Ball.lastVY)
	{
		Ball.lastVY = Ball.VY;
		Ball.VYcount = 0;
	}
	else
		Ball.VYcount++;
		
	if(Ball.VYcount > 4)
	{
		Ball.VY = 1;
		Ball.VYcount = 0;
	}
}


void BallCollision(struct Paddle_Sprite * p1, struct Paddle_Sprite * p2)
{
	//if colliding with player 1
	if(Ball_Col(p1, &Ball) && Collided[0] == 0)
	{
		if(Ball.X  + Ball.Gfx.FrameWd>= (p1->X + p1->Gfx.FrameWd))
		{
			Paddle_Collision(p1);
				 //set AI timer
			if(p2->is_Bot)
				Run_AI_Reaction(p2);
			if(p1->is_Bot)
				Reset_AI_Reaction(p1);			
		}
	}
	//colliding with player 2 paddel
	else if(Ball_Col(p2, &Ball) &&  Collided[1] == 0)
	{
		if(Ball.X <= (p2->X ))
		{
			Paddle_Collision(p1);
			if(p1->is_Bot)
				Run_AI_Reaction(p1);
			if(p2->is_Bot)
				Reset_AI_Reaction(p2);
		}
	}
	
	if(Ball.VY > MaxBallVSpeed ) Ball.VY = MaxBallVSpeed;
	else if(Ball.VY < -MaxBallVSpeed) Ball.VY = -MaxBallVSpeed;	
}	

//Uodate ball movement
void UpdateBall(struct Paddle_Sprite *p1, struct Paddle_Sprite *p2)
{
	
	Ball.Y+=Ball.VY;
	Ball.X+=Ball.VX;
	//player scores
	if(Ball.X<=0)
	{
		Score[1]++;
		Set_Bmp_Frame(&ScoreNums[1], FRAME_VERT, Score[1]);
		ResetBall(p1,p2);
	}
	else if( Ball.X + Ball.Gfx.FrameWd >= SCREEN_WIDTH)
	{
		Score[0]++;
		Set_Bmp_Frame(&ScoreNums[0], FRAME_VERT, Score[0]);
		ResetBall(p1,p2);
	}
	
	if(Ball.Y<=0)
	{
		Stop_Wave(&wall);
		Play_Wave(&wall, 2);
		Ball.VY= -Ball.VY;
		Ball.Y= 1;
		if(Ball.VX > 0 && p2->is_Bot)
		{
			Reset_AI_Reaction(p2);
			Run_AI_Reaction(p2);
		}
		else if(Ball.VX < 0 && p1->is_Bot)
		{
			Reset_AI_Reaction(p1);
			Run_AI_Reaction(p1);
		}
	}
	else if(Ball.Y+Ball.Gfx.Height>=SCREEN_HEIGHT)
	{
		Stop_Wave(&wall);
		Play_Wave(&wall, 2);
		Ball.VY= -Ball.VY;
		Ball.Y=SCREEN_HEIGHT-1-Ball.Gfx.FrameHt ;
		
		if(Ball.VX > 0 && p2->is_Bot)
		{
			Reset_AI_Reaction(p2);
			Run_AI_Reaction(p2);
		}
		else if(Ball.VX < 0 && p1->is_Bot)
		{
			Reset_AI_Reaction(p1);
			Run_AI_Reaction(p1);
		}	
	}

	if(Ball.Hits >= HitsToSpeedUp)
	{
		Ball.Hits = 0;
		if(Ball.Speed < MaxBallSpeed)
			Ball.Speed++;
	}
	
	BallCollision(p1,p2);
	
}

void Render_Gfx(enum SCREEN_ID screen, bool Menu)
{
	unsigned short *dest = NULL;
	if(screen == UP_SCREEN)
		dest = (short*)up_screen_addr;
		
	else if(screen == DOWN_SCREEN)
		dest = (short*)down_screen_addr;
		
	ds2_clearScreen(screen, RGB15(0,0,0));
	
	
	Draw_FastBmp_Obj(&BackGround,dest, 0, 0);
	Draw_Bmp_Obj(&Ball.Gfx,dest, Ball.X, Ball.Y);
	Draw_Bmp_Obj(&Player1.Gfx, dest, Player1.X, Player1.Y);
	Draw_Bmp_Obj(&AI.Gfx, dest, AI.X, AI.Y);
	
	//score
	if(!Menu)
	{
		Draw_Bmp_Obj(&ScoreNums[0], dest, 95, 1);
		Draw_Bmp_Obj(&ScoreNums[1], dest, 149, 1);
	}
	else
	{
		Font_OutputText(up_screen_addr,0,0,256,192,"DS2 Pong",FT_FIXED(40),&Menu_fnt,RGB16(0,0,31));
		Font_OutputText(up_screen_addr,0,40,256,192,"By BassAceGold",FT_FIXED(20),&Menu_fnt,RGB16(0,0,31));
	}

	ds2_flipScreen(screen, 1);
}

void Set_Paddle(struct Paddle_Sprite *paddle, char player, char bot)
{

	paddle->X = 0;
	paddle->Y = 0;
	paddle->ReactionTime = 0;
	paddle->VY = 0;
	paddle->VX = 0;
	paddle->Speed = 0;
	paddle->Reaction = 0;
	paddle->Timer = 0;
	paddle->is_Bot = 0;
	paddle->player = 0;
	paddle->dir = 0;
	
	switch(player)
	{
		case 1:
			paddle->X = 8;
			paddle->Timer = 0;
		break;
		case 2:
			paddle->X = SCREEN_WIDTH - paddle->Gfx.FrameWd -8;
			paddle->Timer = 1;
		break;
	}
	paddle->player = player;
	paddle->Y = (SCREEN_HEIGHT>>1)  - (paddle->Gfx.Height>>1);
	paddle->Speed = MaxPaddleSpeed;
	paddle->is_Bot = bot;
}
	
void Main_Menu(void);

void Game_Menu(bool pause, bool end);

char Game_Over(void)
{
	if(Score[0] > Score[1])
		Draw_Bmp_Obj(&Win,up_screen_addr, 0, 0);
	else
		Draw_Bmp_Obj(&Lose,up_screen_addr, 0, 0);
	
	ds2_flipScreen(UP_SCREEN, 1);
	Game_Menu(0, 1);
	
	
	bool end =1;
	while(end)
	{
		//restart
		if(Stylus.X  >= 35 && Stylus.X <= 115 && Stylus.Y >= 75 && Stylus.Y <= 110)
		{
			mdelay(30);
			UpdatePad();
			return 1;
		}
		//menu
		if(Stylus.X  >= 141 && Stylus.X <= 221 && Stylus.Y >= 75 && Stylus.Y <= 110)
		{
			mdelay(30);
			UpdatePad();
			return 0;
		}
			
		//screen shot
		if(Stylus.X  >= 75 && Stylus.X <= 221 && Stylus.Y >= 122 && Stylus.Y <= 155)
		{
			Game_Menu(0, 1);
			if(Score[0] > Score[1])
				Draw_Bmp_Obj(&Win,up_screen_addr, 0, 0);
			else
				Draw_Bmp_Obj(&Lose,up_screen_addr, 0, 0);			
			
			Screen_Cap(DUAL_SCREEN, "/");
		}		
	
		UpdatePad();
	}
	return 0;
}


void Game_Loop(void)
{
	RESET:
	//reset paddle and ball positions
	
	//set player 1 paddels position
	Set_Paddle(&Player1, 1, 0);
	
	//set player 2 paddels position
	Set_Paddle(&AI, 2, 1);
	
	ResetBall(&Player1, &AI);
	
	Score[0] = 0;
	Score[1] = 0;
	bool Game_Start = 1;
	bool Pause = 0, End = 0;
	//default AI reaction to 1
	Game_Menu(Pause, 0);
	Set_Bmp_Frame(&ScoreNums[0], FRAME_VERT, Score[0]);
	Set_Bmp_Frame(&ScoreNums[1], FRAME_VERT, Score[1]);
	Render_Gfx(UP_SCREEN, 0);
	
	while(Game_Start)
	{
		//check if game over
		if(Score[0] == 9 || Score[1] == 9)
		{
			End = 1;
			Game_Start = 0;
		}
		//play bg music if available
		if(Enable_BGM && GET_FLAG(BGM.Flags, AUDIO_STOP))
			Play_Wave(&BGM,3);
			
		PlayerOneControls(&Player1);
		CPUControls(&AI);
		UpdateBall(&Player1, &AI);
	
		Render_Gfx(UP_SCREEN, 0);
	

		//bottom screen menu
		if(Stylus.Newpress)
		{
			//pause
			if(Stylus.X  >= 35 && Stylus.X <= 115 && Stylus.Y >= 75 && Stylus.Y <= 110)
			{
				Pause = 1;
				Game_Menu(Pause, 0);
				mdelay(30);
				UpdatePad();
				
				while(Pause)
				{
					if(Stylus.Newpress)
					{
						//unpause
						if(Stylus.X  >= 35 && Stylus.X <= 115 && Stylus.Y >= 75 && Stylus.Y <= 110)
						{
							Pause = 0;
							Game_Menu(Pause, 0);
						}
						//menu
						if(Stylus.X  >= 141 && Stylus.X <= 221 && Stylus.Y >= 75 && Stylus.Y <= 110)
						{
							Pause = 0;
							Game_Start = 0;
						}
							
						//screen shot
						if(Stylus.X  >= 75 && Stylus.X <= 221 && Stylus.Y >= 122 && Stylus.Y <= 155)
						{
							Game_Menu(Pause, 0);
							Screen_Cap(DUAL_SCREEN, "/");
						}						
					}
					UpdatePad();
				}
			}
			//menu
			if(Stylus.X  >= 141 && Stylus.X <= 221 && Stylus.Y >= 75 && Stylus.Y <= 110)
				Game_Start = 0;
			
			//screen shot
			if(Stylus.X  >= 75 && Stylus.X <= 221 && Stylus.Y >= 122 && Stylus.Y <= 155)
			{
				Game_Menu(Pause, 0);
				Screen_Cap(DUAL_SCREEN, "/");
			}
		}
		
		
		UpdatePad();
		UpdateAudio();
	}
	int reset = 0;
	if(End)
		reset = Game_Over();
		
	if(!reset)
		Main_Menu();
	
	if(reset)
		goto RESET;
			
}

/*===============================================================
	Skin loading
===============================================================*/
char ChkFileExist(char *filename)
{
	FILE * test = fopen(filename, "r");
	if(!test)
		return -1;
	
	fclose(test);
	return 1;
}

void Load_Skin(char *dir)
{
	char file[256];
	memset(&file, 0, 255);
	
	//load player 1 graphics
	sprintf(file,"%s/paddle.bmp",dir);
	if(ChkFileExist(file))
	{
		Load_BMP_Obj(&Player1.Gfx, file, 0, 0);
		Set_Default_FrameDim(&Player1.Gfx);	
		Set_Bmp_Trans(&Player1.Gfx,RGB15(31,0,31));
	}
	else 
		Debug_Sys_Msg("paddle.bmp not found");

	
	//load player 2 graphics
	sprintf(file,"%s/paddle2.bmp",dir);
	if(ChkFileExist(file))
	{	
		Load_BMP_Obj(&AI.Gfx, file, 0, 0);
		Set_Default_FrameDim(&AI.Gfx);	
		Set_Bmp_Trans(&AI.Gfx,RGB15(31,0,31));
	}
	else 
		Debug_Sys_Msg("paddle2.bmp not found");

	//load the ball graphics
	sprintf(file,"%s/ball.bmp",dir);
	if(ChkFileExist(file))
	{	
		Load_BMP_Obj(&Ball.Gfx, file, 0, 0);
		Set_Default_FrameDim(&Ball.Gfx);	
		Set_Bmp_Trans(&Ball.Gfx,RGB15(31,0,31));
	}
	else
		Debug_Sys_Msg("ball.bmp not found");
		
	//load the background
	sprintf(file,"%s/background.bmp",dir);
	if(ChkFileExist(file))
	{
		Load_BMP_Obj(&BackGround, file, 0, 0);
		Set_Default_FrameDim(&BackGround);
	}
	else
		Debug_Sys_Msg("background.bmp not found");	
	//load score numbers
	sprintf(file,"%s/numbers.bmp",dir);
	if(ChkFileExist(file))
	{	
		Load_BMP_Obj(&ScoreNums[0], file, 24, 32);
		Set_Bmp_Trans(&ScoreNums[0],RGB15(31,0,31));
		Clone_Bitmap(&ScoreNums[0], &ScoreNums[1]);
	}
	else
		Debug_Sys_Msg("numbers.bmp not found");	
		
	//win screen
	sprintf(file,"%s/win.bmp",dir);
	if(ChkFileExist(file))	
	{
		Load_BMP_Obj(&Win, file, 256, 192);
		Set_Bmp_Trans(&Win,RGB15(31,0,31));	
	}
	else
		Debug_Sys_Msg("win.bmp not found");		
	//lose screen
	sprintf(file,"%s/lose.bmp",dir);
	if(ChkFileExist(file))	
	{
		Load_BMP_Obj(&Lose, file, 256, 192);
		Set_Bmp_Trans(&Lose,RGB15(31,0,31));	
	}
	else
		Debug_Sys_Msg("lose.bmp not found");	
		
	//load the sound effects
	sprintf(file,"%s/player.wav",dir);
	if(ChkFileExist(file))
		Load_Wav(file,&Player1.Snd, 1);	
	else 
		Debug_Sys_Msg("player.wav not found");
	
	
	sprintf(file,"%s/ai.wav",dir);
	if(ChkFileExist(file))
		Load_Wav(file,&AI.Snd, 1);	
	else 
		Debug_Sys_Msg("ai.wav not found");
	
	
	sprintf(file,"%s/wall.wav",dir);
	if(ChkFileExist(file))
		Load_Wav(file,&wall, 1);		
	else 
		Debug_Sys_Msg("wall.wav not found");	

	sprintf(file,"%s/bgm.wav",dir);
	if(ChkFileExist(file))
	{
		Load_Wav(file,&BGM, 1);	
		Enable_BGM = 1;
	}
	else
		Enable_BGM = 0;
		
	Skin_Loaded=1;
}	

void Unload_Skin(void)
{
	if(Skin_Loaded)
	{
		//clear graphics
		Delete_Bitmap(&Player1.Gfx);
		Delete_Bitmap(&AI.Gfx);
		Delete_Bitmap(&Ball.Gfx);
		Delete_Bitmap(&BackGround);
		Delete_Bitmap(&ScoreNums[0]);
		Delete_Bitmap(&ScoreNums[1]);
		
		//unload sound effects
		if(Enable_BGM)
		{
			Stop_Wave(&BGM);
			Unload_Snd(&BGM);
		}
			
		Stop_Wave(&wall);
		Unload_Snd(&wall);
		
		Stop_Wave(&AI.Snd);
		Unload_Snd(&AI.Snd);
		
		Stop_Wave(&Player1.Snd);
		Unload_Snd(&Player1.Snd);
	}
	Skin_Loaded = 0;
}
	

void Scan_For_Dirs(struct Directory * folders)
{
	struct stat st;
	DIR* dir;
	dirent *current_file;
	char *folder_name = NULL;
	char temp_folder[256];
	
	dir =  opendir(PongDir);
	while((current_file = readdir_ex(dir, &st)) != NULL)
	{
		if(S_ISDIR(st.st_mode))
		{
			folder_name = current_file->d_name;
			
			if(strcmp (folder_name, ".") && strcmp (folder_name, ".."))
			{
				strcpy(temp_folder, folder_name);
				sprintf(folders->Name[folders->Num_Of_Folds], "%s/%s/",PongDir, temp_folder);
				folders->Num_Of_Folds++;
			}
		}
	}
	 closedir(dir);
}

/*===============================================================
	Program start
===============================================================*/
void Strip_Skin_Name(char *name, char *dest)
{
	int i =strlen(PongDir) + 1;
	int j =0;
	while( i < strlen(name) -1)
		dest[ j++ ]=name[ i++ ];
		
	dest[ j] = '\0';
}

void draw_Line_rect(unsigned short *dest, int x1, int y1, int x2, int y2, unsigned short color)
{
	int i;
	
	for(i = x1; i<= x2; i++)
	{
		dest[ i + (y1 << 8)] = color;
		dest[ i + (y2 << 8)] = color;
	}
	
	i = 0;
	
	for(i = y1; i<= y2; i++)
	{
		dest[ x1 + (i << 8)] = color;
		dest[ x2 + (i << 8)] = color;
	}	
}

void Game_Menu(bool pause, bool end)
{
	ds2_clearScreen(DOWN_SCREEN, RGB15(0,0,0));
	
	draw_Line_rect(down_screen_addr, 35, 75, 115, 110, RGB16(0,31,0));
	if(!end)
	{//pause and unpause
		if(!pause)
			Font_OutputText(down_screen_addr,50,80,256,192,"Pause",FT_FIXED(25),&Menu_fnt,RGB16(0,31,0));
		else
			Font_OutputText(down_screen_addr,40,80,256,192,"Unpause",FT_FIXED(25),&Menu_fnt,RGB16(0,31,0));
	}
	//rematch
	else
		Font_OutputText(down_screen_addr,38,80,256,192,"Rematch",FT_FIXED(25),&Menu_fnt,RGB16(0,31,0));
		
	//menu button	
	draw_Line_rect(down_screen_addr, 141, 75, 221, 110, RGB16(31,0,0));
	Font_OutputText(down_screen_addr,158,80,256,192,"Menu",FT_FIXED(25),&Menu_fnt,RGB16(31,0,0));	
	
	//screnshot button
	draw_Line_rect(down_screen_addr, 75, 122, 184, 154, RGB16(0,0,31));
	Font_OutputText(down_screen_addr,80,127,256,192,"Screenshot",FT_FIXED(25),&Menu_fnt,RGB16(0,0,31));	
	
	//p1 p2
	Font_OutputText(down_screen_addr,0,0,256,192,"P1",FT_FIXED(25),&Menu_fnt,RGB16(31,31,31));	
	Font_OutputText(down_screen_addr,256-20,0,256,192,"P2",FT_FIXED(25),&Menu_fnt,RGB16(31,31,31));	
	
	ds2_flipScreen(DOWN_SCREEN, 1);	
}	


void Update_Menu(char *skin)
{
	ds2_clearScreen(DOWN_SCREEN, RGB15(0,0,0));
	
	Font_OutputText(down_screen_addr,0,0,256,192, "v 1.01",FT_FIXED(16),&Menu_fnt,RGB16(0,0,31));
	
	
	draw_Line_rect(down_screen_addr, 90, 35, 170, 70, RGB16(0,31,0));
	Font_OutputText(down_screen_addr,110,40,256,192,"Play!",FT_FIXED(25),&Menu_fnt,RGB16(0,31,0));
	
	draw_Line_rect(down_screen_addr, 90, 80, 170, 115, RGB16(31,0,0));
	Font_OutputText(down_screen_addr,105,85,256,192,"Exit :(",FT_FIXED(25),&Menu_fnt,RGB16(31,0,0));	
	
	Font_OutputText(down_screen_addr,50,140,256,192,"<-  Current Skin  ->",FT_FIXED(20),&Menu_fnt,RGB16(31,31,0));
	Font_OutputText(down_screen_addr,100,160,256,192,skin,FT_FIXED(20),&Menu_fnt,RGB16(31,31,0));
	
	ds2_flipScreen(DOWN_SCREEN, 1);	
}


char tempSkin[256];

void LoadNewSkin(int skin)
{
	
	if(skin <0 )
		skin = 0;
	else if(skin >= Skins.Num_Of_Folds)
		skin = Skins.Num_Of_Folds-1;
		
	Skin_Selected = skin;
	Unload_Skin();
	Load_Skin(Skins.Name[skin]);

	Strip_Skin_Name(Skins.Name[skin], tempSkin);
	Update_Menu(tempSkin);
}

void Main_Menu(void)
{
	
	char file[256];
	sprintf(file,"%s/block.ttf",PongDir);
	bool test = Font_FatLoad(file,&Menu_fnt);
	if(!test)
		Debug_Sys_Msg("failed to load block.ttf");
		
	
	LoadNewSkin(0);
	Update_Menu(tempSkin);	

	//bottom screen menu
	bool Menu =1;
	
	Set_Paddle(&Player1, 1, 1);
	Set_Paddle(&AI, 2, 1);
	ResetBall(&Player1, &AI);
	
	Render_Gfx(UP_SCREEN, 1);
			
	int skin_num=0;
	while(Menu)
	{
		if(Enable_BGM && GET_FLAG(BGM.Flags, AUDIO_STOP))
			Play_Wave(&BGM,3);
	
			
		CPUControls(&AI);
		CPUControls(&Player1);
		UpdateBall(&Player1, &AI);
		
		Render_Gfx(UP_SCREEN, 1);
		
		if(Pad.Newpress.R)
			Screen_Cap(DUAL_SCREEN, "/");

		if(Stylus.Newpress)
		{
			//backwards one skin
			if(Stylus.X  >= 42 && Stylus.X <= 73 && Stylus.Y >= 135 && Stylus.Y <= 161)
			{
				skin_num--;
				LoadNewSkin(skin_num);
				skin_num = Skin_Selected;
			}
			//forward
			if(Stylus.X  >= 182 && Stylus.X <= 213 && Stylus.Y >= 135 && Stylus.Y <= 161)
			{
				skin_num++;
				LoadNewSkin(skin_num);
				skin_num = Skin_Selected;
			}	
			
			//exit
			if(Stylus.X  >= 90 && Stylus.X <= 170 && Stylus.Y >= 80 && Stylus.Y <= 178)
				ds2_plug_exit 	();
				
			//play game
			if(Stylus.X  >= 90 && Stylus.X <= 170 && Stylus.Y >= 35 && Stylus.Y <= 70)				
				Menu = 0;
		}
		//mdelay(12);
		UpdatePad();
		UpdateAudio();
	}
	
	Game_Loop();
}
		


void ds2_main(void)
{
	//Initial video and audio and other input and output
	ds2io_init(1024);
	//initiate the global interrupt
	sti();
	//set clock speed to 0
	ds2_setCPUclocklevel(6);
	//Initial console for printf funciton
	ConsoleInit(RGB16(31,31,31), RGB15(0,0,0), DOWN_SCREEN, 10);
	//Initial file system
	fat_init();
	Debug_Sys_Init("/pong.log", 0, 0);
	SET_FLAG(Debug.Flags, DBG_HALT);
	
	Get_Data_dir();
	
	
	Init_MixBuffer();
	ds2_setVolume(127);
	
	struct rtc time;
	ds2_getTime(&time);
	srand(time.year + time.month + time.day + time.weekday + time.hours + time.minutes + time.seconds);
	
	Scan_For_Dirs(&Skins);
	
	Main_Menu();

	while(1);
}

