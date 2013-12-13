#include "Precomp.h"
#include "defines.h"
#include "SoundPlayback.h"

//OGG includes
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>


soundExchangeData g_SoundExchange;

using namespace std;

enum E_MAIN_BUFFER_READ
{
	 PLAYBACK_READ_FILE,
	 FFT_READ_FILE,
	 BUFFER_NUM
};

struct Playback_Data
{
	/*OGG data */
	FILE*           oggFile;       /* file handle */
    OggVorbis_File  oggStream;     /* stream handle */
    vorbis_info*    vorbisInfo;    /* some formatting data */
    vorbis_comment* vorbisComment; /* user comments */
	
	/* SDL Audio Data */
	SDL_AudioSpec wanted; /* data about the channel playback rate, sample size etc */

	/* the 2 big buffers for reading data from file (audio data and fft data are offset by the ammount of time it takes to get accross the screen */
	char					*audio_MainData[BUFFER_NUM];
	int						 audio_MainDataLen;
	int						 audio_MainSampleLen;
	int						 current_PlayPosition;

	/* double buffered playback*/
	unsigned char			*audio_BufferData[2];
	int						audio_BufferLen;
	unsigned char			buffer_idx;

	/* used to detect if file has finished playing */
	_BOOL			isPlaying;
	
};

/* internal data */
static Playback_Data _DataOgg;

_BOOL OGG_errorString(unsigned int buffer);   // reloads a buffer
_BOOL Playback_stream(unsigned int buffer);
_BOOL ReadFromOggFile(char *fill_buffer);

/*	callback when finished playing buffer */
 void fill_audio(void *udata, unsigned char *stream, int len)
{
		_DataOgg.current_PlayPosition += AUDIO_SAMPLE_SIZE;
 		int playbackBuffer = _DataOgg.buffer_idx;
		_DataOgg.buffer_idx = (_DataOgg.buffer_idx +1)  % 2;

		_DataOgg.isPlaying = Playback_stream(playbackBuffer);
		
		g_SoundExchange.hasNewData = TRUE;

		if(_DataOgg.isPlaying)
		{
			memcpy(stream, _DataOgg.audio_BufferData[playbackBuffer],_DataOgg.audio_BufferLen);
		}
}



int Playback_initSound()
{
	g_SoundExchange.dataForFFT = NULL;
	
	_DataOgg.isPlaying = TRUE;
	_DataOgg.buffer_idx = 0;
	return 0;
}


E_SOUND_PLAYBACK_ERROR Playback_open(const char* path)
{
    
    if(!(_DataOgg.oggFile = fopen(path, "rb")))
	{
        LOG("Could not open Ogg file.");
		return E_PLAYBACK_OGG_FILE_NOT_FOUND;
	}

    if( ov_open(_DataOgg.oggFile, &_DataOgg.oggStream, NULL, 0) < 0)
    {
        fclose(_DataOgg.oggFile);
 
        LOG("Could not open Ogg stream. %s", path);
		return E_PLAYBACK_OGG_FILE_NOT_VALID;
    }

	_DataOgg.vorbisInfo		= ov_info(&_DataOgg.oggStream, -1);
    _DataOgg.vorbisComment	= ov_comment(&_DataOgg.oggStream, -1);
 
    

	g_SoundExchange.sampleRate = _DataOgg.vorbisInfo->rate; 
	
	_DataOgg.audio_BufferLen		=	AUDIO_SAMPLE_SIZE * _DataOgg.vorbisInfo->channels * 2;
	_DataOgg.audio_BufferData[0]	= new unsigned char[_DataOgg.audio_BufferLen]; // 2 for 16 bit resolution , 8 is not currently supported
	_DataOgg.audio_BufferData[1]	= new unsigned char[_DataOgg.audio_BufferLen];
	
	g_SoundExchange.dataForFFT = new float[FFT_BUFFER_SIZE];

	

	memset(g_SoundExchange.dataForFFT,0,sizeof(float)*FFT_BUFFER_SIZE);

		
	_DataOgg.wanted.freq		= _DataOgg.vorbisInfo->rate;
	_DataOgg.wanted.format		= AUDIO_S16;
	_DataOgg.wanted.channels	= _DataOgg.vorbisInfo->channels;
	_DataOgg.wanted.samples		= AUDIO_SAMPLE_SIZE;
	_DataOgg.wanted.callback	= fill_audio;
	_DataOgg.wanted.userdata	= NULL;

	_DataOgg.audio_MainSampleLen =  _DataOgg.vorbisInfo->rate * TIME_TO_EXIT_SCREEN;
	_DataOgg.audio_MainDataLen = _DataOgg.audio_MainSampleLen *  _DataOgg.vorbisInfo->channels * 2;
	
	_DataOgg.audio_MainData[FFT_READ_FILE] = new char[_DataOgg.audio_MainDataLen];
	_DataOgg.audio_MainData[PLAYBACK_READ_FILE] = new char[_DataOgg.audio_MainDataLen];

	ReadFromOggFile(_DataOgg.audio_MainData[PLAYBACK_READ_FILE]);
	ReadFromOggFile(_DataOgg.audio_MainData[FFT_READ_FILE]);
	_DataOgg.current_PlayPosition = 0;
	
	if ( SDL_OpenAudio(&_DataOgg.wanted, NULL) < 0 ) 
	{
        LOG("Couldn't open audio: %s\n", SDL_GetError());
        return E_PLAYBACK_CANNOT_OPEN_AUDIO;
    }
	// init first buffer
	Playback_stream(0);

	SDL_PauseAudio(0);

	return E_PLAYBACK_OK;
}

