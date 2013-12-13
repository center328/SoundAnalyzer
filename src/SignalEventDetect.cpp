#include "Precomp.h"
#include "SignalEventDetect.h"
#include "SoundPlayback.h"
#include "kiss_fft.h"
#include <math.h> 

/* this may be changed to detect different instruments if you have a good model */
int BandSeparation [] = {0,800,4000,9000,20000}; // values in HZ

struct SignalData
{
	kiss_fft_cfg	cfg;
	kiss_fft_cpx	in[FFT_BUFFER_SIZE];
	
	kiss_fft_cpx	out[FFT_BUFFER_SIZE];
	float			out_amp[FFT_BUFFER_SIZE/2];
};

SignalData sigData;
SignalExchangeData g_sigExchange;

extern soundExchangeData g_SoundExchange;


void SignalProc_init()
{
	
	for (int i = 0; i < FFT_BUFFER_SIZE; i++)
		sigData.in[i].r = sigData.in[i].i = 0;

	sigData.cfg = kiss_fft_alloc(FFT_BUFFER_SIZE, 0/*is_inverse_fft*/, NULL, NULL);
	
}

void SignalProc_sample(float * data)
{
	PROFILE(SignalProc_sample);
	int startPos = 0;
	int endPos = FFT_BUFFER_SIZE;
	
	int k = 0;
	for (int i = startPos; i < endPos; i++)
	{
		sigData.in[k].r = data[i] ;
		sigData.in[k].i = 0;
		k++;
	}
}

void SignalProc_calculateMagnitude()
{
	PROFILE(SignalProc_calculateMagnitude);

	sigData.out_amp[0] = sqrt(sigData.out[0].i* sigData.out[0].i + sigData.out[0].r * sigData.out[0].r); 
	float minAmp = sigData.out_amp[0]; 
	for(int i = 1 ; i< FFT_BUFFER_SIZE/2;i++)
	{
		sigData.out_amp[i] =	sqrt(sigData.out[i].i* sigData.out[i].i + sigData.out[i].r * sigData.out[i].r); 
		if(_isnan(sigData.out_amp[i]))
			{
				assert(0);
			}

		if(minAmp > sigData.out_amp[i])
		{
			minAmp = sigData.out_amp[i];
		}
	}
	// offset the calculated magnitude so we don't get numerical instability
	for(int i = 0 ; i< (FFT_BUFFER_SIZE/2);i++)
	{
		 sigData.out_amp[i] -= minAmp;
	}
}

void SignalProc_updateSignalHistory()
{
	PROFILE(SignalProc_updateSignalHistory);

	for(int i = 0; i< SUB_BAND_NUMBER; i++)
	{
		if(g_sigExchange.out_sub_band[i].currentHistoryIdx < HISTORY_SAMPLES)
		{
			int idx = g_sigExchange.out_sub_band[i].currentHistoryIdx;
			g_sigExchange.out_sub_band[i].historyValues[idx] = 	g_sigExchange.out_sub_band[i].currentValue;
			g_sigExchange.out_sub_band[i].currentHistoryIdx++;
		}
		else
		{
			for(int k = 0; k< (HISTORY_SAMPLES-1); k++)
			{
				g_sigExchange.out_sub_band[i].historyValues[k] = g_sigExchange.out_sub_band[i].historyValues[k +1];  
			}
			
			g_sigExchange.out_sub_band[i].historyValues[HISTORY_SAMPLES-1] = g_sigExchange.out_sub_band[i].currentValue;
		}
	}
}

void SignalProc_calculateAvgEnergy()
{
	PROFILE(SignalProc_calculateAvgEnergy);

	for(int i = 0; i< SUB_BAND_NUMBER; i++)
	{
		g_sigExchange.out_sub_band[i].avg_Energy = 0.0f;
		for(int k = 0; k< HISTORY_SAMPLES; k++)
		{
			g_sigExchange.out_sub_band[i].avg_Energy += g_sigExchange.out_sub_band[i].historyValues[k];  
		}
		g_sigExchange.out_sub_band[i].avg_Energy = g_sigExchange.out_sub_band[i].avg_Energy/HISTORY_SAMPLES;
	}
}

/* split the signal into our subbands */
void SignalProc_splitInBins()
{
	PROFILE(SignalProc_splitInBins);

	float HZ_increment = (float)g_SoundExchange.sampleRate/FFT_BUFFER_SIZE;
	int IndexValues[SUB_BAND_NUMBER + 1];
	
	for(int i = 0; i< SUB_BAND_NUMBER + 1; i++)
	{
		IndexValues[i] = (int)(BandSeparation[i]/HZ_increment);
	}

	for(int i = 0; i< SUB_BAND_NUMBER; i++)
	{
		float s = 0;
		
		int k = IndexValues[i];
		int limit = IndexValues[i+1];
						
		for (; k< limit; k++)
		{
			s += sigData.out_amp[k];
		}

		g_sigExchange.out_sub_band[i].currentValue = (SUB_BAND_NUMBER/(float)(FFT_BUFFER_SIZE/2))  * s;
	}
				

}


void SignalProc_update()
{
	PROFILE(SignalProc_update);
	g_SoundExchange.hasNewData = false;
	
	//Downsample the signal for the FFT transformation , this looses precision
	SignalProc_sample(g_SoundExchange.dataForFFT);
	{
		PROFILE(kiss_fft);
		//Apply FFT transform
		kiss_fft(sigData.cfg, sigData.in, sigData.out);
	}

	//Calculate signal magnitude
	SignalProc_calculateMagnitude();
	
	//Split in bins
	SignalProc_splitInBins();
	
	//Update Signal History
	SignalProc_updateSignalHistory();
	
	//Calculate Average Energy
	SignalProc_calculateAvgEnergy();
}

void SignalProc_end()
{
  free(sigData.cfg);
}
