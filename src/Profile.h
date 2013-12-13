#include <windows.h>
#ifndef _PROFILER_H
#define _PROFILER_H

#include "defines.h"

#define MAX_PROFILE_NUMBER 256


enum E_PROFILE_TYPE
{
	/* TRACKED profile data is registered in the profile vector*/

	TRACKED,
	/*NOT_TRACKED profile data is handled by user*/

	NOT_TRACKED
};

/* Class for measuring time */
class Profile_Data
{
     LARGE_INTEGER	start;
     LARGE_INTEGER	stop;
	 double			elapsedTime;
	 int			profile_id;

	 E_PROFILE_TYPE			type;
public:
	 Profile_Data();

	 Profile_Data(char *name);


	 ~Profile_Data();

	void		ProfileBegin();
	void		ProfileEnd();
	double		ProfileElapsedTime();
};

#define MAX_PROFILE_NAME 1024
/* Hold details about profiling stuff for display */
struct Profile_Info
{
	char name[MAX_PROFILE_NAME];
	double time;
};

/* The actual profiler */
class Profiler
{
	Profile_Info profiles_info[MAX_PROFILE_NUMBER];
	int currentProfile;

public:

	Profiler();
	int getProfileID(char *name);
	void updateProfile(int id, double time);

	void reset();

};


extern Profiler g_Profiler;


#ifdef USE_PROFILER
	#define PROFILE(X) Profile_Data Variabila_cu_nume_lung8080(#X);
#else
	#define PROFILE(X) 
#endif
#endif