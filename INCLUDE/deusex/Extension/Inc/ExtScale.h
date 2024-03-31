
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtScale.h
//  Programmer  :  Scott Martin
//  Description :  Header file for scale manager window class
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_SCALE_H_
#define _EXT_SCALE_H_


// ----------------------------------------------------------------------
// Enumeration for repeat clicks (INTERNAL ONLY!)

enum ERepeatDirection
{
	REPEATDIR_None,
	REPEATDIR_Inc,
	REPEATDIR_Dec
};


// ----------------------------------------------------------------------
// XScaleWindow class

class EXTENSION_API XScaleWindow : public XWindow
{
	DECLARE_CLASS(XScaleWindow, XWindow, 0)
	NO_DEFAULT_CONSTRUCTOR(XScaleWindow)

	public:
		XScaleWindow(XWindow *parent);

		// Structors
		void Init(XWindow *parent);
		void CleanUp(void);
		void Serialize(FArchive &Ar);

	protected:
		BYTE          orientation;

		UTexture      *scaleTexture;
		UTexture      *thumbTexture;
		UTexture      *tickTexture;
		UTexture      *preCapTexture;
		UTexture      *postCapTexture;

		BITFIELD      bRepeatScaleTexture:1 GCC_PACK(4);
		BITFIELD      bRepeatThumbTexture:1;
		BITFIELD      bDrawEndTicks:1;

		BITFIELD      bStretchScale:1;
		BITFIELD      bSpanThumb:1;

		UTexture      *borderPattern GCC_PACK(4);

		FLOAT         scaleBorderSize;
		FLOAT         thumbBorderSize;

		FColor        scaleBorderColor;
		FColor        thumbBorderColor;

		BYTE          scaleStyle;
		BYTE          thumbStyle;
		BYTE          tickStyle;

		FColor        scaleColor;
		FColor        thumbColor;
		FColor        tickColor;

		FLOAT         scaleWidth;
		FLOAT         scaleHeight;
		FLOAT         thumbWidth;
		FLOAT         thumbHeight;
		FLOAT         tickWidth;
		FLOAT         tickHeight;
		FLOAT         preCapWidth;
		FLOAT         preCapHeight;
		FLOAT         postCapWidth;
		FLOAT         postCapHeight;

		FLOAT         startOffset;
		FLOAT         endOffset;

		FLOAT         marginWidth;
		FLOAT         marginHeight;

		INT           numPositions;
		INT           currentPos;
		INT           spanRange;
		INT           thumbStep;

		FLOAT         fromValue;
		FLOAT         toValue;
		FStringNoInit valueFmt;

		FLOAT         initialDelay;
		FLOAT         repeatRate;

		INT           initialPos;

		USound        *setSound;
		USound        *clickSound;
		USound        *dragSound;

	private:
		FLOAT         scaleX;
		FLOAT         scaleY;
		FLOAT         scaleW;
		FLOAT         scaleH;

		FLOAT         thumbX;
		FLOAT         thumbY;
		FLOAT         thumbW;
		FLOAT         thumbH;

		FLOAT         tickX;
		FLOAT         tickY;
		FLOAT         tickW;
		FLOAT         tickH;

		FLOAT         preCapXOff;
		FLOAT         preCapYOff;
		FLOAT         preCapW;
		FLOAT         preCapH;

		FLOAT         postCapXOff;
		FLOAT         postCapYOff;
		FLOAT         postCapW;
		FLOAT         postCapH;

		FLOAT         absStartScale;
		FLOAT         absEndScale;

		BITFIELD      bDraggingThumb:1 GCC_PACK(4);
		FLOAT         mousePos GCC_PACK(4);
		BYTE          repeatDir;
		FLOAT         remainingTime;

		TArray<FString> enumStrings;

	public:
		// XScaleWindow interface
		void  SetOrientation(EOrientation newOrientation);

		void  SetScaleTexture(UTexture *newTexture,
		                      FLOAT newWidth=0, FLOAT newHeight=0,
		                      FLOAT newStartOffset=0, FLOAT newEndOffset=0);
		void  SetThumbTexture(UTexture *newTexture,
		                      FLOAT newWidth=0, FLOAT newHeight=0);
		void  SetTickTexture(UTexture *newTexture, UBOOL bNewDrawEndTicks=TRUE,
		                     FLOAT newWidth=0, FLOAT newHeight=0);
		void  SetThumbCaps(UTexture *newPreCapTexture, UTexture *newPostCapTexture,
		                   FLOAT preCapWidth=0, FLOAT preCapHeight=0,
		                   FLOAT postCapWidth=0, FLOAT postCapHeight=0);

		void  EnableStretchedScale(UBOOL bNewStretch=TRUE);

		void  SetBorderPattern(UTexture *newBorder=NULL);
		void  SetScaleBorder(FLOAT newBorderSize=0,
		                     FColor newBorderColor=FColor(255,255,255));
		void  SetThumbBorder(FLOAT newBorderSize=0,
		                     FColor newBorderColor=FColor(255,255,255));

		void  SetScaleStyle(BYTE newStyle=STY_Normal);
		void  SetThumbStyle(BYTE newStyle=STY_Normal);
		void  SetTickStyle(BYTE newStyle=STY_Normal);

		void  SetScaleColor(FColor newColor=FColor(255, 255, 255));
		void  SetThumbColor(FColor newColor=FColor(255, 255, 255));
		void  SetTickColor(FColor newColor=FColor(255, 255, 255));

		void  SetScaleMargins(FLOAT newMarginWidth=0, FLOAT newMarginHeight=0);

		void  SetNumTicks(INT newTickRange);
		INT   GetNumTicks(void)  { return (numPositions); }

