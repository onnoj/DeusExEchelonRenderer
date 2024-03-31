
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtFlag.h
//  Programmer  :  Scott Martin and Albert Yarusso
//  Description :  Header file for Unreal flags
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_FLAG_H_
#define _EXT_FLAG_H_


// ----------------------------------------------------------------------
// XFlag class

class EXTENSION_API XFlag : public XExtensionObject
{
	DECLARE_CLASS(XFlag, XExtensionObject, 0)
	NO_DEFAULT_CONSTRUCTOR(XFlag)

	friend class XFlagBase;

	public:
		XFlag(XFlagBase *base, FName flagName, BYTE flagType, INT expiration = 0);

		// Structors
		void Destroy(void);

	private:
		FName     flagName;
		INT       flagHash;
		XFlagBase *flagBase;
		XFlag     *nextFlag;
		BYTE      flagType;
		INT       expiration;

	public:
		virtual void SetValue(void *pNewValue){};
		virtual void GetValue(void *pValue){};
		
		void SetExpiration(INT newExpiration);

};  // XFlag


// ----------------------------------------------------------------------
// XFlagBool class

class EXTENSION_API XFlagBool : public XFlag
{
	DECLARE_CLASS(XFlagBool, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagBool);

	public:
		XFlagBool(XFlagBase *base, FName flagName, UBOOL newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Bool, expiration), bValue(newVal) {}

	private:
		BITFIELD bValue:1 GCC_PACK(4);

	public:
		void SetValue(void *pNewValue)  { SetValue(*(UBOOL *)pNewValue);  }
		void GetValue(void *pValue)     { *(UBOOL *)pValue = GetValue();  }

		void  SetValue(UBOOL newValue)  { bValue = newValue; }
		UBOOL GetValue(void)            { return (bValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagBool


// ----------------------------------------------------------------------
// XFlagByte class

class EXTENSION_API XFlagByte : public XFlag
{
	DECLARE_CLASS(XFlagByte, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagByte);

	public:
		XFlagByte(XFlagBase *base, FName flagName, BYTE newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Byte, expiration), byteValue(newVal) {}

	private:
		BYTE byteValue;

	public:
		void SetValue(void *pNewValue)  { SetValue(*(BYTE *)pNewValue);  }
		void GetValue(void *pValue)     { *(BYTE *)pValue = GetValue();  }

		void SetValue(BYTE newValue)    { byteValue = newValue; }
		BYTE GetValue(void)             { return (byteValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagByte


// ----------------------------------------------------------------------
// XFlagInt class

class EXTENSION_API XFlagInt : public XFlag
{
	DECLARE_CLASS(XFlagInt, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagInt);

	public:
		XFlagInt(XFlagBase *base, FName flagName, INT newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Int, expiration), intValue(newVal) {}

	private:
		INT intValue;

	public:
		void SetValue(void *pNewValue)  { SetValue(*(INT *)pNewValue);  }
		void GetValue(void *pValue)     { *(INT *)pValue = GetValue();  }

		void SetValue(INT newValue)     { intValue = newValue; }
		INT  GetValue(void)             { return (intValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagInt


// ----------------------------------------------------------------------
// XFlagFloat class

class EXTENSION_API XFlagFloat : public XFlag
{
	DECLARE_CLASS(XFlagFloat, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagFloat);

	public:
		XFlagFloat(XFlagBase *base, FName flagName, FLOAT newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Float, expiration), floatValue(newVal) {}

	private:
		FLOAT floatValue;

	public:
		void SetValue(void *pNewValue)  { SetValue(*(FLOAT *)pNewValue);  }
		void GetValue(void *pValue)     { *(FLOAT *)pValue = GetValue();  }

		void  SetValue(FLOAT newValue)  { floatValue = newValue; }
		FLOAT GetValue(void)            { return (floatValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagFloat


// ----------------------------------------------------------------------
// XFlagName class

class EXTENSION_API XFlagName : public XFlag
{
	DECLARE_CLASS(XFlagName, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagName);

	public:
		XFlagName(XFlagBase *base, FName flagName, FName newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Name, expiration), nameValue(newVal) {}

	private:
		FName nameValue;

	public:
		void SetValue(void *pNewValue)  { SetValue(*(FName *)pNewValue);  }
		void GetValue(void *pValue)     { *(FName *)pValue = GetValue();  }

		void  SetValue(FName newValue)  { nameValue = newValue; }
		FName GetValue(void)            { return (nameValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagName


// ----------------------------------------------------------------------
// XFlagVector class

class EXTENSION_API XFlagVector : public XFlag
{
	DECLARE_CLASS(XFlagVector, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagVector);

	public:
		XFlagVector(XFlagBase *base, FName flagName, FVector newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Vector, expiration), vectorValue(newVal) {}

	private:
		FVector vectorValue;

	public:
		void SetValue(void *pNewValue)  { SetValue(*(FVector *)pNewValue);  }
		void GetValue(void *pValue)     { *(FVector *)pValue = GetValue();  }

		void    SetValue(FVector newValue)   { vectorValue = newValue; }
		FVector GetValue(void)               { return (vectorValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagVector


// ----------------------------------------------------------------------
// XFlagRotator class

class EXTENSION_API XFlagRotator : public XFlag
{
	DECLARE_CLASS(XFlagRotator, XFlag, 0);
	NO_DEFAULT_CONSTRUCTOR(XFlagRotator);

	public:
		XFlagRotator(XFlagBase *base, FName flagName, FRotator newVal, INT expiration = 0) :
			XFlag(base, flagName, FLAG_Rotator, expiration), rotatorValue(newVal) {}

	private:
		FRotator rotatorValue;

	public:
		void SetValue(void *pNewValue)  { SetValue(*(FRotator *)pNewValue);  }
		void GetValue(void *pValue)     { *(FRotator *)pValue = GetValue();  }

		void     SetValue(FRotator newValue)   { rotatorValue = newValue; }
		FRotator GetValue(void)                { return (rotatorValue);   }

		// Intrinsic routines (called from UnrealScript)

};  // XFlagRotator


#endif // _EXT_FLAG_H_
