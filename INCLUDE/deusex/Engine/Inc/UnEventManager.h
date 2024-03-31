
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  UnEventManager.h
//  Programmer  :  Scott Martin
//  Description :  Header for the audio/visual event manager
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm Austin.  This software is a trade secret.
// ----------------------------------------------------------------------

// DEUS_EX STM - new header for Engine

#ifndef _UN_EVENT_MANAGER_H_
#define _UN_EVENT_MANAGER_H_


// ----------------------------------------------------------------------
// Constants

const INT MAX_EVENT_HASH  = 256;              // Must be a power of two
const INT EVENT_HASH_MASK = (MAX_EVENT_HASH-1);

const INT MAX_SEND_SLOTS  = 16;               // Must be a power of two
const INT SEND_SLOT_MASK  = (MAX_SEND_SLOTS-1);


// ----------------------------------------------------------------------
// Enumerations

enum EIntEventType  {
	EVENTTYPE_Send,
	EVENTTYPE_Receive
};


// ----------------------------------------------------------------------
// Parameters used for event callbacks

struct XAIParams
{
	class AActor *bestActor;
	FLOAT        score;
	FLOAT        visibility;
	FLOAT        volume;
	FLOAT        smell;
};


// ----------------------------------------------------------------------
// Settings used internally by the event manager

struct XAIEventSettings
{
	friend FArchive &operator<<(FArchive &Ar, XAIEventSettings &E)
	{
		guard(XAIEventSettings<<);

		Ar << E.visual;
		Ar << E.audio;
		Ar << E.audioRadius;
		Ar << E.smell;

		return (Ar);
		unguard;
	}

	FLOAT visual;
	FLOAT audio;
	FLOAT audioRadius;
	FLOAT smell;
};


// ----------------------------------------------------------------------
// Event type linked list

struct XAIEventType : public UObject
{
	DECLARE_CLASS(XAIEventType, UObject, 0)

	void Serialize(FArchive &Ar);

	FName                   eventName;
	INT                     eventHash;
	struct XAISenderEvent   *senders;
	struct XAIReceiverEvent *receivers;
	struct XAIEventType     *nextEventType;
};


// ----------------------------------------------------------------------
// Generic event type

struct XAIEvent : public UObject
{
	DECLARE_CLASS(XAIEvent, UObject, 0)

	void Serialize(FArchive &Ar)
	{
		Super::Serialize(Ar);
		Ar << eventType << eventActor << bBeingDestroyed;
		Ar << nextEvent;
	}

	struct XAIEventType *eventType;
	class AActor        *eventActor;
	UBOOL               bBeingDestroyed;
	XAIEvent            *nextEvent;

	XAIEvent *FirstValidEvent(void)
	{
		XAIEvent *pEvent = this;
		while (pEvent)
		{
			if (!pEvent->bBeingDestroyed)
				break;
			pEvent = pEvent->nextEvent;
		}
		return (pEvent);
	}
};


// ----------------------------------------------------------------------
// Sender event

struct XAISenderEvent : public XAIEvent
{
	DECLARE_CLASS(XAISenderEvent, XAIEvent, 0)

	void Serialize(FArchive &Ar)
	{
		Super::Serialize(Ar);
		// ignore holdScore -- it's temporary
		for (INT i=0; i<MAX_SEND_SLOTS; i++)
			Ar << settings[i];
		Ar << currentSettings;
	}

	FLOAT            holdScore;

	XAIEventSettings settings[MAX_SEND_SLOTS];
	XAIEventSettings currentSettings;

	XAISenderEvent *First(void) { return ((XAISenderEvent *)FirstValidEvent()); }
	XAISenderEvent *Next(void)  { return ((XAISenderEvent *)nextEvent->FirstValidEvent()); }
};


// ----------------------------------------------------------------------
// Receiver event

struct XAIReceiverEvent : public XAIEvent
{
	DECLARE_CLASS(XAIReceiverEvent, XAIEvent, 0)

	void Serialize(FArchive &Ar)
	{
		Super::Serialize(Ar);
		Ar << callback << scoreCallback << bInvokeCallback << eventState;
		Ar << bCheckVisibility << bCheckDir << bCheckCylinder << bCheckLOS;
		Ar << bEventOn << bestScore << bestSender;
		Ar << nextSlot;
		Ar << nextProcess << prevProcess;
		// We are deliberately NOT archiving params...
	}

	FName            callback;
	FName            scoreCallback;
	UBOOL            bInvokeCallback;
	BYTE             eventState;

