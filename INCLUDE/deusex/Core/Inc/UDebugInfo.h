// ----------------------------------------------------------------------
//  File Name   :  UDebugInfo.h
//  Programmer  :  Chris Norden
//  Description :  Header for global debug object (duh)
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef __INCLUDED_UDEBUGINFO_H
#define __INCLUDED_UDEBUGINFO_H

////////////////////////////
// UDebugInfo
// is really a bridge class to connect UC to Cpp

class CORE_API UDebugInfo: public UObject
{
	DECLARE_CLASS(UDebugInfo, UObject, 0)

	public:
		UDebugInfo();

	public:
		DECLARE_FUNCTION(execAddTimingData)
		DECLARE_FUNCTION(execCommand)

		DECLARE_FUNCTION(execSetString)
		DECLARE_FUNCTION(execGetString)
};

enum
{
	CORE_DebugInfo_AddTimingData=4000,
	CORE_DebugInfo_Command=4001,

	CORE_DebugInfo_SetString=4002,
	CORE_DebugInfo_GetString=4003,
};


// this typedef is for callbacks out of debug changes
typedef void (*fDbgCallback)(const TCHAR *changed);

////////////
// helper code to allow you to simply declare an array of "DebugControls"
// which map debug system string/value pairs with variables in your code
// you can then just call helper functions in the DebugSys to
//   a) parse the ini file and load up the debug vars
//   b) map the debug vars into your variables

// what sort of timing variable you have declared
enum eTimingType 
{ 
	kTiming_Str,
	kTiming_Int, 
	kTiming_Bool 
};

// the actual variables themselves
struct sDebugControls {
	TCHAR		Str[32];
	eTimingType Type;
	void	   *Var;
};

/////////////////////////////
// UDebugSys
// is the actual class holding the current situation for the debug system

class CORE_API UDebugSys
{
	public:
		// da constructor
		UDebugSys();

		/////////////////
		// ways to add data for timing/profiling of systems
		//   obj is what to associate the timing with
		//   event is what subclass of things the timing is for
		// i.e. AddTimingData("Player","Tick") or ("Player","Physics") or whatever
		void         AddTimingData(const TCHAR *obj, const TCHAR *event, int Time, UObject *me);
		void         ClearTimingData(void);
		void         ShowTimingData(void);
		void         TickTimingData(void);


		/////////////////
		// these are for "configuration" of debugging
		// i.e. you set "AIDebugObj" to "MaggieChow" or something

		// takes a cmd from the enum in DbgInfoCpp.h
		void         Command(int cmd, const TCHAR *param);

		// Set a value string to attach to the name string
		void         SetString(const TCHAR *str, const TCHAR *val);

		// Gets the value string attached to str, or NULL if not attached
		const TCHAR *GetString(const TCHAR *str);
		const TCHAR *GetString(const TCHAR rawstr);

		// returns 0 if undefined, else tries to ASCII->Int the string
		int			 GetValue(const TCHAR *str);
		int			 GetValue(const TCHAR rawstr);

		// register a function to call when the debug variables change
		void         RegisterCallback(fDbgCallback pfCallback);
		void		 RemoveCallback(fDbgCallback pfCallback);

		// for managing and mapping the debugcontrol variables for your code
		void		 LoadVars(sDebugControls *pCtrlList, int iCnt);
		void		 ScanVars(sDebugControls *pCtrlList, int iCnt);
};

#endif  // __INCLUDED_UDEBUGINFO_H