
#include "Precomp.h"
#include "SoundPlayback.h"
#include "SignalEventDetect.h"
#include "Render.h"
#include "defines.h"

#include <algorithm>
#include <vector>

using namespace std;

 
extern soundExchangeData	g_SoundExchange;
extern SignalExchangeData	g_sigExchange;
extern RenderInfo			g_renderInfo;

/* used for sorting energy values on subbands */
struct SortStruct
{
	/* diference between average energy value and current energy value */
	float	diff;
	/* energy band index */
	int		idx;

	bool operator<(const SortStruct& b) 
	{
		return diff < b.diff;
	}
};
std::vector<SortStruct>sortVec;


/* used for detecting if we already showed this frequency configuration */
_BOOL prevShowFreq[SUB_BAND_NUMBER] = {FALSE};
_BOOL currentShowFreq[SUB_BAND_NUMBER] = {FALSE};

int main(int argc, char** argv)
{
//Memory leak detection
#ifdef _DEBUG
	//_crtBreakAlloc = ;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	if(argc < 2)
	{
		LOG("you need to call this: SoundAnalyzer.exe NameOfAudioFile.ogg (only supports 2 channel 16 bit sample ogg files)");
		return 1;
	}
/* Init All the stuff */
	sortVec.resize(SUB_BAND_NUMBER);
	SDL_Event event;

	Render_init();	
	Playback_initSound();

	E_SOUND_PLAYBACK_ERROR result =Playback_open(argv[1]);
	if(result != E_PLAYBACK_OK)
	{
		LOG("Error Playback : %s",Playback_GetError(result));
		return 1;
	}

	Playback_display();

	SignalProc_init();
/* End of Init */
	
	while( TRUE)
	{
		/*SDL Event stuff*/
		while( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_QUIT )
            {
				break;
            }
        }

		if(Playback_playing()) 
		{
			/*if we have new data , we process it */
			if(g_SoundExchange.hasNewData)
			{
				SignalProc_update();
			}
		
			/*calculate the differences between the average energy and the current energy level for each sub band, so we can detect big variations */
			for(int i = 0 ; i< SUB_BAND_NUMBER ; i++)
			{
				float diff = (g_sigExchange.out_sub_band[i].currentValue - (g_sigExchange.out_sub_band[i].avg_Energy * ENERGY_THRESHOLD));
				
				sortVec[i].idx = i;
				sortVec[i].diff = diff;
			}
			/* The below part does not have a good processing basis, it's mostly for gameplay : You may experiment with different stuff here */
		
			std::sort(sortVec.begin(),sortVec.end());

			int k = 0;
			/* should display new?*/
			for(int i = 0 ; i< SUB_BAND_NUMBER ; i++)
			{
				if(k >= MAX_SIMULTANOUS_TRACKS)
					break;

				if(sortVec[i].diff > 0 )
				{
					currentShowFreq[sortVec[i].idx] = TRUE;
				}
				else
				{
					currentShowFreq[sortVec[i].idx] = FALSE;
				}
			}

			if(memcmp(currentShowFreq,prevShowFreq,sizeof(currentShowFreq)) != 0)
			{
				memcpy(prevShowFreq,currentShowFreq,sizeof(prevShowFreq));

				for(int i = 0 ; i< SUB_BAND_NUMBER ; i++)
				{
					if(k >= MAX_SIMULTANOUS_TRACKS)
						break;

					if(sortVec[i].diff > 0 )
					{
						SpriteInfo *newSprite = Render_getNewSprite();
						newSprite->visible = TRUE;
						newSprite->SubBand = sortVec[i].idx;
						k++;
					
					}
			
				}
			}
			Render_drawFrame();
			
		}
		else
		{
			break;			
		}
	}
	
	
	LOG( "Program normal termination.");


    SignalProc_end();
	Render_end();
	Playback_end();

	return 0;
}