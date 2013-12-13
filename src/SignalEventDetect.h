#ifndef _SIGNAL_EVENT_DETECT_H
#define _SIGNAL_EVENT_DETECT_H

#include "defines.h"

/* We will split the signal into subbands with manually chosen frequency ranges */
struct SubBand
{
	/* The instant energy value */
	float currentValue;
	/* The vector used for a number of HISTORY_SAMPLES the previous energy values */
	/* we will do an average with this, so the HISTORY_SAMPLES influences the responsiveness of the sistem */
	float historyValues[HISTORY_SAMPLES];
	/*how much of the historyValues vector is populated, this will most of the time be HISTORY_SAMPLES*/
	int   currentHistoryIdx;
	/* the computed average of the energy, this is calculated from averaging the historyValues array */
	float avg_Energy;

	/*Init all the stuff */
	SubBand()
	{
		currentHistoryIdx = 0;
		currentValue = 0;
		avg_Energy = 0.0f;
		memset(historyValues,0,sizeof(historyValues));
	}
};


/* Structure that holds the exchange data (output of the signal processing) */
struct SignalExchangeData
{
	SubBand			out_sub_band[SUB_BAND_NUMBER];
};

/* init signal processing structs */
void SignalProc_init();
/* The signal processing update function -> handles all the realtime stuff */
void SignalProc_update();
/* Clears the structs */
void SignalProc_end();

#endif