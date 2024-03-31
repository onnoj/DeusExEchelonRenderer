// ----------------------------------------------------------------------
//  File Name   :  DeusExTextParser.h
//  Programmer  :  Albert Yarusso
//  Description :  Augmented strings
// ----------------------------------------------------------------------
//  Copyright ©1999 ION Storm, L.P.  This software is a trade secret.
// ----------------------------------------------------------------------

#ifndef _DEUSEXTEXT_PARSER_H_
#define _DEUSEXTEXT_PARSER_H_

#include "Extension.h"

// DeusEx Text Tags
enum EDeusExTextTags
{
	TT_Text				= 0,
	TT_File				= 1,
	TT_Email			= 2,
	TT_Note				= 3,
	TT_EndNote			= 4,
	TT_Goal				= 5,
	TT_EndGoal			= 6,
	TT_Comment			= 7,
	TT_EndComment		= 8,
	TT_PlayerName		= 9,
	TT_PlayerFirstName  = 10,
	TT_NewPage			= 11,
    TT_CenterText		= 12,
	TT_LeftJustify		= 13,
	TT_RightJustify		= 14,
	TT_DefaultColor		= 15,
	TT_TextColor		= 16,
	TT_RevertColor		= 17,
	TT_NewParagraph		= 18,
	TT_Bold				= 19,
	TT_EndBold			= 20,
	TT_Underline		= 21,
	TT_EndUnderilne		= 22,
	TT_Italics			= 23,
	TT_EndItalics		= 24,
	TT_Graphic			= 25,
	TT_Font				= 26,
	TT_Label			= 27,
	TT_OpenBracket		= 28,
	TT_CloseBracket		= 29,
	TT_None				= 30
};

// ----------------------------------------------------------------------
// DDeusExTextParser class

class DEUSEXTEXT_API DDeusExTextParser : public UObject
{
	DECLARE_CLASS(DDeusExTextParser, UObject, 0)

	protected:
		TCHAR *text;
		TCHAR *textPos;						// Pointer to current text pos
		TCHAR *tagEndPos;					// Pointer to tag end position

		FString lastText;					// Last text block
		EDeusExTextTags lastTag;			// Last tag
		FName  lastName;					// Last name
		FColor lastColor;					// Last color
		FColor defaultColor;				// Last default color
		BITFIELD bParagraphStarted:1;		// True if we've hit our first paragraph
		FString playerName;					// Player's name, used for TT_PlayerName tag
		FString playerFirstName;			// Player's first name, used for TT_PlayerFirstName tag

		FString lastEmailName;
		FString lastEmailSubject;
		FString lastEmailFrom;
		FString lastEmailTo;
		FString lastEmailCC;
		FString lastFileName;
		FString lastFileDescription;

	public:
		// DDeusExTextParser interface
		DDeusExTextParser();
		void Destroy();

		// Interface
		bool OpenText(FName textName, FString textPackage);
		void CloseText(void);
		void SetPlayerName(FString playerName);
		void SetPlayerFirstName(FString playerFirstName);
		bool ProcessText(void);
		bool IsEOF(void);
		FString GetText(void);
		bool GotoLabel(FString &label);
		EDeusExTextTags GetTag(void);
		FName GetName(void);
		FColor GetColor(void);
		void GetEmailInfo(
			FString emailName,
			FString emailSubject,
			FString emailFrom,
			FString emailTo,
			FString emailCC);
		void GetFileInfo(
			FString fileName,
			FString fileDescription);

	private:
		FString ParseTextBlock(void);
		FString ParseTag(void);
		FString ParseToken(EDeusExTextTags tag, TCHAR *tokenStart);
		void ParseFile(TCHAR *tokenStart);
		void ParseEmail(TCHAR *tokenStart);
		void ParseName(TCHAR *tokenStart);
		void ParseColor(TCHAR *tokenStart);
		FString FindEndTag(EDeusExTextTags endTag);
		TCHAR *ParseSubToken(TCHAR *&tokenStart);
		FString TrimSpaces(TCHAR *textStart);

	public:
		// Intrinsics
		DECLARE_FUNCTION(execOpenText)
		DECLARE_FUNCTION(execCloseText)
		DECLARE_FUNCTION(execSetPlayerName)
		DECLARE_FUNCTION(execProcessText)
		DECLARE_FUNCTION(execIsEOF)
		DECLARE_FUNCTION(execGetText)
		DECLARE_FUNCTION(execGotoLabel)
		DECLARE_FUNCTION(execGetTag)
		DECLARE_FUNCTION(execGetName)
		DECLARE_FUNCTION(execGetColor)
		DECLARE_FUNCTION(execGetEmailInfo)
		DECLARE_FUNCTION(execGetFileInfo)
};  // DDeusExTextParser


#endif // _DEUSEXTEXT_PARSER_H_