	UBOOL            bCheckVisibility;
	UBOOL            bCheckDir;
	UBOOL            bCheckCylinder;
	UBOOL            bCheckLOS;

	UBOOL            bEventOn;
	FLOAT            bestScore;
	class AActor     *bestSender;

	INT              nextSlot;

	XAIParams        params;

	XAIReceiverEvent *nextProcess;
	XAIReceiverEvent *prevProcess;

	XAIReceiverEvent *GetNextProcess(void)
	{
		XAIReceiverEvent *pEvent = this;
		while (pEvent)
		{
			pEvent = pEvent->nextProcess;
			if (!pEvent->bBeingDestroyed)
				break;
			if (pEvent == this)
				break;
		}
		return (pEvent);
	}
	XAIReceiverEvent *GetPrevProcess(void)
	{
		XAIReceiverEvent *pEvent = this;
		while (pEvent)
		{
			pEvent = pEvent->prevProcess;
			if (!pEvent->bBeingDestroyed)
				break;
			if (pEvent == this)
				break;
		}
		return (pEvent);
	}

	XAIReceiverEvent *First(void) { return ((XAIReceiverEvent *)FirstValidEvent()); }
	XAIReceiverEvent *Next(void)  { return ((XAIReceiverEvent *)nextEvent->FirstValidEvent()); }
};


// ----------------------------------------------------------------------
// Serialize call for event types (needed here for circular references)

inline void XAIEventType::Serialize(FArchive &Ar)
{
	Super::Serialize(Ar);
	Ar << eventName << eventHash;
	Ar << senders << receivers;
	Ar << nextEventType;
}


// ----------------------------------------------------------------------
// UEventManager class

class ENGINE_API UEventManager : public UObject
{
	DECLARE_CLASS(UEventManager, UObject, 0)

	public:
		UEventManager();
		UEventManager(class ULevel *newLevel);

		void Destroy(void);
		void Serialize(FArchive &Ar);

	public:
		class ULevel     *level;

	protected:
		INT              refProcessing;
		INT              deleteCount;

		INT              currentSlot;
		XAIReceiverEvent *firstProcess;
		XAIEventType     *eventTable[MAX_EVENT_HASH];

	public:
		void AISetEventCallback(class AActor *receiver, FName eventName, FName callback,
		                        FName scoreCallback=NAME_None,
		                        UBOOL bCheckVisibility=true, UBOOL bCheckDir=true,
		                        UBOOL bCheckCylinder=false, UBOOL bCheckLOS=true);
		void AIClearEventCallback(class AActor *receiver, FName eventName);
		void AICheckEvent(class AActor *receiver, FName eventName);

		void AISendEvent(class AActor *sender, FName eventName, EAIEventType eventType,
		                 FLOAT value=1.0, FLOAT radius=-1.0);
		void AIStartEvent(class AActor *sender, FName eventName, EAIEventType eventType,
		                  FLOAT value=1.0, FLOAT radius=-1.0);
		void AIEndEvent(class AActor *sender, FName eventName, EAIEventType eventType);
		void AIClearEvent(class AActor *sender, FName eventName);

		void AIProcess(void);

		void DestroyActor(class AActor *actor);
		void Tick(FLOAT deltaSeconds);

	private:
		XAIEvent *FindEvent(EIntEventType eventType, FName eventName, class AActor *eventActor,
		                    XAIEventType **ppEventType=NULL, XAIEvent ***pppPrev=NULL);
		INT CompareEventNames(FName name1, FName name2);
		void ComputeSenseDetection(XAIReceiverEvent *pReceiver, XAISenderEvent *pSender,
		                           FLOAT &visibility, FLOAT &volume, FLOAT &strength);
		void AIEvent(class AActor *actor, FName eventName, EAIEventState eventState,
		             XAIParams &params, FName callback);
		FLOAT AIComputeScore(class AActor *receiver, class AActor *sender, FName scoreCallback);
		void SafeDelete(XAIEvent *pEvent);
		void CleanupEvents(void);
		void CleanupSlot(XAIEvent **table);

		XAIReceiverEvent *GetFirstProcess(void)
		{
			XAIReceiverEvent *pReceiver = firstProcess;
			if (pReceiver)
			{
				if (pReceiver->bBeingDestroyed)
				{
					pReceiver = pReceiver->GetNextProcess();
					if (pReceiver == firstProcess)
						pReceiver = NULL;
				}
			}
			return (pReceiver);
		}

		INT GetEventHash(FName eventName)  { return ((INT)appStrihash(*eventName)); }

	public:
		// Native routines (called from UnrealScript)

};  // UEventManager


#endif // _UN_EVENT_MANAGER_H_
