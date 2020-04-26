#include <atari.h>
#include <string.h>
#include <peekpoke.h>

#define DLIST_MEM		0x4000		// aligned to 1K
#define SCREEN_MEM		0x5000
#define PMG_MEM			0x6000		// aligned to 2K
#define CHARSET_MEM0	0x3000		// aligned to 1K

#define MOVING_RIGHT	7
#define MOVING_LEFT		11
#define MOVING_NONE		15

#define PLAYER_SPEED 	4

#define LEFT_FIELD		56
#define RIGHT_FIELD		184
#define TOP_FIELD 		40
#define BORDER_THICK	2
#define FIRST_ROW		3

#define SCREEN_WIDTH 	40
#define SCREEN_HIGHT	24
#define ROWS 			5
#define COLUMNS 		12

#define	PLAYER_XPOS  	115
#define	PLAYER_YPOS 	 200
#define	BALL_XPOS  		PLAYER_XPOS + 7
#define	BALL_YPOS  		150
#define	BALL_XV  		-1
#define	BALL_YV  		2

#define BRICK_SIZE 		3

#define COLLISION_DELAY 100


unsigned char stick;
unsigned char trigger;
unsigned char moving_type;
unsigned char fire_state;
unsigned char game_over;
const unsigned char player[8] = {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
const unsigned char ball[4] = {0xff, 0xff, 0xff, 0xff};
unsigned char player_ypos;
unsigned char player_xpos;
unsigned char ball_ypos;
unsigned char ball_xpos;
char ball_xv;
char ball_yv;
unsigned char collision_player;
unsigned char collision_delay;
unsigned char collision_brick;
unsigned char brick_hit_x;
unsigned char brick_hit_y;
unsigned char bricks[ROWS][COLUMNS] = 
{
	{1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1}
	
}; 

unsigned char antic4_display_list[] = 
{
	DL_BLK8,
	DL_BLK8,
	DL_BLK8,
	DL_LMS(DL_CHR40x8x4),
	0x00,
	0x50,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_CHR40x8x4,
	DL_JVB,
	0x00,
	0x40
};

void draw_bricks(void);
void wait_for_vblank(void)
{
    asm("lda $14");
	wvb:
    asm("cmp $14");
    asm("beq %g", wvb);
}

// void dli_routine0(void) // draw ground
// {
// 	asm("pha");
//     asm("txa");
//     asm("pha");
//     asm("tya");
//     asm("pha");

// 	ANTIC.wsync = 1;
// 	ANTIC.chbase = CHAR1;
// 	GTIA_WRITE.colpf0 = 0x2A;
// 	GTIA_WRITE.colpf1 = 0xBA;
// 	GTIA_WRITE.colpf2 = 0x0A;
// 	GTIA_WRITE.colpf3 = 0x68;
// 	GTIA_WRITE.colbk = 0x00;
	
//     OS.vdslst = &dli_routine1;

	
//     asm("pla");
//     asm("tay");
//     asm("pla");
//     asm("tax");
//     asm("pla");
//     asm("rti");
// }

// void dli_routine1(void) // draw ground
// {
// 	asm("pha");
//     asm("txa");
//     asm("pha");
//     asm("tya");
//     asm("pha");

// 	ANTIC.wsync = 1;
// 	ANTIC.chbase = CHAR2;
// 	GTIA_WRITE.colpf0 = 0x2A;
// 	GTIA_WRITE.colpf1 = 0xBA;
// 	GTIA_WRITE.colpf2 = 0x76;
// 	GTIA_WRITE.colpf3 = 0x46;
// 	GTIA_WRITE.colbk = 0x00;
//     OS.vdslst = &dli_routine2;

	
//     asm("pla");
//     asm("tay");
//     asm("pla");
//     asm("tax");
//     asm("pla");
//     asm("rti");
// }

// void dli_routine2(void) // draw ground
// {
// 	asm("pha");
//     asm("txa");
//     asm("pha");
//     asm("tya");
//     asm("pha");

// 	ANTIC.wsync = 1;
// 	ANTIC.chbase = CHAR3;
//     OS.vdslst = &dli_routine0;
	
	
//     asm("pla");
//     asm("tay");
//     asm("pla");
//     asm("tax");
//     asm("pla");
//     asm("rti");
// }

void init_nmis(void)
{
	wait_for_vblank();
	// OS.vdslst = &dli_routine0;
	ANTIC.nmien = NMIEN_DLI | NMIEN_VBI;

}

void restart_game(void)
{
	player_xpos = PLAYER_XPOS;
	player_ypos = PLAYER_YPOS;
	ball_xpos = BALL_XPOS;
	ball_ypos = BALL_YPOS;
	ball_xv = BALL_XV;
	ball_yv = BALL_YV;
	collision_player = 0;
	collision_brick = 0;
	collision_delay = 0;
}

void handle_input(void)
{
	stick = OS.stick0;
	trigger = OS.strig0;
	
	if (!game_over)
	{
		moving_type = stick;
		fire_state = trigger;
	}
	

	if (CONSOL_START(GTIA_READ.consol))
	{
		restart_game();
	}
}

void update_sprite(void) 
{
	// handle player movement
	if (moving_type == MOVING_RIGHT)
	{
		player_xpos += PLAYER_SPEED;
		if (player_xpos >= RIGHT_FIELD)
			player_xpos = RIGHT_FIELD;
	}
	else if (moving_type == MOVING_LEFT)
	{
		player_xpos -= PLAYER_SPEED;
		if (player_xpos <= LEFT_FIELD)
			player_xpos = LEFT_FIELD;
	}

	// player - ball collision
	if (collision_player)
	{
		ball_yv = -ball_yv;
		if (collision_player == 1 && ball_xv == 1)
		{
			ball_xv = -ball_xv;
		}
		if (collision_player == 2 && ball_xv == -1)
		{
			ball_xv = -ball_xv;
		}
		collision_player = 0;
	}

	// ball - border collision
	if (ball_xpos >= RIGHT_FIELD + 14 || ball_xpos <= LEFT_FIELD)
		ball_xv = -ball_xv;
	if (ball_ypos <= TOP_FIELD)
		ball_yv = -ball_yv;

	// ball - brick collision
	if (collision_brick)
	{
		ball_yv = -ball_yv;

		brick_hit_x = (ball_xpos - LEFT_FIELD) / 12; 	// brick width = 12
		brick_hit_y = (ball_ypos - 56) / 8; 			// brick hight = 4
	}

	ball_xpos += ball_xv;
	ball_ypos += ball_yv;


}

void draw_sprite(void)
{
	GTIA_WRITE.hposp0 = player_xpos;
	GTIA_WRITE.hposp1 = player_xpos + 8;
	GTIA_WRITE.hposm0 = ball_xpos;
	memcpy((void*)(PMG_MEM + 0x300 + ball_ypos),&ball,4);

	if (collision_brick)
	{
		memset((void*)(SCREEN_MEM + SCREEN_WIDTH * (brick_hit_y+FIRST_ROW) + brick_hit_x*3+2), 0, 3);
		collision_brick = 0;
	}
	
}

void erase_sprite(void)
{
	memset((void*)(PMG_MEM + 0x300 + ball_ypos),0,4);

}

void detect_collision(void)
{
	// ball-player collision
	if (GTIA_READ.m0pl || GTIA_READ.m0pf) 
	{
		if (collision_delay == 0) // delay to prevent stickiness
		{
			collision_delay = COLLISION_DELAY;
			collision_player = GTIA_READ.m0pl; // player number - 1=letf-side, 2=right-side
			collision_brick = GTIA_READ.m0pf;
		}
		else
			collision_delay--;	
	}
	else
		collision_delay = 0;

	GTIA_WRITE.hitclr = 1;
}	


void setup_pmg(void)
{	
	// tell ANTIC what is the memory address of the PMG memory
    ANTIC.pmbase = PMG_MEM >> 8;

    // clear PMG memory
	bzero((void*)(PMG_MEM+0x300),0x500);


	// tell GTIA what type of PMG do we want?
	GTIA_WRITE.gractl = GRACTL_MISSLES + GRACTL_PLAYERS;

    // set the priority register (order of objects appear on screen)
	// PRIOR_P03_PF03 | PRIOR_5TH_PLAYER | PRIOR_OVERLAP_3RD_COLOR
    OS.gprior = PRIOR_P03_PF03 | PRIOR_OVERLAP_3RD_COLOR | PRIOR_5TH_PLAYER;

    // player colors , set shadows
    OS.pcolr0 = 0xFF;
    OS.pcolr1 = 0xFF;
    OS.pcolr2 = 0xFF;
    OS.pcolr3 = 0xFF;

    // missiles combining the 5th player get their color from location 711
    // which is playfield 3
	OS.color3 = 0x26; 

    // player size (normal, double, quad)
    GTIA_WRITE.sizem  = PMG_SIZE_NORMAL;	
    GTIA_WRITE.sizep0 = PMG_SIZE_NORMAL;
    GTIA_WRITE.sizep1 = PMG_SIZE_NORMAL;
    GTIA_WRITE.sizep2 = PMG_SIZE_NORMAL;
    GTIA_WRITE.sizep3 = PMG_SIZE_NORMAL;

    
    // missile horizontal position
    // GTIA_WRITE.hposm0 = bird_xpos;
    // GTIA_WRITE.hposm1 = bird_xpos -2;
    // GTIA_WRITE.hposm2 = bird_xpos -4;
    // GTIA_WRITE.hposm3 = bird_xpos -6;

    // player horizontal position
    // GTIA_WRITE.hposp0 = 0x20;
    // GTIA_WRITE.hposp1 = 0x41;
    // GTIA_WRITE.hposp2 = 0x49;
    // GTIA_WRITE.hposp3 = 0x49;

    // memset((void*)(PMG_MEM + 0x400 + 0x90),255,8);
}

void draw_border(void)
{
	unsigned char i;
	for (i=0; i<SCREEN_WIDTH; i++)
	{
		POKE(SCREEN_MEM + i, 100);
	}
	for (i=0; i<SCREEN_HIGHT; i++)
	{
		memset((void*)(SCREEN_MEM + SCREEN_WIDTH *i),100,BORDER_THICK);
		memset((void*)(SCREEN_MEM + SCREEN_WIDTH *i + SCREEN_WIDTH - BORDER_THICK),100,BORDER_THICK);
	}
}

void draw_bricks(void)
{
	unsigned char i,j;

	for (i=0; i<= ROWS; i++)
	{
		for (j=0; j<COLUMNS; j++)
		{
			if (bricks[i][j] == 1)
			{
				if (j %2 == 0 )
					memset((void*)(SCREEN_MEM + SCREEN_WIDTH * (i+FIRST_ROW) + j*3+2), 101, 3);
				else
					memset((void*)(SCREEN_MEM + SCREEN_WIDTH * (i+FIRST_ROW) + j*3+2), 102, 3);
			}
		}
	}
}

void main(void)
{
	// shut down ANTIC DMA
	OS.sdmctl = 0;

	memcpy((void*)DLIST_MEM,antic4_display_list, sizeof(antic4_display_list));
	OS.sdlst = (void*)DLIST_MEM;

	// copy original charcter set to new location
	memcpy((void*)CHARSET_MEM0, (void*)0xE000, 0x400);
	// override original characters with custom ones
	memset((void*) (CHARSET_MEM0 + 8 * 100) , 0x55, 8);
	memset((void*) (CHARSET_MEM0 + 8 * 101) , 0xAA, 8);
	memset((void*) (CHARSET_MEM0 + 8 * 102) , 0xFF, 8);

	OS.chbas = CHARSET_MEM0 >> 8; // MSB / 256

	// init colors
	OS.color0 = 0x22;       // playerfield 0 - red
	OS.color1 = 0XB2;		// playerfield 1 - green
	OS.color2 = 0X82;		// playerfield 2 - blue
	OS.color3 = 0XEE;		// playerfield 3 - yello
	OS.color4 = 0x00;		// background - black
	
	init_nmis();
	setup_pmg();
	restart_game();

	memcpy((void*)(PMG_MEM + 0x400 + player_ypos),&player,sizeof(player));
	memcpy((void*)(PMG_MEM + 0x500 + player_ypos),&player,sizeof(player));
	draw_border();
	draw_bricks();
    OS.sdmctl = 0x3E; // single line

	while(1)
	{
		wait_for_vblank();
		handle_input();
		erase_sprite();
		update_sprite();
		draw_sprite();
		detect_collision();
		// handle_game_over();
	};
}