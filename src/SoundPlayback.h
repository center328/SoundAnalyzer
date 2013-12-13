#ifndef _SOUND_PLAYBACK_H
#define _SOUND_PLAYBACK_H

#include "defines.h"

/* The playback error enum */
enum E_SOUND_PLAYBACK_ERROR
{
	E_PLAYBACK_OGG_FILE_NOT_FOUND,
	E_PLAYBACK_OGG_FILE_NOT_VALID,
	E_PLAYBACK_CANNOT_OPEN_AUDIO,
	E_PLAYBACK_OK,
	E_PLAYBACK_ERROR_NUM
};



/* Sound data used for signal processing*/
struct soundExchangeData
{
/* Tells the signal processor if it should process the signal or not */
	_BOOL			hasNewData;
/* Sample rate measured in HZ */
	int				sampleRate; 
/* Input data for FFT */
	float			*dataForFFT;

};

/* init sound stuff */
int		Playback_initSound();

/*get Stream File handle */
E_SOUND_PLAYBACK_ERROR		Playback_open(const char* path); 

/*display Debug info from the ogg file*/
void	Playback_display();         

/* check if OpenAl source is still playing */
_BOOL	Playback_playing();   

/* helper function for getting the error string */
const char* Playback_GetError(E_SOUND_PLAYBACK_ERROR id);

/* clear structs */
void Playback_end();
	
#endif