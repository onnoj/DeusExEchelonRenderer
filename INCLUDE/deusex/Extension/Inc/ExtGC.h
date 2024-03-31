// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//  File Name   :  ExtGC.h
//  Programmer  :  Scott Martin
//  Description :  Header file for Unreal graphics contexts
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _EXT_GC_H_
#define _EXT_GC_H_


// ----------------------------------------------------------------------
// XClipRect - clipping rectangle class

struct EXTENSION_API XClipRect
{

	// Default constructor
	XClipRect()
	{
		originX    = 0;
		originY    = 0;
		clipX      = 0;
		clipY      = 0;
		clipWidth  = 0;
		clipHeight = 0;
	}

	// Other convenience constructors
	XClipRect(FLOAT newClipX,     FLOAT newClipY,
	          FLOAT newClipWidth, FLOAT newClipHeight)
	{
		originX    = newClipX;
		originY    = newClipY;

		clipX      = 0;
		clipY      = 0;
		clipWidth  = newClipWidth;
		clipHeight = newClipHeight;
	}

	XClipRect(FLOAT newOriginX,   FLOAT newOriginY,
	          FLOAT newClipX,     FLOAT newClipY,
	          FLOAT newClipWidth, FLOAT newClipHeight)
	{
		originX    = newOriginX;
		originY    = newOriginY;
		clipX      = newClipX;
		clipY      = newClipY;
		clipWidth  = newClipWidth;
		clipHeight = newClipHeight;
	}

	XClipRect(XClipRect &rect1, XClipRect &rect2)
	{
		originX    = rect1.originX;
		originY    = rect1.originY;
		clipX      = rect1.clipX;
		clipY      = rect1.clipY;
		clipWidth  = rect1.clipWidth;
		clipHeight = rect1.clipHeight;
		Intersect(rect2);
	}

	// Public methods
	void SetOrigin(FLOAT newOriginX, FLOAT newOriginY)
	{
		clipX   += (originX - newOriginX);
		clipY   += (originY - newOriginY);
		originX =  newOriginX;
		originY =  newOriginY;
	}

	void MoveOrigin(FLOAT newDeltaX, FLOAT newDeltaY)
	{
		SetOrigin(originX+newDeltaX, originY+newDeltaY);
	}

	void Intersect(FLOAT newClipX,     FLOAT newClipY,
	               FLOAT newClipWidth, FLOAT newClipHeight)
	{
		XClipRect tempRect(newClipX+originX, newClipY+originY, newClipWidth, newClipHeight);
		Intersect(tempRect);
	}

	void Intersect(XClipRect &newRect)
	{
		FLOAT fromX1, fromY1;
		FLOAT fromX2, fromY2;
		FLOAT toX1,   toY1;
		FLOAT toX2,   toY2;

		// Convert everything to absolute coordinates
		fromX1 = clipX         + originX;
		fromY1 = clipY         + originY;
		fromX2 = newRect.clipX + newRect.originX;
		fromY2 = newRect.clipY + newRect.originY;
		toX1   = fromX1        + clipWidth;
		toY1   = fromY1        + clipHeight;
		toX2   = fromX2        + newRect.clipWidth;
		toY2   = fromY2        + newRect.clipHeight;

		// Clip
		if (fromX1 < fromX2)
			fromX1 = fromX2;
		if (fromY1 < fromY2)
			fromY1 = fromY2;
		if (toX1 > toX2)
			toX1 = toX2;
		if (toY1 > toY2)
			toY1 = toY2;

		// Reconvert to origin of this object
		clipX      = fromX1 - originX;
		clipY      = fromY1 - originY;
		clipWidth  = toX1   - fromX1;
		clipHeight = toY1   - fromY1;
	}

	UBOOL HasArea(void)
	{
		return ((clipWidth > 0) && (clipHeight > 0));
	}

	// Members
	FLOAT originX;    // X origin of rectangle, in absolute coordinates
	FLOAT originY;    // Y origin of rectangle, in absolute coordinates

	FLOAT clipX;      // Leftmost edge of rectangle relative to origin
	FLOAT clipY;      // Topmost edge of rectangle relative to origin
	FLOAT clipWidth;  // Width of rectangle
	FLOAT clipHeight; // Height of rectangle

};


// ----------------------------------------------------------------------
// Internal structure used when word-wrapping text
// (should not be used by any class except XGC)

struct XTextState
{
	XTextState()
	{
		bBold           = FALSE;
		bPlaneSet       = FALSE;
		bCarriageReturn = FALSE;
		bUnderline      = FALSE;
	}
	UBOOL operator==(const XTextState &other) const
	{
		if (bBold != other.bBold)
			return (FALSE);
		else if (bUnderline != other.bUnderline)
			return (FALSE);
		else if (bPlaneSet != other.bPlaneSet)
			return (FALSE);
		else if (bPlaneSet && (plane != other.plane))
			return (FALSE);
		else
			return (TRUE);
	}
	friend class XGC;
	private:
		BYTE   bBold;
		BYTE   bPlaneSet;
		BYTE   bCarriageReturn;
		BYTE   bUnderline;
		FPlane plane;
};


