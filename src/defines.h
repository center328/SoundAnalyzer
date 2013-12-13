/* Screen width and height */
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT  768

/* Number of samples to playback before calling the audio callback */
#define AUDIO_SAMPLE_SIZE (1024)

/* FFT buffer size is always 2 time the audio sample size because we hold the previous data. This is needed for better resolution (overlapping) and Numerical Windowing */
#define FFT_BUFFER_SIZE (AUDIO_SAMPLE_SIZE *2)

/* Number of subbands*/
#define SUB_BAND_NUMBER 4
/* Maximum number of sprites on screen at a certain moment in time */
#define MAX_NOTES_ON_SCREEN (SUB_BAND_NUMBER * 256) 
/* Maximum number of notes shown at the same period of time */
#define MAX_SIMULTANOUS_TRACKS 2

/* THRESHOLD for detecting if the energy variance represents an event (it's multiplicative)*/
#define ENERGY_THRESHOLD 2.0f
/* Number of history samples used for the average energy */
#define HISTORY_SAMPLES 10

/* Speed measured in pixels/second with wich the notes translate */
#define NOTES_SPEED 384
/* Where the notes will actually be heard (SCREEN_HEIGHT - SCREEN_Y_OFFSET)*/
#define SCREEN_Y_OFFSET 50

/* Time necessary for the notes to reach the place where they will be heard */
#define TIME_TO_EXIT_SCREEN ((SCREEN_HEIGHT-SCREEN_Y_OFFSET)/NOTES_SPEED)
/* Time necessary for the notes to exit the screen */
#define TIME_TO_EXIT_SCENE ((SCREEN_HEIGHT)/NOTES_SPEED)
/* 2*TIME_OFFSET_HIT represents the ammmount of time you may hit the note -> This also represents the animation */
#define TIME_OFFSET_HIT 0.1

/* My BOOL */
#define TRUE  1
#define FALSE 0
#define _BOOL  int

#define USE_LOGGER
//#define USE_PROFILER 
#define DISPLAY_FPS
