#include "Precomp.h"
#include "Render.h"
#include "Profile.h"

RenderInfo g_renderInfo;
Profile_Data FrameTime;

/* kind of a nasty way of drawing text.. I sometimes get as much as 3 ms for drawing FPS.. */
void DrawDebugText(char * str,int x, int y)
{
	PROFILE(DrawDebugText);

	SDL_Color textColor;
	textColor.a = textColor.r = textColor.g = textColor.b =255;

	SDL_Surface * message = TTF_RenderText_Solid( g_renderInfo.font, str, textColor );
	SDL_Texture *tex = SDL_CreateTextureFromSurface(g_renderInfo.ren,message);

	SDL_Rect FPSRect;
		
	FPSRect.x = 1; 
    FPSRect.y = 1; 
    FPSRect.w = message->w; 
    FPSRect.h = message->h; 

	SDL_RenderCopy(g_renderInfo.ren,tex,NULL,&FPSRect);
	SDL_FreeSurface(message);
	SDL_DestroyTexture(tex);
}

void SetSprite_Color(SpriteInfo &sprite)
{
	sprite.color.a = 255;
		
	switch (sprite.SubBand)
	{
		case 0:
			{
				sprite.color.r = 255;
				sprite.color.g = 0;
				sprite.color.b = 0;
				break;

			}
		case 1:
			{
				sprite.color.r = 0;
				sprite.color.g = 0;
				sprite.color.b = 255;
				break;
			}
		case 2:
			{
				sprite.color.r = 0;
				sprite.color.g = 255;
				sprite.color.b = 0;
				break;
			}
		case 3:
			{
				sprite.color.r = 255;
				sprite.color.g = 255;
				sprite.color.b = 0;
				break;
			}
	}

}

SpriteInfo *Render_getNewSprite()
{
	 g_renderInfo.new_sprite_idx = (g_renderInfo.new_sprite_idx +1 )%MAX_NOTES_ON_SCREEN;

	 g_renderInfo.sprite_pool[g_renderInfo.new_sprite_idx].rect.x = 0;
	 g_renderInfo.sprite_pool[g_renderInfo.new_sprite_idx].rect.y = 0;
	 g_renderInfo.sprite_pool[g_renderInfo.new_sprite_idx].timeonScreen = 0.0;

	 return &g_renderInfo.sprite_pool[g_renderInfo.new_sprite_idx];
}


void Render_init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		LOG( "SDL_Init Error: %s \n" ,SDL_GetError() );
		return;
	}

	g_renderInfo.win = SDL_CreateWindow("SoundAnalyzer", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
	
	if (g_renderInfo.win == NULL)
	{
		LOG("SDL_CreateWindow Error: %s \n", SDL_GetError() );
		return;
	}

	g_renderInfo.ren = SDL_CreateRenderer(g_renderInfo.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (g_renderInfo.ren == nullptr)
	{
		LOG("SDL_CreateRenderer Error: %s \n" ,SDL_GetError());
		return;
	}
	
	if (TTF_Init() != 0)
	{
      LOG("TTF_Init() Failed: %s \n", TTF_GetError());
	  return;
	}
	
	g_renderInfo.font = NULL;
	g_renderInfo.font = TTF_OpenFont( "Sintony-Regular.ttf", 16 );
	
	if(g_renderInfo.font == NULL )
	{
		 LOG( "Cannot find font: %s \n" , TTF_GetError());
	}

}


void Render_drawFrame()
{
	FrameTime.ProfileEnd();
	double timeInSec = FrameTime.ProfileElapsedTime();
	
	FrameTime.ProfileBegin();

	SDL_SetRenderDrawColor(g_renderInfo.ren,0,0,0,255);
	SDL_RenderClear(g_renderInfo.ren);



	for(int i = 0 ; i< MAX_NOTES_ON_SCREEN ; i++)
	{
		SpriteInfo &currentSprite  = g_renderInfo.sprite_pool[i];

		if(currentSprite.visible == TRUE)
		{
			SetSprite_Color(currentSprite);
			SDL_SetRenderDrawColor(g_renderInfo.ren,currentSprite.color.r,currentSprite.color.g,currentSprite.color.b,currentSprite.color.a);
			currentSprite.rect.x =  currentSprite.SubBand * RENDER_BAND_WIDTH;

			currentSprite.rect.y = (int)(currentSprite.rect.y + NOTES_SPEED * timeInSec);
			currentSprite.rect.w = RENDER_BAND_WIDTH;
			currentSprite.rect.h = RENDER_BAND_HEIGHT;

			currentSprite.timeonScreen += timeInSec;

			if( currentSprite.timeonScreen > TIME_TO_EXIT_SCREEN -TIME_OFFSET_HIT)
			{
				double ammount = 1.0 -abs(TIME_TO_EXIT_SCREEN - currentSprite.timeonScreen)/(TIME_OFFSET_HIT*2);
				currentSprite.rect.h = (int)(RENDER_BAND_HEIGHT* HIT_NOTE_SIZE_MULTIPLICATOR * ammount);
			}
			if( currentSprite.timeonScreen > TIME_TO_EXIT_SCREEN +TIME_OFFSET_HIT)
			{
				currentSprite.rect.h = RENDER_BAND_HEIGHT;
			}

			if(TIME_TO_EXIT_SCENE < currentSprite.timeonScreen)
			{
				currentSprite.visible  = false;
			}

			SDL_RenderFillRect(g_renderInfo.ren,&g_renderInfo.sprite_pool[i].rect);
		}
	}

#ifdef DISPLAY_FPS
	char FPS[64] ={0};
	sprintf(FPS,"FPS:%f",(float)1.0/timeInSec);
	DrawDebugText(FPS,10, 50);
#endif

	SDL_RenderPresent(g_renderInfo.ren);

#ifdef USE_PROFILER
	g_Profiler.reset();
#endif
}


void Render_end()
{
	TTF_CloseFont(g_renderInfo.font);
	SDL_DestroyRenderer(g_renderInfo.ren);
	SDL_DestroyWindow(g_renderInfo.win);
	TTF_Quit();
	SDL_Quit();
}