// ----------------------------------------------------------------------
// XGC class

class EXTENSION_API XGC : public XExtensionObject
{
	DECLARE_CLASS(XGC, XExtensionObject, 0)

	friend class XWindow;

	public:
		XGC();
		XGC(XGC &copyGC);

		void Destroy(void);

	private:
		UCanvas   *canvas;

		XClipRect clipRect;

		BYTE      style;
		BITFIELD  bSmoothed:1 GCC_PACK(4);
		BITFIELD  bDrawEnabled:1;
		BITFIELD  bMasked:1;
		BITFIELD  bTranslucent:1;
		BITFIELD  bModulated:1;
		BITFIELD  bTextTranslucent:1;
		BITFIELD  polyFlags GCC_PACK(4);
		BITFIELD  textPolyFlags;

		FColor    tileColor;
		FPlane    tilePlane;

		FColor    textColor;
		FPlane    textPlane;
		UFont     *normalFont;
		UFont     *boldFont;
		UTexture  *underlineTexture;
		FLOAT     underlineHeight;
		FLOAT     baselineOffset;
		FLOAT     textVSpacing;
		BYTE      hAlign;
		BYTE      vAlign;
		BITFIELD  bWordWrap:1 GCC_PACK(4);
		BITFIELD  bParseMetachars:1;

		INT       hMultiplier GCC_PACK(4);
		INT       vMultiplier;

		BITFIELD  bFree:1 GCC_PACK(4);
		int       gcCount GCC_PACK(4);
		XGC       *gcStack;
		XGC       *gcFree;
		XGC       *gcOwner;

	public:
		void      SetCanvas(UCanvas *canvas);
		UCanvas   *GetCanvas(void)  { return (canvas); }

		void      SetMultipliers(INT hMult=1, INT vMult=1);

		void      SetClipRect(XClipRect &newClipRect);
		void      Intersect(XClipRect &intersectClipRect);
		void      Intersect(FLOAT clipX, FLOAT clipY,
		                    FLOAT clipWidth, FLOAT clipHeight);
		XClipRect GetClipRect(void)  { return (clipRect); }

		void      EnableSmoothing(UBOOL bNewSmoothing=TRUE);
		UBOOL     IsSmoothingEnabled(void)  { return (bSmoothed); }

		void      SetStyle(BYTE newStyle=STY_Normal);
		BYTE      GetStyle(void)  { return (style); }

		void      EnableDrawing(UBOOL bNewDrawEnabled=TRUE);
		UBOOL     IsDrawingEnabled(void)  { return (bDrawEnabled); }

		void      EnableMasking(UBOOL bNewMasking=TRUE);
		UBOOL     IsMaskingEnabled(void)  { return (bMasked); }

		void      EnableTranslucency(UBOOL bNewTranslucency=TRUE);
		UBOOL     IsTranslucencyEnabled(void)  { return (bTranslucent); }

		void      EnableModulation(UBOOL bNewModulation=TRUE);
		UBOOL     IsModulationEnabled(void)  { return (bModulated); }

		void      SetPolyFlags(DWORD newPolyFlags);
		DWORD     GetPolyFlags(void)  { return (polyFlags); }

		void      SetTileColor(FColor newTileColor);
		FColor    GetTileColor(void)  { return (tileColor); }

		void      SetTextColor(FColor newTextColor);
		FColor    GetTextColor(void)  { return (textColor); }

		void      EnableTranslucentText(UBOOL bNewTranslucency=TRUE);
		UBOOL     IsTranslucentTextEnabled(void)  { return (bTextTranslucent); }

		void      SetFont(UFont *newFont=NULL)  { SetFonts(newFont, newFont); }
		void      SetNormalFont(UFont *newFont=NULL);
		void      SetBoldFont(UFont *newFont=NULL);
		void      SetFonts(UFont *newNormalFont=NULL, UFont *newBoldFont=NULL);
		void      GetFonts(UFont **pNormalFont=NULL, UFont **pBoldFont=NULL)
		{
			if (pNormalFont)  *pNormalFont = normalFont;
			if (pBoldFont)    *pBoldFont   = boldFont;
		}

		void      SetBaselineData(FLOAT newBaselineOffset=2, FLOAT newUnderlineHeight=1);

		void      SetTextVSpacing(FLOAT newVSpacing=1);
		FLOAT     GetTextVSpacing(void)  { return (textVSpacing); }

