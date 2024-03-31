/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2 $
** $Date: 6/17/97 10:58a $
**
*/

#ifndef _FXIMG_H_
#define _FXIMG_H_

#ifdef __DOS__
#define READ_ATTRIBS "rb"
#define WRITE_ATTRIBS "wb"
#elif defined( _WIN32 )
#define READ_ATTRIBS "rb"
#define WRITE_ATTRIBS "wb"
#else
#define READ_ATTRIBS "r"
#define WRITE_ATTRIBS "w" 
#endif

#define IMAGE_PATH "PATH_IMAGE"

typedef enum imgTxColorFormatEnum
{
  txColorUnknown,              /* Color format has not yet been specified.                    */
  txColorTrue,                 /* True color, 24-bit color (RGB) + 8-bit alpha.               */
  txColorI_8,                  /* 8-bit intensity.                                            */
  txColorA_8,                  /* 8-bit alpha.                                                */
  txColorAI_44,                /* 4-bit alpha, 4-bit intensity.                               */
  txColorYIQ,                  /* Narrow-channel, compressed color.                           */
  txColorRGB_332,              /* 3-bit red, 3-bit green, 2-bit blue.                         */
  txColorRGB_565,              /* 5-bit red, 6-bit green, 5-bit blue.                         */
  txColorARGB_8332,            /* 8-bit alpha, 3-bit red, 3-bit green, 2-bit blue.            */
  txColorARGB_1555,            /* 1-bit alpha, 5-bit red, 5-bit green, 5-bit blue.            */
  txColorAYIQ_8422,            /* 8-bit alpha + narrow-channel compressed color.              */
  txColorARGB_4444,            /* 4-bit alpha, 4-bit red, 4-bit green, 4-bit blue.            */
  txColorAI_88,                /* 8-bit alpha, 8-bit intensity.                               */
  txColorARGB_8888             /* 8-bits of Alpha, Red, Green, and Blue.  Not yet supported.  */
} imgTxColorFormat;     


typedef enum { TOP, BOTTOM } YOriginT;
typedef enum { IMG_UNKNOWN, IMG_SBI, IMG_P6, IMG_3DF, IMG_RGT, IMG_TGA32 , IMG_SRLE } ImgType;

extern const char *imgTxColorFormatNames[];
extern const char *imgTypeString[];

// Internal Memory Format BGRA ( Least Sig Byte -> Most Sig Byte )
// Memory image has it's origin in the upper left always.
typedef unsigned char ImgData;

typedef struct 
{
    ImgType  type;
    FxU32    width;
    FxU32    height;
    FxU32    sizeInBytes;
    ImgData  *data;
    FxU32    yOrigin;     // 0 - Lower 1 - Upper
    FxU32    redBits;
    FxU32    greenBits;
    FxU32    blueBits;
} SbiInfo;

typedef struct
{
    ImgType  type;
    FxU32 width;
    FxU32 height;
    FxU32 sizeInBytes;
    ImgData  *data;
    FxU32 colorChannelMax;
} P6Info;

typedef struct
{
    ImgType  type;
    FxU32 width;
    FxU32 height;
    FxU32 sizeInBytes;
    ImgData  *data;
	float version;
	FxU32 colorFormat;
	FxU32 lodMin;
	FxU32 lodMax;
	FxU32 aspectWidth;
	FxU32 aspectHeight;
} TdfInfo;

typedef struct
{
    ImgType  type;
    FxU32 width;
    FxU32 height;
    FxU32 sizeInBytes;
    ImgData  *data;
    FxU32 rle;
    FxU32 bgr;
    FxU32 rgb;
    FxU32 ncc;
} RgtInfo;

typedef struct
{
    ImgType  type;
    FxU32 width;
    FxU32 height;
    FxU32 sizeInBytes;
    ImgData  *data;
    FxU32 yOrigin;   // 0 - Lower Left, 1 - Upper Left
} TgaInfo;

typedef struct
{
    ImgType  type;
    FxU32 width;
    FxU32 height;
    FxU32 sizeInBytes;
    ImgData  *data;
    FxU32 pixelsize;
    FxU32 c_type;
} SrleInfo;

typedef struct
{
    ImgType  type;
    FxU32 width;
    FxU32 height;
    FxU32 sizeInBytes;
    ImgData  *data;
} AnyInfo;

// This is the only structure intended to be referenced
// by a user.
typedef union
{
    AnyInfo any;
    SbiInfo sbiInfo;
    P6Info  p6Info;
    TdfInfo tdfInfo;
    RgtInfo rgtInfo;
    TgaInfo tgaInfo;
    SrleInfo srleInfo;
} ImgInfo;

const char *imgGetErrorString( void );
const char *imgTypeName( const ImgInfo *info );
FxBool imgReadInfo( FILE *stream, ImgInfo *info );
FxBool imgReadData( FILE *stream, ImgInfo *info );
FxBool imgReadFile(const char *filename, ImgInfo *info);
FxBool imgWriteImage( FILE *stream, const ImgInfo *info, const ImgType type, const ImgData *data);
FxBool imgWriteFile( const char *filename, const ImgInfo *info, const ImgType type, const ImgData *data);

#endif
