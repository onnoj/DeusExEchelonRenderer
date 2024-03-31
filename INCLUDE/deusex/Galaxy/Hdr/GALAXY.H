/*ƒ- Internal revision no. 5.00b -ƒƒƒƒ Last revision at 15:52 on 29-09-1999 -ƒƒ

                        The 32 bit definition headerfile

                €€€ﬂﬂ€€€ €€€ﬂ€€€ €€€    €€€ﬂ€€€ €€€  €€€ €€€ €€€
                €€€  ﬂﬂﬂ €€€ €€€ €€€    €€€ €€€  ﬂ€€€€ﬂ  €€€ €€€
                €€€ ‹‹‹‹ €€€‹€€€ €€€    €€€‹€€€    €€     ﬂ€€€ﬂ
                €€€  €€€ €€€ €€€ €€€    €€€ €€€  ‹€€€€‹    €€€
                €€€‹‹€€€ €€€ €€€ €€€‹‹‹ €€€ €€€ €€€  €€€   €€€

                                MUSIC SYSTEM 
                This document contains confidential information
                     Copyright (c) 1993-99 Carlo Vogelsang

  ⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒø
  ≥€≤± COPYRIGHT NOTICE ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±≤€≥
  √ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¥
  ≥ This headerfile, GALAXY.H, is Copyright (c) 1993-99  by  Carlo Vogelsang. ≥
  ≥ You may not copy, distribute, duplicate or clone this file  in  any form, ≥
  ≥ modified or non-modified without written permission of the author. By un- ≥
  ≥ authorized copying this file you are violating laws and will be punished. ≥
  ≥ So don't do that and let us all live in peace..                           ≥
  ≥                                                                     Carlo ≥
  ¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒŸ
*/
#ifndef _GALAXY_H
#define _GALAXY_H

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")