		void      SetHorizontalAlignment(EHAlign newHAlign);
		EHAlign   GetHorizontalAlignment(void)  { return ((EHAlign)hAlign); }

		void      SetVerticalAlignment(EVAlign newVAlign);
		EVAlign   GetVerticalAlignment(void)  { return ((EVAlign)vAlign); }

		void      SetAlignments(EHAlign newHAlign, EVAlign newVAlign);
		void      GetAlignments(EHAlign *pHAlign=NULL, EVAlign *pVAlign=NULL);

		void      EnableWordWrap(UBOOL bNewWordWrap=TRUE);
		UBOOL     IsWordWrapEnabled(void)  { return (bWordWrap); }

		void      EnableSpecialText(UBOOL bNewSpecial=TRUE);
		UBOOL     IsSpecialTextEnabled(void)  { return (bParseMetachars); }

		void      CopyGC(XGC &copyGC);

		int       PushGC(void);
		void      PopGC(int gcNum);
		void      PopGC(void);

		void GetTextExtent(FLOAT destWidth, FLOAT &xExtent, FLOAT &yExtent,
		                   const TCHAR *textStr);
		FLOAT GetFontHeight(UBOOL bIncludeSpace=FALSE);
		void DrawText(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		              const TCHAR *textStr);
		void Printf(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		            const TCHAR *fmt, ...);

		void DrawTextLine(FLOAT destX, FLOAT destY, XTextState *textState,
		                  const TCHAR *textStr, INT lineSize=-1);
		UBOOL GetTextLine(const TCHAR *textStr, XTextState *textState=NULL,
		                  INT count=-1, FLOAT destWidth=0,
		                  const TCHAR **pNextLine=NULL, XTextState *pNewState=NULL,
		                  INT *pNumChars=NULL, FLOAT *pXExtent=NULL);
		FLOAT CharToPixel(const TCHAR *textStr, INT textPos, INT count=-1, XTextState *textState=NULL);
		INT   PixelToChar(const TCHAR *textStr, FLOAT pixel, INT count=-1, XTextState *textState=NULL);

		UBOOL DrawTextChar(FLOAT &destX, FLOAT &destY, XTextState *textState,
		                   const TCHAR *&textStr, INT &charCount);

		void DrawIconPattern(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		                     FLOAT srcX, FLOAT srcY, FLOAT srcWidth, FLOAT srcHeight,
		                     UTexture *texture);
		void DrawStretchedIcon(FLOAT destX, FLOAT destY,
		                       FLOAT destWidth, FLOAT destHeight, UTexture *texture);
		void DrawScaledIcon(FLOAT destX, FLOAT destY,
		                    FLOAT scaleX, FLOAT scaleY, UTexture *texture);
		void DrawIcon(FLOAT destX, FLOAT destY, UTexture *texture);
		void DrawPattern(FLOAT destX, FLOAT destY,
		                 FLOAT destWidth, FLOAT destHeight, FLOAT orgX, FLOAT orgY,
		                 UTexture *texture);
		void DrawTexture(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		                 FLOAT srcX, FLOAT srcY, UTexture *texture);
		void DrawStretchedTexture(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		                 FLOAT srcX, FLOAT srcY, FLOAT srcWidth, FLOAT srcHeight, UTexture *texture);

		void DrawBox(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		             FLOAT srcX, FLOAT srcY, FLOAT thickness, UTexture *texture);

		void DrawBorders(FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		                 FLOAT leftMargin, FLOAT rightMargin, FLOAT topMargin, FLOAT bottomMargin,
		                 UTexture *bordTL, UTexture *bordTR, UTexture *bordBL, UTexture *bordBR,
		                 UTexture *bordL, UTexture *bordR, UTexture *bordT, UTexture *bordB,
		                 UTexture *center=NULL,
		                 UBOOL bStretchHorizontally=FALSE, UBOOL bStretchVertically=FALSE);

		void DrawActor(class AActor *actor, UBOOL bClearZ=FALSE, UBOOL bConstrain=FALSE,
		               UBOOL bUnlit=FALSE, FLOAT drawScale=1.0, FLOAT scaleGlow=1.0,
		               UTexture *newSkin=NULL);

		void ClearZ(void);

		// Utility to get character size
		static void GetCharSize(UFont *font, TCHAR ch, INT *pWidth=NULL, INT *pHeight=NULL);

		// Cleanup routine
		static void CleanUp(void);

	protected:
		UBOOL GetColorByte(const TCHAR *&pStr, BYTE &byteVal, const TCHAR *&pEnd);
		void  ReadColor(const TCHAR *&pStr, FPlane &plane, const TCHAR *&pEnd);
		TCHAR GetNextChar(const TCHAR *&pStr, XTextState &textState, const TCHAR *&pEnd);
		void  ConvertPixelPos(const TCHAR *&pStr, INT &pos, FLOAT &pixel, UBOOL bDir,
		                      INT count, XTextState *pTextState);

