#ifndef _RENDER_H
#define _RENDER_H
#include "defines.h"


/* The subband width calculated from the screen size*/
#define RENDER_BAND_WIDTH (SCREEN_WIDTH/SUB_BAND_NUMBER)
/* The subband normal height */
#define RENDER_BAND_HEIGHT (20)
/* The subband height becomes HIT_NOTE_SIZE_MULTIPLICATOR * RENDER_BAND_HEIGHT when you need to hit it */
#define HIT_NOTE_SIZE_MULTIPLICATOR 5.0

/* Structure that holds the data for rendering the subbands */
struct SpriteInfo
{
/* is the subband visible */
	_BOOL		visible;

/* time it was active on screen */
	double		timeonScreen;

/* subband it represents, so we know how to color it */
	int			SubBand;

/* Rect used to draw the subband (x,y,w,h) */
	SDL_Rect	rect;

/* Color used to draw it */
	SDL_Color	color;

/* Init the struct */
	SpriteInfo():visible(FALSE),SubBand(0),timeonScreen(0.0){ }
};

/*Holds the SDL render stuff */
struct RenderInfo
{
	SDL_Window*		win;
	SDL_Renderer*	ren;

	TTF_Font*		font;

/* pool used for getting new sprites */
	SpriteInfo		sprite_pool[MAX_NOTES_ON_SCREEN];
	int				new_sprite_idx;
};


/* Init the render system*/
void Render_init();

/* Draw function */
void Render_drawFrame();

/* Clean the structs */
void Render_end();

/* Get new Sprite from pool */
SpriteInfo *Render_getNewSprite();

#endif