		void  SetThumbSpan(INT newThumbSpan=0);
		INT   GetThumbSpan(void);
		void  SetRanges(INT newThumbSpan, INT newNumTicks);
		void  SetTickPosition(INT newTickPosition);
		INT   GetTickPosition(void)  { return (currentPos); }

		void  SetValueRange(FLOAT newFromValue, FLOAT newToValue);
		void  SetValue(FLOAT newValue);
		FLOAT GetValue(void);
		void  GetValues(FLOAT *pFromValue, FLOAT *pToValue);

		void  SetValueFormat(const TCHAR *newFmt);
		TCHAR *GetValueString(void);

		void  SetEnumeration(INT tickPos, const TCHAR *newStr);
		void  ClearAllEnumerations(void);

		void  MoveThumb(EMoveThumb moveThumb);
		void  SetThumbStep(INT newStep);

		void  SetRepeatRates(FLOAT newInitialDelay=0.0, FLOAT newRepeatRate=0.0)
		{
			initialDelay = newInitialDelay;
			repeatRate   = newRepeatRate;
		}

		void ChangeScaleAttributes(void);
		void ChangeScalePosition(UBOOL bFinal=TRUE);

		void SetScaleSounds(USound *setSound=NULL, USound *clickSound=NULL, USound *dragSound=NULL);
		void PlayScaleSound(USound *sound, FLOAT volume=-1.0, FLOAT pitch=1.0);

		// XWindow interface callbacks
		void    ParentRequestedPreferredSize(UBOOL bWidthSpecified,  FLOAT &preferredWidth,
		                                     UBOOL bHeightSpecified, FLOAT &preferredHeight);
		void    ConfigurationChanged(void);
		void    Draw(XGC *gc);

		UBOOL   MouseButtonPressed(FLOAT pointX, FLOAT pointY, EInputKey button,
		                           INT numClicks);
		void    MouseMoved(FLOAT pointX, FLOAT pointY);
		UBOOL   MouseButtonReleased(FLOAT pointX, FLOAT pointY, EInputKey button,
		                            INT numClicks);
		void    Tick(FLOAT deltaSeconds);

	private:
		// Internal routines used by this class
		FLOAT TickToPixel(INT tick);
		INT   PixelToTick(FLOAT pixel);
		FLOAT TickToValue(INT tick);
		INT   ValueToTick(FLOAT value);

		void ComputeTextureSize(UTexture *texture,
		                        FLOAT codedWidth, FLOAT codedHeight,
		                        FLOAT borderSize,
		                        FLOAT &newWidth,  FLOAT &newHeight);
		void ComputeThumbConfig(void);
		void DrawScaleTexture(XGC *gc, UTexture *texture,
		                      FLOAT tX, FLOAT tY,
		                      FLOAT tW, FLOAT tH,
		                      UBOOL bRepeat, BYTE style,
		                      FColor color,
		                      FLOAT borderSize, FColor borderColor,
		                      UTexture *preCap=NULL, UTexture *postCap=NULL,
		                      FLOAT preXOff=0, FLOAT preYOff=0,
		                      FLOAT preW=0, FLOAT preH=0,
		                      FLOAT postXOff=0, FLOAT postYOff=0,
		                      FLOAT postW=0, FLOAT postH=0);

		void ChangeThumbPosition(INT newPos, UBOOL bForceAttEvent=FALSE, UBOOL bFinal=TRUE);

	public:
		// Intrinsic routines (called from UnrealScript)
		DECLARE_FUNCTION(execSetScaleOrientation)
		DECLARE_FUNCTION(execSetScaleTexture)
		DECLARE_FUNCTION(execSetThumbTexture)
		DECLARE_FUNCTION(execSetTickTexture)
		DECLARE_FUNCTION(execSetThumbCaps)
		DECLARE_FUNCTION(execEnableStretchedScale)
		DECLARE_FUNCTION(execSetBorderPattern)
		DECLARE_FUNCTION(execSetScaleBorder)
		DECLARE_FUNCTION(execSetThumbBorder)
		DECLARE_FUNCTION(execSetScaleStyle)
		DECLARE_FUNCTION(execSetThumbStyle)
		DECLARE_FUNCTION(execSetTickStyle)
		DECLARE_FUNCTION(execSetScaleColor)
		DECLARE_FUNCTION(execSetThumbColor)
		DECLARE_FUNCTION(execSetTickColor)
		DECLARE_FUNCTION(execSetScaleMargins)
		DECLARE_FUNCTION(execSetNumTicks)
		DECLARE_FUNCTION(execGetNumTicks)
		DECLARE_FUNCTION(execSetThumbSpan)
		DECLARE_FUNCTION(execGetThumbSpan)
		DECLARE_FUNCTION(execSetTickPosition)
		DECLARE_FUNCTION(execGetTickPosition)
		DECLARE_FUNCTION(execSetValueRange)
		DECLARE_FUNCTION(execSetValue)
		DECLARE_FUNCTION(execGetValue)
		DECLARE_FUNCTION(execGetValues)
		DECLARE_FUNCTION(execSetValueFormat)
		DECLARE_FUNCTION(execGetValueString)
		DECLARE_FUNCTION(execSetEnumeration)
		DECLARE_FUNCTION(execClearAllEnumerations)
		DECLARE_FUNCTION(execMoveThumb)
		DECLARE_FUNCTION(execSetThumbStep)
		DECLARE_FUNCTION(execSetScaleSounds)
		DECLARE_FUNCTION(execPlayScaleSound)

};  // XScaleWindow


#endif // _EXT_SCALE_H_