void Playback_display()
{
	LOG( "OGG INFO_______________________________________________ \n");
    LOG( "version     %d   \n " , _DataOgg.vorbisInfo->version);
	LOG( "channels     %d  \n " , _DataOgg.vorbisInfo->channels);
    LOG( "rate (hz)     %lu \n" ,_DataOgg.vorbisInfo->rate);
	LOG( "____________________________________________________ \n");

}



_BOOL Playback_playing()
{
	return 	_DataOgg.isPlaying;
}

_BOOL ReadFromOggFile(char *fill_buffer)
{
	int  size = 0;
	int  section;
	int  result;

	while(size < _DataOgg.audio_MainDataLen)
    {
		char * BufferStart= (char*)(fill_buffer + size);
        result = ov_read(&_DataOgg.oggStream, BufferStart, _DataOgg.audio_MainDataLen - size, 0, 2, 1, & section);
       
        if(result > 0)
            size += result;
        else
		{
            if(result < 0)
			{
                cout<<"error:"<< result;
			}
            else
			{
                break;
			}
		}
    }

    if(size == 0)
        return FALSE;

	return TRUE;
}

_BOOL Playback_stream(unsigned int buffer)
{
	if(_DataOgg.current_PlayPosition + AUDIO_SAMPLE_SIZE > _DataOgg.audio_MainSampleLen)
	{
		memcpy(_DataOgg.audio_MainData[PLAYBACK_READ_FILE],_DataOgg.audio_MainData[FFT_READ_FILE],_DataOgg.audio_MainDataLen);
		_BOOL isPlaying = ReadFromOggFile(_DataOgg.audio_MainData[FFT_READ_FILE]);
		_DataOgg.current_PlayPosition = 0;

		if(isPlaying == FALSE)
			return FALSE;
	}
	

	
	// deinterlace add for fft on end
	int k = 0;
	short *samples = ((short*)(_DataOgg.audio_MainData[FFT_READ_FILE])) + _DataOgg.current_PlayPosition *2;
	//copy old data to first half
	memcpy(g_SoundExchange.dataForFFT,g_SoundExchange.dataForFFT + AUDIO_SAMPLE_SIZE,sizeof(float) *(AUDIO_SAMPLE_SIZE));
	//copy new data to second half
	for(int i = AUDIO_SAMPLE_SIZE ; i< 2 * AUDIO_SAMPLE_SIZE; i++)
	{
		if(_DataOgg.vorbisInfo->channels == 2)
		{
			g_SoundExchange.dataForFFT[i] = (samples[k] + samples[k+1])/2.0f;
			k+=2;
		}
		else
		{
			g_SoundExchange.dataForFFT[i] = samples[k];
			k++;
		}
	}
	/* populate sound playback buffer with first part of wave;*/
	char *playback = _DataOgg.audio_MainData[PLAYBACK_READ_FILE]+ _DataOgg.current_PlayPosition *4;
	memcpy(_DataOgg.audio_BufferData[buffer],playback,AUDIO_SAMPLE_SIZE * 4);

    return TRUE;
}


const char* OGG_errorString(int code)
{
    switch(code)
    {
        case OV_EREAD:
            return "Read from media.";
        case OV_ENOTVORBIS:
            return "Not Vorbis data.";
        case OV_EVERSION:
            return "Vorbis version mismatch.";
        case OV_EBADHEADER:
            return "Invalid Vorbis header.";
        case OV_EFAULT:
            return "Internal logic fault (bug or heap/stack corruption.";
        default:
            return "Unknown Ogg error.";
    }
}

void Playback_end()
{
	 ov_clear(&_DataOgg.oggStream);

	 	
	delete [] _DataOgg.audio_MainData[FFT_READ_FILE];
	delete [] _DataOgg.audio_MainData[PLAYBACK_READ_FILE];

	delete[](_DataOgg.audio_BufferData[0]); 
	delete[](_DataOgg.audio_BufferData[1]);

	if(g_SoundExchange.dataForFFT != NULL)
		delete [] g_SoundExchange.dataForFFT;

}

#define MAX_ERROR_STRING 512
char E_SOUND_PLAYBACK_ERROR_strings[E_PLAYBACK_ERROR_NUM][MAX_ERROR_STRING] ={"File not found", "File not valid", "Cannot open audio","ok"};
const char *Playback_GetError(E_SOUND_PLAYBACK_ERROR id)
{
	return E_SOUND_PLAYBACK_ERROR_strings[id];
}