// $Header: DbgInfoCpp.h$

// Include this file in any CPP file from which you want to use the debug object
// this has #defines to use to add data to the timing system
// and if you change the state of the master define, it will take out all code
//   associated with this system....

#ifndef __INCLUDED_DBGINFOCPP_H
#define __INCLUDED_DBGINFOCPP_H

// for now... this could simply be its own #define, if we wanted
//#ifdef PLAYTEST
//#define USE_DEBUG_SYSTEM
//#endif

#ifdef USE_DEBUG_SYSTEM

// get the actual debug object class definition
#include "UDebugInfo.h"

// go and get the debug object itself, so we can do things with it...
#define GetDebugObj()						(&GDebugSys)

// these declare and use a timer named iTimer to get the timing data
#define DbgDeclareTimer(iTimer)             int iTimer=0;
#define DbgClock(iTimer)					do { clock(iTimer); } while (0)
#define DbgUnclock(iTimer)					do { unclock(iTimer); } while (0)

// actual calls to the debug object functions
#define DbgAddTiming(sObj,sEv,iTime)		GetDebugObj()->AddTimingData(sObj,sEv,iTime,this)
#define DbgAddTimingAnon(sObj,sEv,iTime)	GetDebugObj()->AddTimingData(sObj,sEv,iTime)
#define DbgClearTiming()					GetDebugObj()->ClearTimingData()
#define DbgShowTiming()						GetDebugObj()->ShowTimingData()
#define DbgTickTiming()						GetDebugObj()->TickTimingData()
#define DbgCommandTiming(iCmd,sParam)		GetDebugObj()->CommandTimingData(iCmd,sParam)

// if you rather save one step, the finish here "Auto-Adds" the timing data too
#define DbgStartTimer(iTimer)				DbgDeclareTimer(iTimer); DbgClock(iTimer)
#define DbgFinishTimer(sObj,sEv,iTimer)		do { DbgUnclock(iTimer); DbgAddTiming(sObj,sEv,iTimer); } while (0)

// this expression should be compiled in in a DEBUG_SYSTEM build
#define DbgExp(x)                           x

#define DbgSetString(key,val)               GetDebugObj()->SetString(key,val)
#define DbgGetString(key)                   GetDebugObj()->GetString(key)
#define DbgGetBool(key)                    ((GetDebugObj()->GetString(key)!=NULL)&&(GetDebugObj()->GetValue(key)!=0))
#define DbgGetValue(key)                    GetDebugObj()->GetValue(key)

#define DbgSetCallback(fCallback)           GetDebugObj()->RegisterCallback(fCallback)
#define DbgClearCallback(fCallback)         GetDebugObj()->RemoveCallback(fCallback)

#define DbgLoadConfig(tstr) \
	do { \
		TCHAR _CfgStr[256]; \
		if (GConfig->GetString(TEXT("Debug.Vars"),tstr,_CfgStr,256)) \
			DbgSetString(tstr,_CfgStr); \
	} while (0)

#define DbgLoadVars(vars) do { GetDebugObj()->LoadVars(vars,sizeof(vars)/sizeof(vars[0])); } while (0)
#define DbgScanVars(vars) do { GetDebugObj()->ScanVars(vars,sizeof(vars)/sizeof(vars[0])); } while (0)


// and, for the really hardcore
// so you dont need to put stupid finishes on every damn return statement....
// the correct solution
class CORE_API UDebugAutoTimer
{
	private:
		int		     m_iTimer;
		UObject     *m_pMe;
		const TCHAR *m_psObj;
		const TCHAR *m_psEvent;
		int          m_iReleased;
	
	public:
		// pass this as me, or, if you want to be anon, just dont
		UDebugAutoTimer(const TCHAR *psObj, const TCHAR *psEvent, UObject *me=NULL)
		{ 
			m_psObj=psObj;
			m_psEvent=psEvent;
			m_pMe=me;
			m_iTimer=0;
			m_iReleased=0;
			clock(m_iTimer);
		}

		void StoreTimer(void)
		{
			unclock(m_iTimer);
			GDebugSys.AddTimingData(m_psObj, m_psEvent, m_iTimer, m_pMe);
			m_iReleased=1;
		}

		~UDebugAutoTimer()  
		{ 
			if (m_iReleased==0)
				StoreTimer();
		}
};

#define DbgDeclTimer(tag,sObj,sEv,self)     UDebugAutoTimer cAutoTimer##tag##(sObj,sEv,self)
#define DbgDeclTimerAnon(tag,sObj,sEv)		UDebugAutoTimer cAutoTimer##tag##(sObj,sEv)
#define DbgStoreTimer(tag)					cAutoTimer##tag##.StoreTimer()

enum eDebugCmds {
	kDebugCmd_ListVars,
};

#else  // USE_DEBUG_SYSTEM

#define GetTheDebugObject()                 NULL

#define DbgDeclareTimer(iTimer)
#define DbgClock(iTimer)
#define DbgUnclock(iTimer)
#define DbgAddTiming(sObj,sEvent,iTime)
#define DbgClearTiming()
#define DbgShowTiming()
#define DbgTickTiming()
#define DbgCommandTiming(iCmd,sParam)

#define DbgStartTimer(iTimer)
#define DbgFinishTimer(sObj,sEvent,iTimer)

#define DbgExp(x)

#define DbgSetString(key,val)
#define DbgGetString(key)                   NULL
#define DbgGetBool(key)                     (0)
#define DbgGetValue(key)                    (0)

#define DbgSetCallback(fCallback)
#define DbgClearCallback(fCallback)

#define DbgLoadConfig(tstr)

#define DbgLoadVars(vars)
#define DbgScanVars(vars)

#define DbgDeclTimer(tag,sObj,sEve,self)
#define DbgDeclTimerAnon(tag,sObj,sEv)
#define DbgStoreTimer(tag)

#endif // USE_DEBUG_SYSTEM

#endif // __INCLUDED_DBGINFOCPP_H