#ifdef __cplusplus
extern "C" {
#endif

/* Define constants used for output driver */

#define GLX_AUTODETECT			   -1
#define GLX_NOSOUND					0
#define GLX_DIRECTSOUND				1
#define GLX_WAVEDRIVER				2

/* Define constants used for output format */

#define GLX_MONO 	   				0
#define GLX_STEREO					1

#define GLX_8BIT  					0
#define GLX_16BIT    				2

#define GLX_SHOLD					0
#define GLX_COSINE					4

#define GLX_2DAUDIO					0
#define GLX_3DAUDIO					8

/* Define constants used for output type */

#define GLX_NORMAL					0
#define GLX_G3D       				1
#define GLX_A3D						2
#define GLX_EAX						3
#define GLX_A3D2					4
#define GLX_EAX2					5

/* Define constants used for sample type */

#define GLX_DELTA					1
#define GLX_UNSIGNED				2
#define GLX_16BITSAMPLE				4
#define GLX_LOOPED					8
#define	GLX_BIDILOOP				16
#define GLX_PANNING					32
#define GLX_STEREOSAMPLE			64
#define GLX_ALWAYSLOOP				128
#define GLX_LPC						4096
#define GLX_IMAADPCM				8192
#define GLX_MPEGAUDIO				16384
#define GLX_COMPRESSED				(GLX_MPEGAUDIO|GLX_IMAADPCM)
#define GLX_STREAMINGAUDIO			32768

/* Define constants used for voice control */

#define GLX_OFF						0
#define GLX_ON						1

#define GLX_SETMODE     			0
#define GLX_SETVOLUME				1
#define GLX_GETSTATUS				2

/* Define constants used for music control */

#define	GLX_VOLSET          		0
#define	GLX_VOLFADE					1
#define GLX_VOLFADE2				2

#define GLX_SETPOSITION				1
#define GLX_SETPOSITION2			2
#define GLX_SETLOOPMODE				3

#define GLX_PREV				   -3
#define GLX_CURR				   -2
#define GLX_NEXT				   -1

#define GLX_NOLOOP					0
#define GLX_LOOP					1

/* Define constants used for sample control */

#define GLX_MINSMPPANNING			0
#define GLX_MIDSMPPANNING			16384
#define GLX_MAXSMPPANNING			32767
#define GLX_SURSMPPANNING			32768

#define GLX_MINSMPVOLUME			0
#define GLX_MAXSMPVOLUME			32767

#define GLX_MINSMPVELOCITY			0
#define GLX_MAXSMPVELOCITY			32767  

#define GLX_MINSMPEXPRESSION		0
#define GLX_MAXSMPEXPRESSION		32767  

#define GLX_DEFSMPFREQUENCY		   -1
#define GLX_DEFSMPPANNING		   -1
#define GLX_DEFSMPVOLUME		   -1

/* Define constants used for instrument control */

#define GLX_MININSPANNING			0
#define GLX_MIDINSPANNING			64
#define GLX_MAXINSPANNING			127

#define GLX_MININSVOLUME			0
#define GLX_MAXINSVOLUME			127

#define GLX_MININSVELOCITY			0
#define GLX_MAXINSVELOCITY			127  

#define GLX_MININSEXPRESSION		0
#define GLX_MAXINSEXPRESSION		127  

#define GLX_MININSCHORUS			0
#define GLX_MAXINSCHORUS			127

#define GLX_MININSREVERB			0
#define GLX_MAXINSREVERB			127

#define GLX_DEFINSPITCH				32768
#define GLX_DEFINSPANNING		   -1
#define GLX_DEFINSVOLUME		   -1
#define GLX_DEFINSVELOCITY		   -1
#define GLX_DEFINSCHORUS		   -1
#define GLX_DEFINSREVERB		   -1
#define GLX_DEFINSEXPRESSION	   -1
#define GLX_DEFINSMODULATION       -1

/* Define constants used for sample and instrument control */

#define GLX_AUTO					0

#define GLX_NORMAL  				0
#define GLX_LOCKED					1

#define GLX_NOCALLBACK				0
#define GLX_CALLBACK				2

#define GLX_NOPOSITIONAL			0
#define GLX_POSITIONAL				4

#define GLX_SLAVE					0
#define GLX_MASTER					8

/* Define constants used for loaders */

#define GLX_LOADFROMSTREAM			1
#define GLX_LOADFROMMEMORY			2
#define GLX_LOADFROMARCHIVE			4
#define GLX_LOADASSTREAMING			8

#define GLX_SAVETOSTREAM			1
#define GLX_SAVETOMEMORY			2
#define GLX_SAVETOARCHIVE			4

#define GLX_FOURCC_AE				'  EA'
#define GLX_FOURCC_AI				'  IA'
#define GLX_FOURCC_AM				'  MA'
#define GLX_FOURCC_AS				'  SA'

#define GLX_FOURCC_BUFF				'FFUB'
#define GLX_FOURCC_INIT				'TINI'
#define GLX_FOURCC_MEMO				'OMEM'
#define GLX_FOURCC_RVRB				'BRVR'
#define GLX_FOURCC_CHRS				'SRHC'
#define GLX_FOURCC_ORDR				'RDRO'
#define GLX_FOURCC_OUTP				'PTUO'
#define GLX_FOURCC_PATT				'TTAP'
#define GLX_FOURCC_TRAK				'KART'
#define GLX_FOURCC_STRM				'MRTS'
#define GLX_FOURCC_CHAN				'NAHC'
#define GLX_FOURCC_VOIC				'CIOV'
#define GLX_FOURCC_INST				'TSNI'
#define GLX_FOURCC_SAMP				'PMAS'

#define GLX_FOURCC_RIFF				'FFIR'
#define GLX_FOURCC_LIST				'TSIL'

/* Define maximum number of voices */

#define GLX_TOTALBANKS				2
#define GLX_TOTALCHANNELS			32
#define GLX_TOTALVOICES			    256
#define GLX_TOTALINSTR				128

/* Define constants used for error messages */

#define GLX_NULL					0
#define GLXERR_NOERROR				0
#define GLXERR_MUSICLOADED			1
#define GLXERR_MUSICPLAYING			2
#define GLXERR_NOMUSICLOADED		3
#define GLXERR_NOMUSICPLAYING		4
#define GLXERR_OUTPUTACTIVE			5
#define GLXERR_OUTPUTNOTACTIVE		6
#define GLXERR_OUTOFVOICES		    7
#define GLXERR_OUTOFINSTRUMENTS		8
#define GLXERR_OUTOFMEMORY			9
#define GLXERR_OUTOFPATTERNMEM		10
#define GLXERR_OUTOFSAMPLEMEM		11
#define GLXERR_DEVICEBUSY			12
#define GLXERR_DEVICENOWRITE		13
#define GLXERR_UNSUPPORTEDDEVICE	14
#define GLXERR_UNSUPPORTEDFORMAT	15
#define GLXERR_DAMAGEDFILE			16
#define GLXERR_BADPARAMETER			17
#define GLXERR_OUTOFSOURCEDATA		18
#define GLXERR_CANNOTSTOPVOICE		19
									
/* Type definitions used internally */

typedef unsigned long udword;                   
typedef signed long sdword;                     
typedef unsigned short uword;                   
typedef signed short sword;                     
typedef unsigned char ubyte;                    
typedef signed char sbyte;                      

/* Type definitions used externally */

typedef int	(__cdecl glxCallback)(void *Voice,void *Param1,int Param2);
typedef int (__cdecl glxEffect)(void *Data,void *Out,void *In,int Samples,int Flags);

/* Define structures used internally */

#pragma pack (push,1) 							/* Turn off alignment */

typedef struct
{
	ubyte		VibType;                        /* Vibrato waveform */
	uword		VibDelay;                       /* Vibrato start delay */
	sword		VibDepth;                       /* Vibrato depth (1/256 semi) */
	sword		VibSpeed;                       /* Vibrato speed (1/64 hz) */
	ubyte		TremType;                       /* Tremolo waveform */
	uword		TremDelay;                      /* Tremolo start delay */
	sword		TremDepth;                      /* Tremolo depth (1/32768 vol) */
	sword		TremSpeed;                      /* Tremolo speed (1/64 hz) */
	ubyte		VolFlag;    					/* Volume flag */
	ubyte		VolReserved;					/* Volume reserved */	  
	ubyte		VolSize;                        /* Volume points */
	ubyte		VolSustain;   					/* Volume sustain */
	ubyte		VolLS;       					/* Volume loop start */
	ubyte		VolLE;      					/* Volume loop end */
	struct
	{
		uword Time;
		sword Value;
	}			Volume[10];                     /* Volume envelope */
	uword		VolFadeOut;						/* Volume fade-out */
	ubyte		PitFlag;    					/* Pitch flag */
	ubyte		PitReserved;					/* Pitch reserved */	  
	ubyte		PitSize;                        /* Pitch points */
	ubyte		PitSustain;   					/* Pitch sustain */
	ubyte		PitLS;       					/* Pitch loop start */
	ubyte		PitLE;      					/* Pitch loop end */
	struct
	{
		uword Time;
		sword Value;
	}			Pitch[10];						/* Pitch envelope */
	uword		PitFadeOut;						/* Pitch fade-out */
	ubyte		PanFlag;    					/* Panning flag */
	ubyte		PanReserved;					/* Panning reserved */	  
	ubyte		PanSize;                        /* Panning points */
	ubyte		PanSustain;   					/* Panning sustain */
	ubyte		PanLS;       					/* Panning loop start */
	ubyte		PanLE;      					/* Panning loop end */
	struct
	{
		uword Time;
		sword Value;
	}			Panning[10];					/* Panning envelope */
	uword		PanFadeOut;						/* Panning fade-out */
} glxArti;	    								/* Articulation structure */

typedef struct
{
	udword		FourCC;                         /* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	void *		Handle;							/* Handle (LPDIRECTSOUNDBUFFER or LPWAVEHDR) */
	ubyte *		Data;							/* Actual audio data */
	udword		Length;                         /* Buffer length in bytes */
	udword		PlayPos;						/* Play position in bytes */
	udword		WritePos;						/* Write position in bytes */
	sdword		Premix;  						/* Premix in bytes */				
} glxBuffer;	                                /* Buffer structure */

typedef struct
{
	udword		FourCC;							/* FourCC value */
	udword		Size;							/* Actual data size */
} glxChunk;										/* Chunk structure */

typedef struct
{
	udword		FourCC;							/* FourCC value */
	udword		Size;							/* Actual data size */
	udword		Type;							/* Type value */
} glxForm; 										/* Chunk structure */

typedef struct
{
	float 		X;      						/* Left handed X Coordinate */
	float 		Y;   							/* Left handed Y Coordinate */
	float 		Z;   							/* Left handed Z Coordinate */
	float		W;								/* left handed W Coordinate (not used) */
} glxVector; 									/* Vector structure */

typedef struct
{
	udword		FourCC;                         /* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	ubyte *		Data;                           /* Actual data */
	udword		Length;							/* Block length in bytes */
	udword		DataPos;						/* Current position in block */
} glxMemory;									/* Memory structure */

typedef struct
{
	udword		FourCC;                         /* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	void *		Handle;							/* Handle (LPDIRECTSOUND or HWAVEOUT) */
	void *		Mixer;							/* Mixer (LPDIRECTSOUNDBUFFER or NULL) */
	void *		Listener;						/* Listener (LPDIRECTSOUND3DLISTENER or LPA3DLISTENER) */
	void *		Extensions;						/* Extensions (LPIA3D,LPIA3D3 or LPKSPROPERTYSET) */
	sbyte		Driver;							/* Output driver */
	ubyte		Format;							/* Output format */
	ubyte	    Type;							/* Driver type */
	ubyte		Reserved;						/* Reserved for future use */
} glxOutput;                                    /* Output structure */

typedef struct
{
	udword		FourCC;                         /* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	ubyte		Message[32];                    /* Name of sample */
	uword		Panning;						/* Panning position */
	uword		Volume;							/* Volume level */
	uword		Type;							/* Looping type/sample type */
	uword		Reserved;						/* Reserved for future use */
	udword		Length;							/* Length in samples */
	udword		LoopStart;						/* Loop starting point in samples*/
	udword		LoopEnd;						/* Loop ending point in samples */
	udword		C4Speed;						/* Framerate for C-4 in Hz */
	glxArti *	Articulation;                   /* Articulation structure */
	void *		Data;                           /* Sample data */
} glxSample;									/* Sample structure */

typedef struct
{
	udword      FourCC;                         /* FourCC value */
	udword      Size;                           /* Size of rest of structure */
	ubyte       Bank;                           /* Bank number */
	ubyte       Program;                        /* Program number */
	ubyte       Message[32];					/* Name of instrument */
	ubyte       Split[128];             		/* All keys, split points */
	glxArti     Articulation;					/* Articulation structure */
	uword       Samples;		 				/* Sample structures */
	glxSample   Sample[];                       /* Actual sample structures */
} glxInstrument;								/* Instrument structure */

typedef struct
{
	udword      FourCC;                         /* FourCC value */
	udword      Size;                           /* Size of rest of structure */
	udword		Flags;							/* Track flags */
	udword		Status;							/* Track status */
	udword		TimeToNext;						/* Time to next event */
	ubyte *		Cursor;							/* Play cursor */
	ubyte *		Events;							/* Event list */
} glxTrack;										/* Track structure */

typedef struct
{
	udword      FourCC;                         /* FourCC value */
	udword      Size;                           /* Size of rest of structure */
	udword		Tracks;							/* Tracks in pattern */
    glxTrack    Track[];						/* Actual tracks */
} glxPattern;									/* Pattern structure */

typedef struct
{
	udword		FourCC;							/* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	udword		StartTime;        				/* 0=Ins/-1=Stream/-2=Eff */
	ubyte		NoteNo;							/* Note number (&128=Key off) */
	ubyte		InstNo;							/* Instrument number */
	ubyte		SmpNo;							/* Sample number */
	ubyte		Active;           				/* 0=Stopped/1=Running */
	ubyte		Enabled;                		/* 0=Disabled/1=Enabled */
	ubyte		Flags;                  		/* Operation flags */
	uword		Volume;   						/* 0=Silent..32767=Max. */
	uword		Panning;                		/* 0=Left..32767=Right */
	uword		BasePanning;					/* 0=Left..32767=Right */
	uword		Velocity;                       /* 0=Silent..32767=Max. */
	uword		Expression;						/* 0=Silent..32767=Max. */
	uword		Chorus;							/* Chorus send 0..127 */
	uword		Reverb;							/* Reverb send 0..127 */
	glxArti *	InsArt;                         /* Pointer to articulation data */
	sword		InsVol;                         /* Volume envelope value */
	sword		InsVolStep;                     /* Volume envelope step */
	uword		InsVolTime;                     /* Volume envelope time */
	uword		InsVolFade;                     /* Volume envelope fade-out */
	ubyte		InsVolPoint;                    /* Volume envelope point */
	ubyte		InsPitPoint;                    /* Pitch envelope point */
	uword		InsPitFade;                     /* Pitch envelope fade-out */
	uword		InsPitTime;                     /* Pitch envelope time */
	sword		InsPitStep;                     /* Pitch envelope step */
	sword		InsPit;                         /* Pitch envelope value */
	sword		InsPan;                         /* Panning envelope value */
	sword		InsPanStep;                     /* Panning envelope step */
	uword		InsPanTime;                     /* Panning envelope time */
	uword		InsPanFade;                     /* Panning envelope fade-out */
	ubyte		InsPanPoint;                    /* Panning envelope point */
	ubyte		Vol;							/* Volume 0..127 */
	sbyte		VolSlide;						/* Volume slide step */
	ubyte		VolDest;						/* Destination volume 0..127 */
	glxSample *	SmpHdr;                         /* Pointer to sample header */
	udword		SmpPtr;							/* Linear address play int. */
	uword		SmpFrac;						/* Linear address play frac. */
	uword		SmpType;	       				/* Looping/sample type */
	udword		SmpStart;						/* Linear address start */
	udword		SmpLoopStart; 					/* Linear address lpstart */
	udword		SmpLoopEnd;                     /* Linear address lpend */
	udword		SmpEnd;							/* Linear address end */
	udword		SmpC4Speed;						/* Just like a piano */
	sdword		SmpPitch;						/* Samplingrate relative */
	uword		SmpVol;                 		/* Current sample volume */
	uword		SmpBaseVol;             		/* Current sample base volume */
	uword		SmpPeriod;						/* Period value */
	uword		SmpBasePeriod;					/* Period base value */
	sword		VibDepth;               		/* Vibrato depth */
	sword		VibSpeed;                       /* Vibrato speed */
	uword		VibIndex;               		/* Vibrato time index */
	ubyte		VibWaveType;            		/* Vibrato waveform */
	ubyte		TremWaveType;           		/* Tremolo waveform */
	uword		TremIndex;              		/* Tremolo time index */
	sword		TremSpeed;                      /* Tremolo speed */
	sword		TremDepth;              		/* Tremolo depth */
	uword		PortaDest;            			/* Portamento dest. */
	uword		PortaSpeed;    					/* Portamento speed */
	sword		BenderValue;                    /* Pitch bend value */
	ubyte		Vibrato;       					/* Vibrato settings */
	ubyte		Tremolo;             			/* Tremolo settings */
	ubyte		Portamento;                     /* Portamento settings */
	ubyte		SampleOffset;             		/* Sample offset */
	ubyte		Command;						/* Command number */
	ubyte		CommandValue;					/* Command data value */
	udword		Custom1;						/* Custom data */
	udword		Custom2;						/* Custom data */
	udword		Custom3;						/* Custom data */
	udword		Custom4;						/* Custom data */
	void *		Mutex;							/* Voice mutex */
	void *		Link;							/* Linked voices */
} glxVoice;										/* Voice structure */

typedef struct
{
	udword		FourCC;							/* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	uword		Flags;							/* Operation flags */
	uword		Bank;							/* Current bank */
	uword		Program;						/* Program number */
	sword		Pitchwheel;						/* Pitchwheel value */
	sword 		PitchwheelSens;					/* Sensitivity (256 units/semitone) */
	uword 		Panning;						/* Panning */
	uword 		Modulation;						/* Modulation */
	uword 		Expression;						/* Expression */
	uword 		Volume;							/* Volume */
	uword		Hold1;							/* Hold1 */
	uword 		Reverb;							/* Reverb */
	uword 		Chorus;							/* Chorus */
	uword 		RPN;							/* Registered Parameter Number */
	uword 		NRPN;							/* Non-registered Parameter Number */
	uword 		Data;							/* Data entry (for RPN/NRPN) */
	char		Hold1Map[128];					/* Hold1/Key map */
	glxVoice *  VoiceMap[128];					/* Voice/Key map */
} glxChannel;									/* Channel structure */

typedef struct
{
	udword		FourCC;                         /* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	float		Time;							/* Decay time -60dB */
	float		Volume;							/* Overall volume */
	float		HFDamp;                         /* HFCutoff in Hz */
	struct
	{
		float Time;                             /* Time 0 to 0.340Sec */
		float Gain;                             /* Gain 0 to 1.0 */
	}			Direct[4];                      /* Direct layout */
	struct
	{
		float Time;                             /* Time 0 to 0.340Sec */
		float Gain;                             /* Gain 0 to 1.0 */
	}			Delay[6];                       /* Delay layout */
	void *		Data;    						/* Engine data */
	glxEffect *	Code;							/* Engine code */
} glxReverb;									/* Reverb structure */

typedef struct
{
	udword		FourCC;                         /* FourCC value */
	udword		Size;                           /* Size of rest of structure */
	float		Time;							/* Not used for chorus */
	float		Volume;							/* Overall volume */
	float		HFDamp;                         /* HFCutoff in Hz */
	float		Delay;							/* Delay in 0.0 to 50.0 msec */
	float		Rate;							/* Rate 0.0 to 10.0 in Hz */
	float	    Depth;							/* Depth 0.0 to 1.0 */
	float		Feedback;						/* Feedback 0.0 to 1.0 */
	void *		Data;      						/* Engine data */
	glxEffect * Code;							/* Engine code */
} glxChorus;									/* Chorus structure */

#pragma pack (pop)								/* Default alignment */

/* Public external functions/procedures */

extern glxSample *	   __cdecl glxAllocateStreamingBuffer(char *Name,int Length,int Frequency,int Format);
extern int			   __cdecl glxControlChannel(int Channel,int Command,int Parameter1,int Parameter2);
extern int             __cdecl glxControlVoice(glxVoice *Voice,int Command,int Parameter1,int Parameter2);
extern int             __cdecl glxControlMusic(int Command,int Parameter);
extern int             __cdecl glxControlInstrument(glxVoice *Voice,int Pitchwheel,int Voume,int Velocity,int Expression,int Panning,int Modulation,int Reverb,int Chorus);
extern int             __cdecl glxControlSample(glxVoice *Voice,int Frequency,int Volume,int Panning);
extern int             __cdecl glxControlSample3D(glxVoice *Voice,int Frequency,int Volume,glxVector *Position,glxVector *Velocity);
extern int             __cdecl glxDeinit(void);
extern int             __cdecl glxDetectOutput(int Driver,int Flags);
extern int			   __cdecl glxFlushOutput(void);
extern int             __cdecl glxInfo(char **Version,char **Driver);
extern int             __cdecl glxInit(void);
extern int			   __cdecl glxLock(void);
extern int			   __cdecl glxLoadEffects(void *Stream,int Flags);
extern int             __cdecl glxLoadMusic(void *Stream,int Flags);
extern glxSample *     __cdecl glxLoadSample(void *Stream,int Flags);
extern int             __cdecl glxLoadInstrument(int Instrument,void *Stream,int Flags);
extern glxSample *	   __cdecl glxLoadInstrumentSample(int Instrument,int Sample,void *Stream,int Flags);
extern int			   __cdecl glxObstructSample3D(glxVoice *Voice,int Obstruction,int Occlusion);
extern int             __cdecl glxResetMusic(void);
extern int			   __cdecl glxSaveEffects(void *Stream,int Flags);
extern int             __cdecl glxSaveMusic(void *Stream,int Flags);
extern int             __cdecl glxSaveSample(glxSample *Sample,void *Stream,int Flags);
extern int             __cdecl glxSaveInstrument(int Instrument,void *Stream,int Flags);
extern int			   __cdecl glxSaveInstrumentSample(int Instrument,int Sample,void *Stream,int Flags);
extern int             __cdecl glxSetCallback(glxCallback *Function);
extern int             __cdecl glxSetCDAudioVolume(int Volume,int Speed);
extern int			   __cdecl glxSetMusicVoices(int VoiceCount);
extern int             __cdecl glxSetMusicVolume(int Volume,int Speed);
extern int             __cdecl glxSetSampleVoices(int VoiceCount);
extern int             __cdecl glxSetSampleVolume(int Volume,int Speed);
extern int             __cdecl glxSetIOInterface(void *newread,void *newseek,void *newtell,void *newwrite);
extern int             __cdecl glxSetMemInterface(void *newmalloc,void *newrealloc,void *newfree);
extern int			   __cdecl glxSetMusicChorus(glxChorus *Chorus);
extern int			   __cdecl glxSetMusicReverb(glxReverb *Reverb);
extern int			   __cdecl glxSetSampleChorus(glxChorus *Chorus);
extern int			   __cdecl glxSetSampleReverb(glxReverb *Reverb);
extern int			   __cdecl glxSetSampleReverb2(glxReverb *Reverb);
extern int             __cdecl glxStartCDAudio(int StartTrack,int EndTrack);
extern int             __cdecl glxStartMusic(void);
extern int             __cdecl glxStartOutput(void *Owner,unsigned int Rate,int Format,int Mixahead);
extern glxVoice *      __cdecl glxStartSample(int Voice,glxSample *Sample,int Frequency,int Volume,int Panning,int Flags);
extern glxVoice *      __cdecl glxStartSample3D(int Voice,glxSample *Sample,int Frequency,int Volume,glxVector *Position,glxVector *Velocity,int Flags);
extern glxVoice *      __cdecl glxStartInstrument(int Voice,int Instrument,int Key,int Pitchwheel,int Volume,int Velocity,int Expression,int Panning,int Modulation,int Reverb,int Chorus,int Flags);
extern int             __cdecl glxStopCDAudio(void);
extern int             __cdecl glxStopMusic(void);
extern int             __cdecl glxStopOutput(void);
extern int             __cdecl glxStopSample(glxVoice *Voice);
extern int			   __cdecl glxStopSample3D(glxVoice *Voice);
extern int             __cdecl glxStopInstrument(glxVoice *Voice);
extern int			   __cdecl glxUnlock(void);
extern int             __cdecl glxUnloadMusic(void);
extern int             __cdecl glxUnloadSample(glxSample *Sample);
extern int             __cdecl glxUnloadInstrument(int Instrument);
extern int			   __cdecl glxUnloadInstrumentSample(int Instrument,int Sample);
extern int			   __cdecl glxSetEAXManager(void *EaxMan);

/* Public external uninitialised variables */

extern glxOutput       glxAudioOutput;			/* Audiooutput structure */
extern glxBuffer       glxAudioBuffer;			/* Audiobuffer structure */
extern glxVoice		   glxVoices[256];			/* Voice structures */
extern glxChannel	   glxChannels[32];			/* Channel structures */
extern ubyte           glxSongName[32];         /* ASCIIZ Name of song */
extern ubyte           glxAuthorName[32];	    /* ASCIIZ Name of author */
extern ubyte           glxPlayerMode;			/* Period system etc. */
extern ubyte           glxMusicVoices;			/* Music voices to be used */
extern uword           glxInitialSpeed;	        /* Default 48 (Ticks per beat) */
extern uword           glxInitialTempo;	        /* Default 120 (Beats per min) */
extern uword           glxMinPeriod;  		    /* Minimum period allowed */
extern uword           glxMaxPeriod;  			/* Maximum period allowed */
extern ubyte           glxInitialPanning[256];	/* Initial panning data */
extern ubyte           glxEffectChannels;       /* Effect channels to be used */
extern ubyte           glxSongLength;		    /* Length of song base zero */
extern ubyte           glxOrders[256];			/* 256 Orders max. */
extern glxPattern *    glxPatterns[256];	    /* 256 Patterns max. */
extern glxInstrument * glxInstruments[2][128];	/* 2 Banks, 128 instruments/bank */
extern glxPattern *	   glxCurrentPtr;           /* Current data */
extern volatile uword  glxCurrentTime;          /* Current time */
extern volatile ubyte  glxPatternRow;           /* Pattern row */
extern volatile ubyte  glxPatternBreak;	        /* Pattern break flag */
extern volatile uword  glxCurrentTempo;	        /* Current beats per min */
extern volatile uword  glxCurrentSpeed;	        /* Current ticks per row */
extern volatile ubyte  glxCurrentOrder;	        /* Current order */
extern volatile udword glxCurrentTick;			/* Current tick */
extern volatile udword glxCurrentSample;		/* Current sample */
extern volatile sbyte  glxMusicVolume;			/* 128 Levels */
extern volatile sbyte  glxSampleVolume;	        /* 128 Levels */
extern volatile sbyte  glxCDVolume;             /* 128 Levels */
extern volatile ubyte  glxMusicEnabled;	        /* Music enabled flag */
extern volatile ubyte  glxOutputActive;         /* Output active flag */
extern volatile ubyte  glxMusicLooping;         /* Music looping flag */
extern volatile udword glxSamplingrate;	        /* Sampling/mixing rate */
extern glxCallback *   glxCallbackFunction;     /* Callback function */
extern ubyte		   glxMMXFound;             /* MMX Found flag */
extern ubyte		   glxK3DFound;				/* 3DNow! Found flag */
extern ubyte		   glxKNIFound;				/* KNI Found flag */
extern glxReverb       glxMusicReverb;			/* Reverb for music */
extern glxReverb       glxSampleReverb;			/* Reverb for samples */
extern glxChorus       glxMusicChorus;			/* Chorus for music */
extern glxChorus       glxSampleChorus;			/* Chorus for samples */

#ifdef __cplusplus
};
#endif

#endif