		UBOOL ParseLine(const TCHAR *textStr, XTextState textState,
		                const TCHAR *pEnd, FLOAT destWidth,
		                const TCHAR **pNextLine, XTextState *pNewState,
		                INT *pCount, FLOAT *pXExtent);
		const TCHAR *GetLine(XTextState &textState, FLOAT destWidth,
		                     const TCHAR *&textStr, const TCHAR *pEnd,
		                     FLOAT &xExtent, INT &count);

		void DrawChar(UFont *font, TCHAR ch, FPlane &plane, BYTE &bUnderline,
		              FLOAT destX, FLOAT destY, FLOAT *pXOffset=NULL, FLOAT *pHeight=NULL);

		void ClipTile(UTexture *texture,
		              FLOAT &destX, FLOAT &destY, FLOAT &destWidth, FLOAT &destHeight,
		              FLOAT &srcX, FLOAT &srcY, FLOAT &srcWidth, FLOAT &srcHeight,
		              UBOOL bWrapHorizontal, UBOOL bWrapVertical);

		void DrawTile(UTexture *texture,
		              FLOAT destX, FLOAT destY, FLOAT destWidth, FLOAT destHeight,
		              FLOAT srcX, FLOAT srcY, FLOAT srcWidth, FLOAT srcHeight,
		              UBOOL bWrapHorizontal, UBOOL bWrapVertical);

	private:
		void      GenerateStyle(void);
		void      GeneratePolyFlags(void);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execIntersect)

		DECLARE_FUNCTION(execEnableSmoothing)
		DECLARE_FUNCTION(execIsSmoothingEnabled)
		DECLARE_FUNCTION(execSetStyle)
		DECLARE_FUNCTION(execGetStyle)
		DECLARE_FUNCTION(execEnableDrawing)
		DECLARE_FUNCTION(execIsDrawingEnabled)
		DECLARE_FUNCTION(execEnableMasking)
		DECLARE_FUNCTION(execIsMaskingEnabled)
		DECLARE_FUNCTION(execEnableTranslucency)
		DECLARE_FUNCTION(execIsTranslucencyEnabled)
		DECLARE_FUNCTION(execEnableModulation)
		DECLARE_FUNCTION(execIsModulationEnabled)

		DECLARE_FUNCTION(execSetTileColor)
		DECLARE_FUNCTION(execGetTileColor)

		DECLARE_FUNCTION(execSetTextColor)
		DECLARE_FUNCTION(execGetTextColor)
		DECLARE_FUNCTION(execEnableTranslucentText)
		DECLARE_FUNCTION(execIsTranslucentTextEnabled)
		DECLARE_FUNCTION(execSetFont)
		DECLARE_FUNCTION(execSetNormalFont)
		DECLARE_FUNCTION(execSetBoldFont)
		DECLARE_FUNCTION(execSetFonts)
		DECLARE_FUNCTION(execGetFonts)

		DECLARE_FUNCTION(execSetTextVSpacing)
		DECLARE_FUNCTION(execGetTextVSpacing)
		DECLARE_FUNCTION(execSetHorizontalAlignment)
		DECLARE_FUNCTION(execGetHorizontalAlignment)
		DECLARE_FUNCTION(execSetVerticalAlignment)
		DECLARE_FUNCTION(execGetVerticalAlignment)
		DECLARE_FUNCTION(execSetAlignments)
		DECLARE_FUNCTION(execGetAlignments)
		DECLARE_FUNCTION(execEnableWordWrap)
		DECLARE_FUNCTION(execIsWordWrapEnabled)
		DECLARE_FUNCTION(execEnableSpecialText)
		DECLARE_FUNCTION(execIsSpecialTextEnabled)
		DECLARE_FUNCTION(execSetBaselineData)

		DECLARE_FUNCTION(execCopyGC)
		DECLARE_FUNCTION(execPushGC)
		DECLARE_FUNCTION(execPopGC)

		DECLARE_FUNCTION(execGetTextExtent)
		DECLARE_FUNCTION(execGetFontHeight)
		DECLARE_FUNCTION(execDrawText)
		DECLARE_FUNCTION(execDrawIcon)
		DECLARE_FUNCTION(execDrawTexture)
		DECLARE_FUNCTION(execDrawPattern)
		DECLARE_FUNCTION(execDrawBox)
		DECLARE_FUNCTION(execDrawStretchedTexture)
		DECLARE_FUNCTION(execDrawActor)
		DECLARE_FUNCTION(execDrawBorders)

		DECLARE_FUNCTION(execClearZ)

};


#endif  // _EXT_GC_H_
