// UnRenderIterator.h

struct ActorBuffer
{
	char Buffer[ sizeof(AActor) ];
};

/*-----------------------------------------------------------------------------
	URenderIterator.
-----------------------------------------------------------------------------*/

class ENGINE_API URenderIterator : public UObject
{
	DECLARE_CLASS(URenderIterator,UObject,0)

	INT				MaxItems;
	INT				Index;
	APlayerPawn*	Observer;

	// Constructor.
	URenderIterator();

	// URenderIterator interface
	virtual void Init( APlayerPawn* Camera );	//override to initialize subclass data (call Super required)
	virtual void First();
	virtual void Next();
	virtual bool IsDone();						//override to adjust iteration termination criteria (call Super recommended)
	virtual AActor* CurrentItem();				//override to adjust actor render properties based on Index (call Super recommended)
};

/*----------------------------------------------------------------------------
	FActorNode.
----------------------------------------------------------------------------*/

class ENGINE_API FActorNode
{
public:
	ActorBuffer ActorProxy;
	FActorNode* NextNode;

	// Constructors.
	FActorNode(): NextNode( NULL ) {}
	~FActorNode() 
	{
		if( NextNode != NULL )
			delete NextNode;
	}
};

// end of UnRenderIterator.h
