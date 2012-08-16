#ifndef NX_FOUNDATION_NXPROFILER
#define NX_FOUNDATION_NXPROFILER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nx.h"
#include "NxAllocateable.h"

/**
This profiler is a modified version of the "low level" portion of the code
published with the article "Interactive Profiling" in the Dec 2002 GDMag.

This program is Copyright (c) 2002 Jonathan Blow.  All rights reserved.
Permission to use, modify, and distribute a modified version of 
this software for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and 
that both the copyright notice and this permission notice appear 
in supporting documentation and that the source code is provided 
to the user.

THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
FITNESS FOR A PARTICULAR PURPOSE.


Usage:

1) define a zone -- put this in the global namespace:

NxProfiler::DefineZone myZoneName("MyZoneName");

2) set the zone whenever it is used:

NxProfiler::SetCurrentZone xx(myZoneName);

*/
#define NX_MAX_PROFILING_ZONES 64
#define NX_MAX_PROFILING_STACK_DEPTH 256

#define NX_NUM_PROFILE_TRACKER_HISTORY_SLOTS 3
#define NX_NUM_THROWAWAY_UPDATES 10

//new stuff to avoid other SDKs from bypassing public interface of foundation:
class NxProfilingZone
	{
	public:
	virtual void release() = 0;
	virtual void enter() = 0;
	virtual void leave() = 0;
	};

class NxSetCurrentZone
	{
	NxProfilingZone * zone;
	public:
	NX_INLINE NxSetCurrentZone(NxProfilingZone * z);
	NX_INLINE ~NxSetCurrentZone();
	};

enum NxDisplayMode 
	{
    NX_SELF_TIME = 0,
    NX_HIERARCHICAL_TIME,
    NX_SELF_STDEV,
    NX_HIERARCHICAL_STDEV
	};

class NXF_DLL_EXPORT NxProfiler
	{
	public:
		class NXF_DLL_EXPORT DefineZone : public NxAllocateable, public NxProfilingZone
		{
		const char *name;
		int index;

		NxI64 total_self_ticks;
		NxI64 total_hier_ticks;

		NxI64 t_self_start;
		NxI64 t_hier_start;

		int total_entry_count;

		public:
		DefineZone(const char *name);
		DefineZone();
		~DefineZone();
		const char * getName();

		virtual void release();
		virtual void enter();
		virtual void leave();


		friend class NxProfiler;
		friend class Profiler;
		};

	class NXF_DLL_EXPORT SetCurrentZone
		{
		public:
		SetCurrentZone(DefineZone &zone);
		~SetCurrentZone();
		};


	/**
	call at startup to initialize the data structures.
	may call multiple times with no ill effect.
	*/
	static void initialize();

	/**
	set what sort of meta data is to be computed.
	*/
    static void setMode(NxDisplayMode);

	/**
	call to update the meta data.
	*/
    static void update();

	//second profiler part, analysis -- could put in another class:

	struct History_Scalar 
		{
		double values[NX_NUM_PROFILE_TRACKER_HISTORY_SLOTS];
		double variances[NX_NUM_PROFILE_TRACKER_HISTORY_SLOTS];
		
		void update(double new_value, double *k_array);
		void eternity_set(double value);
		
		void clear();
		};

	//public data that can be used by extern routines to display info:
	
	struct Profile_Tracker_Data_Record 
		{
		int index;
		History_Scalar self_time;
		History_Scalar hierarchical_time;
		History_Scalar entry_count;
		double displayed_quantity;
		};

	//stuff that can be displayed:
    static DefineZone **zone_pointers_by_index;
	static int maxZones;	//size of above array
    static History_Scalar frame_time;
    static NxDisplayMode displayed_quantity;
    static History_Scalar integer_timestamps_per_second;
    static int num_active_zones;

    static Profile_Tracker_Data_Record *sorted_pointers[NX_MAX_PROFILING_ZONES];


	private:
	static void initializeHighLevel();

	static bool initialized;

	//measurement data:
	static DefineZone defaultZone;
    static int current_zone;
    static int stack_pos;
    static int num_zones;
    static int * zone_stack;
	static int stackSize;	//size of above array

    static void Enter_Zone(DefineZone &zone);
    static void Exit_Zone();
	static void getTime(NxI64 *result);

	friend class DefineZone;
	friend class SetCurrentZone;



	//second profiler part, analysis -- could put in another class:

    static int stackData[NX_MAX_PROFILING_STACK_DEPTH];
    static DefineZone *pointersData[NX_MAX_PROFILING_ZONES];


	static int sort_records(const void *a, const void *b);
	static double get_stdev(History_Scalar *scalar, int slot);

	//meta data:

    static Profile_Tracker_Data_Record data_records[NX_MAX_PROFILING_ZONES];
    static double times_to_reach_90_percent[NX_NUM_PROFILE_TRACKER_HISTORY_SLOTS];
    static double precomputed_factors[NX_NUM_PROFILE_TRACKER_HISTORY_SLOTS];

    static NxI64 last_integer_timestamp;
    static NxI64 current_integer_timestamp;
    static int update_index;
    static double last_update_time;
    static double dt;
    static double dt_per_integer_timestamp;
	};


NX_INLINE NxSetCurrentZone::NxSetCurrentZone(NxProfilingZone * z) : zone(z)
	{
	zone->enter();
	}

NX_INLINE NxSetCurrentZone::~NxSetCurrentZone()
	{
	zone->leave();
	}

#endif
