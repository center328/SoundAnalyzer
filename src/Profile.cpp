#include "Precomp.h"
#include <windows.h>
 /* using QueryPerformanceCounter for precise measurement */
 #include "Profile.h"
 
Profiler g_Profiler;
LARGE_INTEGER frequency;

 double LIToSecs( LARGE_INTEGER & L) 
 {
     return ((double)L.QuadPart /(double)frequency.QuadPart) ;
 }

 void InitProfiler()
 {
     QueryPerformanceFrequency( &frequency ) ;
 }
 
 Profile_Data::Profile_Data()
{
	start.QuadPart=0;
	stop.QuadPart=0; 
	ProfileBegin();
	type = NOT_TRACKED;
}

Profile_Data::Profile_Data(char *name)
{
	start.QuadPart=0;
	stop.QuadPart=0; 
	profile_id = g_Profiler.getProfileID(name);
	ProfileBegin();
	type = TRACKED;
}

Profile_Data::~Profile_Data()
{
	ProfileEnd();
		
	if(type == TRACKED)
	{
		g_Profiler.updateProfile(profile_id,ProfileElapsedTime() * 1000.0);// hold time in milis
	}
}

 void Profile_Data::ProfileBegin()
 {
     QueryPerformanceCounter(&start) ;
 }
 
 void Profile_Data::ProfileEnd() 
 {
     QueryPerformanceCounter(&stop) ;
 }
 
 double Profile_Data::ProfileElapsedTime()
 {
     LARGE_INTEGER time;
     time.QuadPart = stop.QuadPart - start.QuadPart;
     
	 return LIToSecs(time) ;
 }

 Profiler::Profiler()
{
	currentProfile = 0;
	 QueryPerformanceFrequency( &frequency ) ;
}

	
int Profiler::getProfileID(char *name)
{
	currentProfile++;
	strcpy(profiles_info[currentProfile].name,name);
	return currentProfile;
}

void Profiler::updateProfile(int id, double time)
{
	profiles_info[id].time = time;
}

void Profiler::reset()
{
	currentProfile = 0;
}