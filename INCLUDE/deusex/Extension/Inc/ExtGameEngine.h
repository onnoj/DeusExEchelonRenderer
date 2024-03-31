
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtGameEngine.h
//  Programmer  :  Scott Martin
//  Description :  Header for the GameEngine extended class
// ----------------------------------------------------------------------
//  Copyright �1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_GAME_ENGINE_H_
#define _EXT_GAME_ENGINE_H_


// ----------------------------------------------------------------------
// XGameEngineExt class

class EXTENSION_API XGameEngineExt : public UGameEngine
{
	DECLARE_CLASS(XGameEngineExt, UGameEngine, CLASS_Config|CLASS_Transient)

	public:
		XGameEngineExt();

		// Structors
		void Init(void);
		void Destroy(void);
		void Serialize(FArchive &ar);

		UBOOL Browse(FURL URL, const TMap<FString,FString>* TravelInfo, FString& Error);

		// UGameEngine interface
		void  MouseDelta(class UViewport *pViewport, DWORD clickFlags, FLOAT dX, FLOAT dY);
		void  MousePosition(class UViewport *pViewport, DWORD buttons, FLOAT x, FLOAT y);
		int   Key(UViewport *viewport, EInputKey key);
		void  Tick(FLOAT deltaSeconds);

};  // XGameEngineExt


#endif // _EXT_GAME_ENGINE_H_
