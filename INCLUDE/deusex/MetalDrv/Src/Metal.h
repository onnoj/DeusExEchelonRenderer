/********************************************************************************/
/*																				*/
/* MTL API																		*/
/*																				*/
/* metal.h																		*/
/*																				*/
/* Main (public) MTL header.													*/
/*																				*/
/********************************************************************************/
/*																				*/
/* REVISION HISTORY																*/
/*																				*/
/* 0.1	18th Nov 97		Martin Hoffesommer		File created.					*/
/* 0.2	27th Aug 98		Derek Gladding			Reformatted and commented.		*/
/* 0.3	11th Sep 98		KC Li					Added 720x ... modes			*/
/* 0.4  14th Sep 98		Derek Gladding			Slight change to above for		*/
/*												cleaner fit with implementation	*/
/*																				*/
/********************************************************************************/
/*																				*/
/* NOTES																		*/
/*																				*/
/********************************************************************************/

#ifndef __METAL_H
#define __METAL_H

////
//// COMMON MACROS
//// =============
////

/// sanity check for OS type
/// ------------------------
/// only one of the following macros may be defined:
/// __WIN95__, __WINNT__, __DOS__

#ifdef __WIN95__
  #define __WIN32__ 1
  #undef __WINNT__
  #undef __DOS__
#endif // __WIN95__

#ifdef __WINNT__
  #define __WIN32__ 1
  #undef __WIN95__
  #undef __DOS__
#endif // __WINNT__

#ifdef __DOS__
  #undef __WIN32__
#endif // __DOS__

#if !defined(__WIN95__) && !defined(__WINNT__) && !defined(__DOS__)
  #error target OS must be defined
#endif

// include WINDOWS.H for WIN32 apps
#ifdef __WIN32__
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #define UNDEF_WIN32_LEAN_AND_MEAN
  #endif
  #include "windows.h"
  #ifdef UNDEF_WIN32_LEAN_AND_MEAN
    #undef WIN32_LEAN_AND_MEAN
    #undef UNDEF_WIN32_LEAN_AND_MEAN
  #endif
#endif // __WIN32__

/// interface helper
/// ----------------
#ifdef __WIN32__
  #define MTL_DLLTYPE   __declspec(dllexport)
  #define MTL_CALLTYPE  __cdecl               // use cdecl calling convention
#endif

#ifdef __DOS__
  #define MTL_DLLTYPE
  #define MTL_CALLTYPE
#endif

// help macros for MTL_CONTEXT definition
// either for C or C++
#if defined(__cplusplus) && !defined(MTL_INTERNAL)
  #define MTL_INTERFACE         struct _MTL_CONTEXT 
  #define MTL_FUNCTION_(ret,n)  virtual ret MTL_CALLTYPE n
  #define MTL_THIS_             void
  #define MTL_THIS
  #define MTL_PURE              =0
#else // !_cplusplus
  #ifdef MTL_INTERNAL // internal CONTEXT definition
    #define MTL_INTERFACE         struct MTL_CONTEXT_FUNC
  #else // !MTL_INTERNAL: standard CONTEXT definition
    #define MTL_INTERFACE         typedef struct _MTL_CONTEXT \
                                  { \
                                    const struct MTL_CONTEXT_FUNC *func; \
                                  } _MTL_CONTEXT; \
                                  struct MTL_CONTEXT_FUNC
  #endif // !MTL_INTERNAL
  #define MTL_FUNCTION_(ret,n)  ret (MTL_CALLTYPE *n)
  #define MTL_THIS_             MTL_CONTEXT This
  #define MTL_THIS              MTL_THIS_ ,
  #define MTL_PURE
#endif

#define MTL_FUNCTION(n)       MTL_FUNCTION_(MTL_RESULT,n)

// Sorry this looks that messy but it allows to use the same code for C and
// C++. While using the context in C you are calling each function through the
// function list, in C++ you are calling each function as a class member.
//
// Sample:
//    MTL_INTERFACE
//    {
//      MTL_FUNCTION(mtlDummy)(MTL_THIS int dummy) MTL_PURE:
//    };
//
// C++ expansion:
//    struct _MTL_CONTEXT
//    {
//      virtual MTL_RESULT MTL_CALLTYPE mtlDummy(int dummy)=0;
//    };
//
// C expansion:
//    typedef struct _MTL_CONTEXT
//    {
//      const struct MTL_CONTEXT_FUNC *func;
//    }
//
//    struct MTL_CONTEXT_FUNC
//    {
//      MTL_RESULT (MTL_CALLTYPE *mtlDummy)(_MTL_CONTEXT *This, int dummy);
//    };
//
// Calling:
//    MTL_CONTEXT ctx;
//
//    C-style:
//      ctx->func->mtlDummy(ctx,1);
//    C++-style:
//      ctx->mtlDummy(1);

/// macro for handle definition (type safe)
/// ---------------------------------------
#ifdef MTL_INTERNAL
  #define MTL_DECLARE_HANDLE(n)
#else
  #define MTL_DECLARE_HANDLE(n) struct __##n { int dummy; }; \
                                typedef struct __##n *n;
#endif // MTL_INTERNAL

/// macros for parameter direction indication
/// -----------------------------------------

#define MTL_IN    // in parameter
#define MTL_OUT   // out parameter
#define MTL_INOUT // in&out parameter

////
//// METAL DATA TYPES
//// ================
////

// result of a function call: ok or fail
typedef enum
{
  MTL_FAILED  = 0,
  MTL_OK
} MTL_RESULT;

// MeTaL error codes
typedef enum
{
	MTL_ERR_NONE      = 0,		// no error

	// init related
	MTL_ERR_NO_S3D,				// board is no S3D board
	MTL_ERR_UNSUPPORTED_MODE,	// unsupported video mode
	MTL_ERR_NO_SYS_MEMORY,		// out of system memory (e.g. for creating new context)
	MTL_ERR_NO_VIDEO_MEMORY,	// out of video memory (e.g. for allocating buffers)

	// state related
	MTL_ERR_INVALID_STATE,		// the given state key is invalid

	// vertex buffer
	MTL_VB_INUSE,				// vertex buffer can't be locked because it is still in use

	// texture related
	MTL_ERR_LOST_TEXTURE,		// being returned if the texture selected with mtlSetState
								// has lost its surface; data should be re-uploaded using
								// mtlUpdateTexture

	// misc
	MTL_ERR_INTERNAL,			// unspecified internal error
	MTL_ERR_INACTIVE			// BeginDraw called while the Metal app is minimised

} MTL_ERROR;

// MeTaL simple types
typedef float MTL_VALUE;
typedef unsigned char MTL_BYTE;
typedef unsigned short MTL_WORD;
typedef unsigned MTL_DWORD;
#define MTL_FALSE           ((MTL_DWORD)0)
#define MTL_TRUE            ((MTL_DWORD)1)

// MeTaL color structure
typedef struct
{
  MTL_BYTE b;     // blue component
  MTL_BYTE g;     // green component
  MTL_BYTE r;     // red component
  MTL_BYTE af;    // alpha component or fog value
} MTL_COLOR_STRUCT;

// MeTaL color type
typedef MTL_DWORD MTL_COLOR;

// MeTaL vertex (binary identical to D3DTLVERTEX)
typedef struct
{
  MTL_VALUE sx;
  MTL_VALUE sy;
  MTL_VALUE sz;
  MTL_VALUE rhw;
  union
  {
    MTL_COLOR color;
    MTL_COLOR_STRUCT sColor;
  };
  union
  {
    MTL_COLOR specular;
    MTL_COLOR_STRUCT sSpecular;
  };
  MTL_VALUE tu;
  MTL_VALUE tv;
} MTL_VERTEX, *MTL_PVERTEX;


typedef enum
    {
    MTL_VTX_X = 0x01,               // X field present in vertex
    MTL_VTX_Y = 0x02,               // Y field present in vertex
    MTL_VTX_Z = 0x04,               // Z field present in vertex
    MTL_VTX_RHW = 0x08,             // RHW field present in vertex
    MTL_VTX_DIFFUSE = 0x10,         // Diffuse lighting field present in vertex
    MTL_VTX_SPECULAR = 0x20,        // Specular lighting field present in vertex
    MTL_VTX_U0 = 0x40,              // First U co-ordinate present in vertex
    MTL_VTX_V0 = 0x80,              // First V co-ordinate present in vertex
    MTL_VTX_U1 = 0x100,             // Second U co-ordinate present in vertex
    MTL_VTX_V1 = 0x200,             // Second V co-ordinate present in vertex
    MTL_VTX_U2 = 0x400,             // Third U co-ordinate present in vertex
    MTL_VTX_V2 = 0x800,             // Third V co-ordinate present in vertex
    MTL_VTX_U3 = 0x1000,            // Fourth U co-ordinate present in vertex
    MTL_VTX_V3 = 0x2000,            // Fourth V co-ordinate present in vertex
    MTL_VTX_U4 = 0x4000,            // Fifth U co-ordinate present in vertex
    MTL_VTX_V4 = 0x8000,            // Fifth V co-ordinate present in vertex
    MTL_VTX_U5 = 0x10000,           // Sixth U co-ordinate present in vertex
    MTL_VTX_V5 = 0x20000,           // Sixth V co-ordinate present in vertex
    MTL_VTX_U6 = 0x40000,           // Seventh U co-ordinate present in vertex
    MTL_VTX_V6 = 0x80000,           // Seventh V co-ordinate present in vertex
    MTL_VTX_U7 = 0x100000,          // Eighth U co-ordinate present in vertex
    MTL_VTX_V7 = 0x200000,          // Eighth V co-ordinate present in vertex
    MTL_VTX_RSVD1 = 0x400000,       // Reserved
    MTL_VTX_RSVD2 = 0x800000,       // Reserved
    MTL_VTX_RSVD3 = 0x1000000,      // Reserved
    MTL_VTX_RSVD4 = 0x2000000,      // Reserved
    MTL_VTX_RSVD5 = 0x4000000,      // Reserved
    MTL_VTX_RSVD6 = 0x8000000,      // Reserved
    MTL_VTX_RSVD7 = 0x10000000,     // Reserved
    MTL_VTX_RSVD8 = 0x20000000,     // Reserved
    MTL_VTX_RSVD9 = 0x40000000,     // Reserved
    MTL_VTX_RSVD10 = 0x80000000     // Reserved
    } MTL_VERTEX_TYPE;




// available video modes
typedef enum
{
  MTL_MODE_320_200_16, 
  MTL_MODE_512_384_16,
  MTL_MODE_640_400_16,
  MTL_MODE_640_480_16,
  MTL_MODE_800_600_16,
  MTL_MODE_320_200_32,
  MTL_MODE_512_384_32,
  MTL_MODE_640_400_32,
  MTL_MODE_640_480_32,
  MTL_MODE_1024_768_16,
  MTL_MODE_800_600_32,
  MTL_MODE_320_240_16,
  MTL_MODE_320_240_32,
  MTL_MODE_400_300_16,
  MTL_MODE_400_300_32,
  MTL_MODE_1024_768_32,
  MTL_MODE_1152_864_16,
  MTL_MODE_1152_864_32,
  MTL_MODE_1280_1024_16,
  MTL_MODE_1280_1024_32,
  MTL_MODE_1600_1200_16,
  MTL_MODE_1600_1200_32,
  MTL_MODE_720_480_16,
  MTL_MODE_720_480_32,
  MTL_MODE_720_576_16,
  MTL_MODE_720_576_32,
  MTL_MODE_NUM_MODES
} MTL_MODE;

// buffer types for mtlCreateBuffers
typedef unsigned MTL_BUFFER_TYPE;

#define MTL_BUF_FRONT		0x00000001	// front buffer (necessary)
#define MTL_BUF_BACK		0x00000002	// back buffer
#define MTL_BUF_BACK2		0x00000004	// secondary back buffer
#define MTL_BUF_OVERLAY		0x00000008	// force use of overlay buffer
#define MTL_BUF_Z16			0x00000010	// Z buffer, 16 bit values
#define MTL_BUF_Z24			0x00000020	// Z buffer, 24 bit values
#define MTL_BUF_STENCIL		0x00000040	// Stencil buffer

// common combinations:
#define MTL_BUF_DOUBLE				((MTL_BUFFER_TYPE)(MTL_BUF_FRONT|MTL_BUF_BACK))
#define MTL_BUF_TRIPLE				((MTL_BUFFER_TYPE)(MTL_BUF_FRONT|MTL_BUF_BACK|MTL_BUF_BACK2))
#define MTL_BUF_DOUBLE_Z			((MTL_BUFFER_TYPE)(MTL_BUF_DOUBLE|MTL_BUF_Z16))
#define MTL_BUF_TRIPLE_Z			((MTL_BUFFER_TYPE)(MTL_BUF_TRIPLE|MTL_BUF_Z16))
#define MTL_BUF_DOUBLE_Z24			((MTL_BUFFER_TYPE)(MTL_BUF_DOUBLE|MTL_BUF_Z24))
#define MTL_BUF_TRIPLE_Z24			((MTL_BUFFER_TYPE)(MTL_BUF_TRIPLE|MTL_BUF_Z24))
#define MTL_BUF_DOUBLE_Z_STENCIL	((MTL_BUFFER_TYPE)(MTL_BUF_DOUBLE|MTL_BUF_Z24|MTL_BUF_STENCIL))
#define MTL_BUF_TRIPLE_Z_STENCIL	((MTL_BUFFER_TYPE)(MTL_BUF_TRIPLE|MTL_BUF_Z24|MTL_BUF_STENCIL))

// active render buffer
typedef enum
{
  MTL_RENDER_BUFFER_FRONT,          // render to front buffer (can be overlay buffer...)
  MTL_RENDER_BUFFER_BACK,           // render to back buffer
  MTL_RENDER_BUFFER_FORCE_FRONT,    // render to real front buffer (if possible)
  MTL_RENDER_BUFFER_FORCE_OVERLAY   // render to overlay buffer
} MTL_RENDER_BUFFER;

// location of vertex buffer
typedef unsigned MTL_VB_TYPE;

#define MTL_VB_PCI       0x00000001   // vertex buffer is in system memory
#define MTL_VB_AGP       0x00000002   // vertex buffer is in AGP memory
#define MTL_VB_VIDEO     0x00000004   // vertex buffer is in video memory
#define MTL_VB_DOUBLE    0x00000100   // create auto-flipping double buffered vertex buffer

// vertex special type identifiers
#define MTL_VERTEX_NOZERO	  0x01000000	// Vertex data has no primitives of zero width
#define MTL_VERTEX_MMX		  0x02000000	// Sending data by MMX will be faster (no EMMS overhead)

// Flags for resize
#define MTL_RESIZE_MINIMISE		0x00000001	// Signal to Metal that the application has been minimised
#define MTL_RESIZE_MINIMIZE		0x00000001	// Accept both spellings
#define MTL_RESIZE_MAXIMISE		0x00000002	// Signal to Metal that the application has been maximised
#define MTL_RESIZE_MAXIMIZE		0x00000002

// native palette formats
typedef enum
{
  MTL_PAL_RGB565,
  MTL_PAL_ARGB1555,
  MTL_PAL_ARGB4444
} MTL_PAL_FORMAT;

// native texture formats
typedef enum
{
  MTL_TEX_S3TC,
  MTL_TEX_PAL,
  MTL_TEX_RGB565,
  MTL_TEX_ARGB8888,
  MTL_TEX_ARGB1555,
  MTL_TEX_ARGB4444,
  MTL_TEX_INTENSITY = 6,			// All colour + alpha == value
  MTL_TEX_I8 = 6,
  MTL_TEX_LUMINANCE = 7,			// All colour == value, alpha == 0xff
  MTL_TEX_L8 = 7,
  MTL_TEX_ALPHA = 8,				// All colour = 0xff, alpha = value
  MTL_TEX_A8 = 8,
  MTL_TEX_VIDEOMEMORY = 0x00000000,   // allocate texture in video memory
  MTL_TEX_PCIMEMORY   = 0x80000000,   // allocate texture in PCI memory
  MTL_TEX_AGPMEMORY   = 0xc0000000    // allocate texture in AGP memory
} MTL_TEX_FORMAT;

// logical texture formats
typedef enum
{
  MTL_LOGTEX_S3TC = 0x10,
  MTL_LOGTEX_PAL = 0x20,
  MTL_LOGTEX_RGB565 = 0x30,
  MTL_LOGTEX_ARGB8888 = 0x40,
  MTL_LOGTEX_ARGB1555 = 0x50,
  MTL_LOGTEX_ARGB4444 = 0x60,
  MTL_LOGTEX_INTENSITY = 0x70,
  MTL_LOGTEX_I8 = 0x70,
  MTL_LOGTEX_INTENSITY_ALPHA = 0x80,
  MTL_LOGTEX_ABGR8888 = 0x90,
  MTL_LOGTEX_LUMINANCE = 0xa0,
  MTL_LOGTEX_L8 = 0xa0,
  MTL_LOGTEX_ALPHA = 0xb0,
  MTL_LOGTEX_A8 = 0xb0,
  MTL_LOGTEX_AUTOMIPMAP = 0x80000000,	// generate lower mipmap levels automatically
  MTL_LOGTEX_NOSYNC = 0x40000000		// Don't synchronise updates if texture is in use
} MTL_LOGTEX_FORMAT;

// texture format conversions
// --------------------------
//   logical: S3TC      PAL       (A)RGBxxxx
// native:
//    S3TC    ok        ok        ok
//    PAL     invalid   ok        invalid
//    (A)RGB  ok        ok        ok

typedef unsigned MTL_PRIMITIVE;

#define MTL_PRIM_TRILIST   0        // triangle list
#define MTL_PRIM_TRISTRIP  1        // triangle strip (i,i+1,i+2)
#define MTL_PRIM_TRIFAN    2        // triangle fan (0,i,i+1)
#define MTL_PRIM_LINELIST  3        // line list

// chip capabilities
typedef struct
{
  MTL_DWORD size;             // sizeof(MTL_CAPS)
  char id[8];                 // ASCII chip ID
  MTL_DWORD light;            // lighting caps (MTL_CAPS_LIGHT_xxx)
  MTL_DWORD tex;              // texture caps (MTL_CAPS_TEX_xxx)
  MTL_DWORD blend;            // blending modes (MTL_CAPS_BLEND_xxx)
  MTL_DWORD srf;              // surface caps (MTL_CAPS_SRF_xxx)
  MTL_DWORD alpha;            // alpha blending caps (MTL_CAPS_ALPHA_xxx)
  MTL_DWORD fog;              // fog caps (MTL_CAPS_FOG_xxx)
  MTL_DWORD scale;            // scaling caps (MTL_CAPS_SCALE_xxx)
  MTL_DWORD mem;              // memory caps (MTL_CAPS_MEM_xxx)
} MTL_CAPS;

//@@@ TBD: CAPS defines

// list of states
typedef enum
{
  /// texture
  /// -------
    MTL_ST_TEX_ADDRESS=0,
      // SET/GET: texture address
      // Bit 0-1: (MTL_ADDRESS_xxx, below)
      // Bit 2: =0
      // Bit 3-31: video/agp memory address (bit 0-2 of addr always 0)
    MTL_ST_TEX_WIDTH=1,
      // SET/GET: width of texture in pixel (2**n)
    MTL_ST_TEX_HEIGHT=2,
      // SET/GET: height of texture in pixel (2**n)
    MTL_ST_TEX_FORMAT=3,
      // SET/GET: format of texture (MTL_TEX_FORMAT_xxx, below)
    MTL_ST_TEX_FILTER=4,
      // SET/GET: filter mode (MTL_TEX_FILTER_xxx, below)
    MTL_ST_TEX_MIPMAP=5,
      // SET/GET: mipmapping mode (MTL_TEX_MIPMAP_xxx, below)
      //          (also effects MTL_ST_ALPHA_D_FRACTION)
    MTL_ST_TEX_MIPMAP_BIAS=6,
      // SET/GET: constant offset to mipmap level (format S4.4)
    MTL_ST_TEX_D3D_WRAP_U=7,
      // SET/GET: Direct3D style U wrapping on/off
    MTL_ST_TEX_D3D_WRAP_V=8,
      // SET/GET: Direct3D style V wrapping on/off
    MTL_ST_TEX_ADDRESS_MODEL=9,
      // SET/GET: texture addressing model (MTL_TEX_ADDRESS_MODEL_xxx, below)
    MTL_ST_TEX_ENABLE=10,
      // SET/GET: enable/disable texture mapping
    MTL_ST_TEX_PAL=11,
      // SET/GET: handle of current palette that is attached to current texture
      //          or default palette if no texture is being selected
    MTL_ST_TEX_SELECT=12,
      // SET/GET: handle of current texture or NULL if no texture is selected
    MTL_ST_TEX_BLENDING=29,
      // SET/GET: texture blending mode (MTL_TEX_BLENDING_xxx, below)
    MTL_ST_TEX_UV_OFFSET=59,
      // SET/GET: true if 0.5 pixel offset should be added to U/V, false if otherwise
    MTL_ST_ALPHA_TEX_COLOR=15,
    MTL_ST_TEX_COLORKEY_COLOR=15,				// Alias
      // SET/GET: transparent color for RGB565 and Palette/RGB565 mode
	MTL_ST_TEX_COLORKEY_ENABLE=60,
	  // SET/GET: enable colorkeying on this texture

  /// palette
  /// -------
    MTL_ST_PAL_SIZE=13,
      // SET/GET: # of entries in palette (n*64+64)
    MTL_ST_PAL_RELOAD=0x100,
      // SET: force reload of palette
    MTL_ST_PAL_ADDRESS=14,
      // SET/GET: address of current palette, must be in same address space
      //          as current texture (qword-aligned)

  /// alpha
  /// -----
    MTL_ST_ALPHA_D_FRACTION=16,
      // SET/GET: enable/disable using D fraction as alpha value
      //          (this state get also effected when changing MTL_ST_TEX_MIPMAP)
    MTL_ST_ALPHA_TEX_COLOR_CTRL=58,
      // SET/GET: true if partially transparent pixels should be belnded with destination
      //          color, false if resulting alpha should be set to either 0 or 1
    MTL_ST_ALPHA_TEX=17,
      // SET/GET: enable/disable texture transparency
    MTL_ST_ALPHA_BLEND_SRC=18,
    MTL_ST_ALPHA_BLEND_DEST=19,
      // SET/GET: source/destination alpha blend mode (MTL_ALPHA_BLEND_xxx, below)
    MTL_ST_ALPHA_COMPARE=20,
      // SET/GET: current alpha compare function (MTL_COMPARE_xxx, below)
    MTL_ST_ALPHA_TEST=21,
      // SET/GET: enable/disable alpha test
    MTL_ST_ALPHA_REFERENCE=22,
      // SET/GET: alpha reference value (8 bit)

  /// fog
  /// ---
    MTL_ST_FOG_COLOR=23,
      // SET/GET: current fog color (MTL_COLOR/_STRUCT)
    MTL_ST_FOG_START=24,
      // SET/GET: Z starts at 1-2**(-n), n=0..7
    MTL_ST_FOG_ENABLE=25,
      // SET/GET: enable/disable fog
    MTL_ST_FOG_MODE_VERTEX=26,
      // SET/GET: use vertex fog parameter (MTL_TRUE) or table fog (MTL_FALSE)
    MTL_ST_FOG_TABLE=0x101,
      // SET: defines entry in fog table
      // GET: retrieve entry from fog table
      //    bit 0-7: fog value
      //    bit 8-13: table register number (must be filled before call to
      //                                      mtlGetState!)

  /// lighting
  /// --------
    MTL_ST_LIGHT_SHADE=27,
      // SET/GET: shading type (MTL_LIGHT_SHADE_xxx, below)
    MTL_ST_LIGHT_SPECULAR=28,
      // SET/GET: enable/disable specular shading

  /// framebuffer
  /// -----------
    MTL_ST_FB_OFFSET=30,
      // SET/GET: framebuffer offset (quadword aligned)
    MTL_ST_FB_WIDTH=31,
      // SET/GET: width of framebuffer line in tiles
    MTL_ST_FB_DEPTH=32,
      // SET/GET: bit depth of framebuffer (8/16/32)

  /// draw buffer
  /// -----------
    MTL_ST_DEST_OFFSET=33,
      // SET/GET: destination buffer offset (2k aligned)
    MTL_ST_DEST_WIDTH=34,
      // SET/GET: width of destination buffer in tiles
    MTL_ST_DEST_FORMAT=35,
      // SET/GET: destination pixel format (MTL_DEST_FORMAT_xxx, below)
    MTL_ST_DEST_UPDATE=36,
      // SET/GET: enable (MTL_TRUE) or disable (MTL_FALSE) draw buffer update

  /// Z buffer
  /// --------
    MTL_ST_Z_COMPARE=37,
      // SET/GET: current Z compare function (MTL_COMPARE_xxx, below)
    MTL_ST_Z_UPDATE=38,
      // SET/GET: if Z test passes update Z buffer (MTL_TRUE) or don't (MTL_FALSE)
    MTL_ST_Z_ENABLE=39,
      // SET/GET: enable/disable Z buffer compare and update
    MTL_ST_Z_EXPONENT=40,
      // SET/GET: current Z exponent offset (-128..+127), bit 8..31=0!
    MTL_ST_Z_AFTER_ALPHA=41,
      // SET/GET: Z test after alpha test (MTL_TRUE) or before texture read (MTL_FALSE)
    MTL_ST_Z_OFFSET=42,
      // SET/GET: offset of Z buffer (2k aligned)
    MTL_ST_Z_WIDTH=43,
      // SET/GET: width of Z buffer in tiles
    MTL_ST_Z_DEPTH=44,
      // SET/GET: bit depth of Z buffer (16/32)

  /// misc
  /// ----
    MTL_ST_STATUS_AREA=0x102,
      // GET: returns pointer to shadow status area
    MTL_ST_STATUS_UPDATE=0x103,
      // SET/GET: force update of shadow status area
    MTL_ST_CLIP_X_START=45,
    MTL_ST_CLIP_Y_START=46,
    MTL_ST_CLIP_X_END=47,
    MTL_ST_CLIP_Y_END=48,
      // SET/GET: clipping area (all coordinates inclusive)
    MTL_ST_DITHERING=49,
      // SET/GET: enable/disable dithering
    MTL_ST_CULL_MODE=50,
      // SET/GET: backface cull mode (MTL_CULL_MODE_xxx, below)
    MTL_ST_INTERPOLATE_LINEAR=51,
      // SET/GET: set linear (MTL_TRUE) or perspective corrected (MTL_FALSE)
      //          interpolation mode for color, vertex fog and vertex alpha
    MTL_ST_VERTEXBUFFER=52,
      // SET/GET: vertex buffer address
      // Bit 0-1: (MTL_ADDRESS_xxx, below)
      // Bit 2: =0
      // Bit 3-31: video/agp memory address (bit 0-2 of addr always 0)
    MTL_ST_ERASE_COLOR=53,
      // SET/GET: color for mtlEraseBuffers Front/Back-Buffers (MTL_COLOR/_STRUCT)
    MTL_ST_ERASE_Z=54,
      // SET/GET: Z value for mtlEraseBuffers Z-buffer (MTL_VALUE)
    MTL_ST_BUFFER_FRONT=0x104,
    MTL_ST_BUFFER_BACK=0x105,
    MTL_ST_BUFFER_BACK2=0x106,
    MTL_ST_BUFFER_Z=0x107,
    MTL_ST_BUFFER_OVERLAY=0x108,
      // GET (Win32 only): returns an LPDIRECTDRAWSURFACE3 pointer to the given buffer surface
      //                   or NULL if there is no such buffer; LPDIRECTDRAWSURFACE3* must be passed
      //                   to mtlGetState. The pointer must be released after done using it!
    MTL_ST_OVERLAY_COLOR=55,
      // SET/GET: color for overlay background (MTL_COLOR/_STRUCT)
    MTL_ST_AUTOSCALE_X=56,
    MTL_ST_AUTOSCALE_Y=57,
      // SET/GET: current width/height of render area

	  // New for Savage2:

	MTL_ST_VERTEX_FORMAT=0x109,
	  // SET: current vertex format to be accepted by all Draw routines

	MTL_ST_Z_FLOAT_ENABLE=67,
	  // SET/GET: Enable storage of floating point Z or W
	MTL_ST_DEPTH_TYPE=68,
	  // SET/GET: Sets to one of MTL_DEPTH_xxx, below

	MTL_ST_ALPHA_BLEND_D_SUB_S_ENABLE=62,
	  // SET/GET: enable D-S blending mode (ignores other blend state)

	MTL_ST_TEX_FACTOR=63,
	  // SET/GET: set the texture constant colour factor
    MTL_ST_TEX_ADDRESS_MODEL_U=64,
      // SET/GET: texture addressing model for U axis (MTL_TEX_ADDRESS_MODEL_xxx, below)
    MTL_ST_TEX_ADDRESS_MODEL_V=65,
      // SET/GET: texture addressing model for V axis (MTL_TEX_ADDRESS_MODEL_xxx, below)
	MTL_ST_TEX_MAX_D_LEVEL=66,
	  // SET/GET: maximum texture D level
	MTL_ST_TEX_STAGE=69,
	  // SET/GET: current texture stage affected by all TEX operations
	MTL_ST_TEX_BLEND=70,
	  // SET/GET: Blending mode by structure MTL_TEXBLEND_CONTROL

	MTL_ST_STENCIL_ENABLE=71,
	  // SET/GET: Enable/disable stencil buffer (32-bit Z modes only)
	MTL_ST_STENCIL_COMPARE=72,
	  // SET/GET: stencil buffer compare mode (MTL_COMPARE_xxx below)
	MTL_ST_STENCIL_REFERENCE=73,
	  // SET/GET: stencil buffer reference value for comparison
	MTL_ST_STENCIL_READ_MASK=74,
	  // SET/GET: stencil buffer read bitmask
	MTL_ST_STENCIL_WRITE_MASK=75,
	  // SET/GET: stencil buffer write bitmask
	MTL_ST_STENCIL_FAIL_OP=76,
	  // SET/GET: operation to be performed if stencil test fails (MTL_STENCILOP_xxx below)
	MTL_ST_STENCIL_PASS_Z_FAIL_OP=77,
	  // SET/GET: operation to be performed if stencil test passes but Z buffer test fails (MTL_STENCILOP_xxx below)
	MTL_ST_STENCIL_PASS_Z_PASS_OP=78,
	  // SET/GET: operation to be performed if stencil test and Z test both pass (MTL_STENCILOP_xxx below)

	MTL_ST_BINARY_FINAL_ALPHA=79,		// Alias with reversed meaning of state 58
	  // SET/GET; see state 58 (Colour Control Alpha); valid on both GX3 and MS1

	MTL_ST_STENCIL_CLEAR=80,
	  // SET/GET: Value to clear stencil buffer to; only valud on GX3

	MTL_ST_Z_AUTOCLEAR_ENABLE=81,
	  // SET/GET: Auto Z clear enabled
	MTL_ST_Z_AUTOCLEAR_FRAMEID=82,
	  // SET/GET: Auto Z clear frame ID

	MTL_ST_D_PERFORMANCE_ACCELERATOR=83,
	  // SET/GET: enables the change in D-level scaling; actually gives slightly better visual appearance usually

	MTL_ST_FLUSH_Z=84,
	  // SET/GET: enables flushing of Z values before doing Z reads
	MTL_ST_FLUSH_DEST=85,
	  // SET/GET: enables flushing of destination values before doing destination reads

	MTL_ST_CHIP_ID=0x10a,
	  // GET: returns chip ID. valid on GX3 and MS1
	MTL_ST_VRAM_SIZE=0x10b,
	  // GET: returns size of video memory in bytes


    // max Index A:    83 (including)
    // max Index B: 0x10b (including)
} MTL_STATE;



/// special definitions for set/getState
/// ------------------------------------

// for MTL_ST_TEX_ADDRESS and MTL_ST_VERTEXBUFFER:
#define MTL_ADDRESS_VIDEO       0
#define MTL_ADDRESS_PCI         2
#define MTL_ADDRESS_AGP         3

// for MTL_ST_TEX_FORMAT:
#define MTL_TEX_FORMAT_S3TC			 (0<<16)
#define MTL_TEX_FORMAT_4S3TC		 (0<<16)	// 4-bit S3TC, one-bit alpha optionally encoded
#define MTL_TEX_FORMAT_PAL_RGB565	 (1<<16)
#define MTL_TEX_FORMAT_PAL_ARGB1555	 (2<<16)
#define MTL_TEX_FORMAT_ARGB8888		 (3<<16)
#define MTL_TEX_FORMAT_ARGB1555		 (4<<16)
#define MTL_TEX_FORMAT_ARGB4444		 (5<<16)
#define MTL_TEX_FORMAT_RGB565		 (6<<16)
#define MTL_TEX_FORMAT_PAL_ARGB4444	 (7<<16)
#define MTL_TEX_FORMAT_8A4S3TCC4	 (8<<16)	// 8-bit S3TC; 4 bits alpha + 4 bits S3TC
#define MTL_TEX_FORMAT_8S3TCA4C4	 (9<<16)	// 8-bit S3TC; 4 bits S3TC alpha + 4 bits S3TC
#define MTL_TEX_FORMAT_4S3TCL4		(10<<16)	// 4-bit S3TC luminance
#define MTL_TEX_FORMAT_8A4S3TCL4	(11<<16)	// 8-bit S3TC; 4 bits alpha + 4 bits S3TC luminance
#define MTL_TEX_FORMAT_L8			(12<<16)	// colour set to texture value, alpha set to 0xff
#define MTL_TEX_FORMAT_A4L4			(13<<16)	// 4-bit alpha + 4-bit luminance
#define MTL_TEX_FORMAT_I8			(14<<16)	// all colour & alpha set to same
#define MTL_TEX_FORMAT_A8			(15<<16)	// all colour set to 0xff, alpha set to value

// for MTL_ST_TEX_FILTER:
#define MTL_TEX_FILTER_1TPP   0
#define MTL_TEX_FILTER_4TPP   1
#define MTL_TEX_FILTER_16TPP  3

// for MTL_ST_TEX_MIPMAP:
#define MTL_TEX_MIPMAP_OFF    1
#define MTL_TEX_MIPMAP_ON     0
#define MTL_TEX_MIPMAP_FULL   2   // OpenGL-compliant trilinear filtering

// for MTL_ST_TEX_ADDRESS_MODEL:
#define MTL_TEX_ADDRESS_MODEL_WRAP    (0<<14)
#define MTL_TEX_ADDRESS_MODEL_CLAMP   (1<<14)
#define MTL_TEX_ADDRESS_MODEL_MIRROR  (2<<14)

// for MTL_ST_ALPHA_BLEND_SRC and MTL_ST_ALPHA_BLEND_DEST:
#define MTL_ALPHA_BLEND_ZERO    0
#define MTL_ALPHA_BLEND_ONE     1
#define MTL_ALPHA_BLEND_COL     2 // source or destination color
#define MTL_ALPHA_BLEND_INVCOL  3
#define MTL_ALPHA_BLEND_SRC     4 // source alpha
#define MTL_ALPHA_BLEND_INVSRC  5
#define MTL_ALPHA_BLEND_DEST    6 // destination alpha
#define MTL_ALPHA_BLEND_INVDEST 7

// for MTL_ST_ALPHA_COMPARE and MTL_ST_Z_COMPARE:
#define MTL_COMPARE_NEVER         0   // never pass A/Z test
#define MTL_COMPARE_LESS          1   // pass if A/Znew < Aref/Zfb
#define MTL_COMPARE_EQUAL         2   // pass if A/Znew == Aref/Zfb
#define MTL_COMPARE_LESSEQUAL     3   // pass if A/Znew <= Aref/Zfb
#define MTL_COMPARE_GREATER       4   // pass if A/Znew > Aref/Zfb
#define MTL_COMPARE_NOTEQUAL      5   // pass if A/Znew != Aref/Zfb
#define MTL_COMPARE_GREATEREQUAL  6   // pass if A/Znew >= Aref/Zfb
#define MTL_COMPARE_ALWAYS        7   // always pass A/Z test

// for MTL_ST_LIGHT_SHADE:
#define MTL_LIGHT_SHADE_GOURAUD           (1<<4)
#define MTL_LIGHT_SHADE_FLAT              (1<<5)
#define MTL_LIGHT_SHADE_WIREFRAME         (1<<6)

// for MTL_ST_TEX_BLENDING:
#define MTL_TEX_BLENDING_DECAL          (0<<26) // cPix=cTex, aPix=aSrc
#define MTL_TEX_BLENDING_MODULATE       (1<<26) // cPix=cSrc*cTex, aPix=aTex
#define MTL_TEX_BLENDING_DECAL_ALPHA    (2<<26) // cPix=(cSrc*(1-aTex))+aTex*cTex, aPix=aSrc
#define MTL_TEX_BLENDING_MODULATE_ALPHA (3<<26) // cPix=cSrc*cTex, aPix=aSrc*aTex
#define MTL_TEX_BLENDING_COPY           (6<<26) // cPix=cTex, aPix=aTex

// for MTL_ST_DEST_FORMAT:
#define MTL_DEST_FORMAT_RGB565    (0<<28)
#define MTL_DEST_FORMAT_XRGB8888  (2<<28)

// for MTL_ST_CULL_MODE:
#define MTL_CULL_MODE_NONE    (1<<2)
#define MTL_CULL_MODE_CW      (2<<2)
#define MTL_CULL_MODE_CCW     (3<<2)

// for MTL_ST_DEPTH_TYPE
#define MTL_DEPTH_W				0
#define MTL_DEPTH_Z				1

// for MTL_ST_STENCIL_xxx_OP
#define MTL_STENCILOP_UNCHANGED		0
#define MTL_STENCILOP_ZERO			1
#define MTL_STENCILOP_SET_REFERENCE	2
#define MTL_STENCILOP_INC_CLAMP		3
#define MTL_STENCILOP_DEC_CLAMP		4
#define MTL_STENCILOP_INVERT		5
#define MTL_STENCILOP_INC			6
#define MTL_STENCILOP_DEC			7


#ifdef MTL_INTERNAL
  #ifdef DXSIM
    #include "dxsim\context.h"
    #include "dxsim\inttype.h"
  #else
    #include "context.h"
    #include "inttype.h"
  #endif
#endif

// MeTaL vertex buffer
MTL_DECLARE_HANDLE(MTL_VERTEXBUFFER);

// MeTaL palette
MTL_DECLARE_HANDLE(MTL_PALETTE);

// MeTaL texture
MTL_DECLARE_HANDLE(MTL_TEXTURE);

typedef struct _MTL_CONTEXT *MTL_CONTEXT;


/* Gamma correction control structure */
typedef struct {
	MTL_DWORD *set;		// Palette to set
	MTL_DWORD *get;		// Palette; if nonzero, specifies directly
	MTL_VALUE r, g, b;	// Otherwise, rgb gamma values
} MTL_GAMMA_CONTROL;


/* Texture blending control structures */

typedef enum {
	MTL_TEXARG_TEXTURE,
	MTL_TEXARG_DIFFUSE,
	MTL_TEXARG_FACTOR,
	MTL_TEXARG_CURRENT,
	MTL_TEXARG_SPECULAR,				// Only valid on arg2
	MTL_TEXARG_INVERT		= 0x0100,
	MTL_TEXARG_COPYALPHA	= 0x0200	// Only valid on color
} MTL_TEXARG;

typedef enum {
	/* Color and alpha ops */
	MTL_TEXOP_MOD1_ARG1		= (0<<0),
	MTL_TEXOP_MOD1_ZERO		= (1<<0),

	MTL_TEXOP_MOD2_ARG2		= (0<<1),
	MTL_TEXOP_MOD2_ZERO		= (1<<1),
	MTL_TEXOP_MOD2_255		= (2<<1),
	MTL_TEXOP_MOD2_PREMOD	= (3<<1),

	MTL_TEXOP_ADD_ZERO		= (0<<3),
	MTL_TEXOP_ADD_ARG2		= (1<<3),
	MTL_TEXOP_ADD_ALPHA		= (3<<3),	// Not valid on alpha

	MTL_TEXOP_DO_BLEND		= (1<<5),

	MTL_TEXOP_DO_NEGATE		= (1<<6),

	MTL_TEXOP_CLAMP			= (1<<7),

	/* Color op only */
	MTL_TEXOP_PREMOD_ARG1COLOR	= (0<<16),
	MTL_TEXOP_PREMOD_ARG1ALPHA	= (1<<16),

	MTL_TEXOP_INVERT_ALPHA		= (1<<17),

	MTL_TEXOP_INVERT_MOD1ARG2	= (1<<18),

	MTL_TEXOP_BIAS				= (1<<19),

	MTL_TEXOP_2X				= (1<<20),
	MTL_TEXOP_4X				= (2<<20),

	MTL_TEXOP_DIFFUSE_MUL		= (1<<22)	// Stage 1 only

} MTL_TEXOP;


/* TEXOP shortcuts */
#define MTL_TEXOP_SELECTARG1	(MTL_TEXOP) (MTL_TEXOP_MOD1_ARG1 | MTL_TEXOP_MOD2_255 | MTL_TEXOP_ADD_ZERO )
#define MTL_TEXOP_SELECTARG2	(MTL_TEXOP) (MTL_TEXOP_MOD1_ZERO | MTL_TEXOP_MOD2_255 | MTL_TEXOP_ADD_ARG2 )
#define MTL_TEXOP_MODULATE		(MTL_TEXOP) (MTL_TEXOP_MOD1_ARG1 | MTL_TEXOP_MOD2_ARG2 | MTL_TEXOP_ADD_ZERO )
#define MTL_TEXOP_MODULATE2X	(MTL_TEXOP) (MTL_TEXOP_MOD1_ARG1 | MTL_TEXOP_MOD2_ARG2 | MTL_TEXOP_ADD_ZERO | MTL_TEXOP_2X)


typedef enum {
	MTL_TEXBLEND_VERSION_1,
} MTL_TEXBLEND_VERSION;


typedef struct {
	MTL_TEXBLEND_VERSION	version;
	BOOL					update;			// Set this whenever the cache needs recalculating INCLUDING first use
	MTL_TEXARG				color_arg1;
	MTL_TEXARG				color_arg2;
	MTL_TEXOP				color_op;
	MTL_TEXARG				alpha_arg1;
	MTL_TEXARG				alpha_arg2;
	MTL_TEXOP				alpha_op;
	MTL_DWORD				cache[4];		// Cached state, used internally: DO NOT MODIFY
} MTL_TEXBLEND_CONTROL;







////
//// MAIN INTERFACE (MTL_CONTEXT)
//// ============================
////

// ::mtlOpen:
//    Initializes library, must be the first function call into library.
//    Returns MTL_CONTEXT in given buffer and MTL_ERR_NONE on
//    success or any of the defined error codes on failure.

#ifdef __cplusplus
extern "C"
#endif

MTL_DLLTYPE MTL_ERROR                 // success or failure code
MTL_CALLTYPE mtlOpen(
  void *pReserved,                    // reserved, must be NULL
  MTL_OUT MTL_CONTEXT *pCtx           // return buffer for new
                                      // context structure
  );

// MTL_CONTEXT interface:

MTL_INTERFACE
{
  // MTL_CONTEXT::mtlClose:
  //    Shuts down and cleans up context.
  MTL_FUNCTION(mtlClose) (MTL_THIS_) MTL_PURE;

  // MTL_CONTEXT::mtlGetError:
  //    Retrieve error code of last failed operation. Returns full error code of
  //    last failed operation or MTL_ERR_NONE if no error has occured. A call to
  //    mtlGetError does *NOT* reset the error code!
  MTL_FUNCTION_(MTL_ERROR,mtlGetError) (MTL_THIS_) MTL_PURE;

  // Window Mode
  // -----------

#ifdef __WIN32__
  // MTL_CONTEXT::mtlSetDisplayModeWindow (Win32)
  //    Sets up application for windowed mode.
  MTL_FUNCTION(mtlSetDisplayModeWindow) (MTL_THIS
    MTL_IN HWND hWnd,                 // handle of window that receives app msgs
    MTL_IN MTL_DWORD x,               // origin
    MTL_IN MTL_DWORD y,
    MTL_IN MTL_DWORD w,               // size of client area
    MTL_IN MTL_DWORD h
    ) MTL_PURE;

  // MTL_CONTEXT::mtlMoveWindow (Win32)
  //    Moves buffers in windowed mode.
  MTL_FUNCTION(mtlMoveWindow) (MTL_THIS
    MTL_IN MTL_DWORD x,               // new origin
    MTL_IN MTL_DWORD y
    ) MTL_PURE;

  // MTL_CONTEXT::mtlResizeWindow (Win32)
  //    Resizes buffers in windowed mode.
  MTL_FUNCTION(mtlResizeWindow) (MTL_THIS
    MTL_IN MTL_DWORD w,               // new size of render area
    MTL_IN MTL_DWORD h,
	MTL_IN MTL_DWORD flags				// MTL_RESIZE_xxx
    ) MTL_PURE;
#endif // __WIN32__

  // Fullscreen Mode
  // ---------------

#ifdef __WIN32__
  // MTL_CONTEXT::mtlSetDisplayMode (Win32)
  //    Sets up applcation for fullscreen exclusive mode and selects
  //    a new display mode.
  MTL_FUNCTION(mtlSetDisplayMode) (MTL_THIS
    MTL_IN HWND hWnd,                 // handle of window that receives app msgs
    MTL_IN MTL_MODE mode              // screen resolution to set
    ) MTL_PURE;
#endif // __WIN32__

#ifdef __DOS__
  // MTL_CONTEXT::mtlSetDisplayMode (DOS)
  //    Sets up applcation for fullscreen exclusive mode and selects
  //    a new display mode.
  MTL_FUNCTION(mtlSetDisplayMode) (MTL_THIS
    MTL_IN MTL_MODE mode              // screen resolution to set
    ) MTL_PURE;
#endif // __DOS__

  // MTL_CONTEXT::mtlInitAutoscaleMode
  //    Initializes MeTaL for using autoscale mode. In this mode the area that is to be rendered
  //    will be changed dynamically by measuring the current frame rate and comparing it to the
  //    given values. The GX3 scaler will then be used in order to rescale this image to a
  //    full screen image.
  MTL_FUNCTION(mtlInitAutoscaleMode) (MTL_THIS
    MTL_IN MTL_DWORD minX,            // minimum X value allowed
    MTL_IN MTL_DWORD minY,            // minimum Y value allowed
    MTL_IN MTL_VALUE fpsLow,          // low fps treshhold, if fps drops below render area will be reduced
    MTL_IN MTL_VALUE fpsHigh,         // high fps treshhold, if fps goes above render area will be increased
    MTL_IN MTL_VALUE stepFac          // size render area will be changed per step
    ) MTL_PURE;

  // Buffers
  // -------

  // MTL_CONTEXT::mtlCreateBuffers
  //    Creates and activates front-, back- and Z buffers.
  MTL_FUNCTION(mtlCreateBuffers) (MTL_THIS
    MTL_IN MTL_BUFFER_TYPE type       // describes which buffers are to be
                                      // created (see MTL_BUF_xxx)
    ) MTL_PURE;

  // MTL_CONTEXT::mtlSetRenderBuffer
  //    Defines the new rendering buffer used by all MeTaL rendering functions.
  //    You should avoid selecting the front buffer as render buffer in windowed
  //    mode.
  MTL_FUNCTION(mtlSetRenderBuffer) (MTL_THIS
    MTL_IN MTL_RENDER_BUFFER rendBuf  // front or back buffer
    ) MTL_PURE;

  // MTL_CONTEXT::mtlSwapBuffers
  //    Swaps back buffer with front buffer in double-buffered case or rotates
  //    current back buffer to front in tripe-buffered case. In windowed mode
  //    the back buffer gets blitted to the front buffer.
  MTL_FUNCTION(mtlSwapBuffers) (MTL_THIS_) MTL_PURE;

  // MTL_CONTEXT::mtlSetEraseValueBuffers
  //    Defines z value used for mtlEraseBuffers call.
  MTL_FUNCTION(mtlSetEraseValueBuffers) (MTL_THIS
    MTL_IN MTL_VALUE zFillValue
    ) MTL_PURE;

  // MTL_CONTEXT::mtlEraseBuffers
  //    Erases given buffers.
  MTL_FUNCTION(mtlEraseBuffers) (MTL_THIS
    MTL_IN MTL_BUFFER_TYPE buffer     // specifies which buffers to clear
    ) MTL_PURE;

  // MTL_CONTEXT::mtlDestroyBuffers
  //    Destroys buffers created with mtlCreateBuffers. Buffers are
  //    automatically destroyed when application is terminated or
  //    mtlSetDisplayMode is called.
  MTL_FUNCTION(mtlDestroyBuffers) (MTL_THIS_) MTL_PURE;

  // Vertex buffer
  // -------------

  // MTL_CONTEXT::mtlCreateVertexbuffer
  //    Creates a new vertex buffer with a given maximum number of entries.
  MTL_FUNCTION(mtlCreateVertexbuffer) (MTL_THIS
    MTL_IN MTL_VB_TYPE vbtype,        // type/location of vertex buffer
    MTL_IN MTL_DWORD cVertex,         // number of MTL_VERTEX entries wanted
    MTL_OUT MTL_VERTEXBUFFER *phBuf   // returns pointer to vertex buffer handle
    ) MTL_PURE;

  // MTL_CONTEXT::mtlLockVertexbuffer
  //    Locks existing vertex buffer so that entries can be modified.
  //    Calls can be nested.
  MTL_FUNCTION(mtlLockVertexbuffer) (MTL_THIS
    MTL_IN MTL_VERTEXBUFFER hBuf,     // vertex buffer that is to be locked
    MTL_OUT MTL_PVERTEX *ppVertex     // returns pointer to vertex buffer
    ) MTL_PURE;

  // MTL_CONTEXT::mtlUnlockVertexbuffer
  //    Unlocks a previously locked vertex buffer. Calls can be nested. If e.g.
  //    a buffer has been locked twice mtlUnlockVertexbuffer must be called twice in
  //    order to really unlock the vertex buffer.
  MTL_FUNCTION(mtlUnlockVertexbuffer) (MTL_THIS
    MTL_IN MTL_VERTEXBUFFER hBuf      // vertex buffer that should be unlocked
    ) MTL_PURE;

  // MTL_CONTEXT::mtlDestroyVertexbuffer
  //    Destroys a vertex buffer. If the buffer is still locked it is being unlocked
  //    and then destroyed.
  MTL_FUNCTION(mtlDestroyVertexbuffer) (MTL_THIS
    MTL_IN MTL_VERTEXBUFFER hBuf        // vertex buffer that will be destroyed
    ) MTL_PURE;

  // Palette
  // -------

  // MTL_CONTEXT::mtlCreatePalette
  //    Creates a new palette. All colors are initially set to black (0,0,0)
  MTL_FUNCTION(mtlCreatePalette) (MTL_THIS
    MTL_IN MTL_PAL_FORMAT format,     // native format of palette
    MTL_IN MTL_DWORD cEntry,          // number of color entries (1..256)
    MTL_OUT MTL_PALETTE *phPal        // returns pointer to new palette handle
    ) MTL_PURE;

  // MTL_CONTEXT::mtlReadPalette
  //    Reads all or some palette entries and returns them in the given buffer.
  MTL_FUNCTION(mtlReadPalette) (MTL_THIS
    MTL_IN MTL_PALETTE hPal,          // palette to be read
    MTL_IN MTL_DWORD cFrom,           // from entry (0..cEntry-1)
    MTL_IN MTL_DWORD cCount,          // number of entries (1..cEntry-cFrom)
    MTL_OUT MTL_COLOR_STRUCT *pBuf    // pointer to buffer (cCount entries)
    ) MTL_PURE;

  // MTL_CONTEXT::mtlWritePalette
  //    Writes all or some palette entries.
  MTL_FUNCTION(mtlWritePalette) (MTL_THIS
    MTL_IN MTL_PALETTE hPal,          // palette to be written to
    MTL_IN MTL_DWORD cFrom,           // from entry (0..cEntry-1)
    MTL_IN MTL_DWORD cCount,          // number of entries (1..cEntry-cFrom)
    MTL_IN MTL_COLOR_STRUCT *pBuf     // pointer to buffer (cCount entries)
    ) MTL_PURE;

  // MTL_CONTEXT::mtlDestroyPalette
  //    Destroys a palette.
  MTL_FUNCTION(mtlDestroyPalette) (MTL_THIS
    MTL_IN MTL_PALETTE hPal           // palette that will be destroyed
    ) MTL_PURE;

  // Texture
  // -------

  // MTL_CONTEXT::mtlCreateTexture
  //    Creates a new texture either in video, PCI or AGP memory and uploads data. 
  //    Given width and height should be aligned to 2**n; if not, they will automatically be aligned.
  //    If the native texture format is MTL_TEX_PAL the texture will be associated with the
  //    last palette created by calling mtlCreatePalette. This association can be changed by
  //    calling mtlSetState(MTL_ST_TEX_PAL,hPal).
  MTL_FUNCTION(mtlCreateTexture) (MTL_THIS
    MTL_IN MTL_TEX_FORMAT format,     // native texture format (and memory type)
    MTL_IN MTL_DWORD dwWidth,         // width in pixel (2..2048)
    MTL_IN MTL_DWORD dwStride,        // line stride in byte
    MTL_IN MTL_DWORD dwHeight,        // height in pixel (2..2048)
    MTL_IN MTL_DWORD cMip,            // number of mip map levels (1..1+log2(max(w,h)-1))
    MTL_IN MTL_LOGTEX_FORMAT logFmt,  // source format
    MTL_IN void *pData,               // pointer to source data (NULL if no data is to be uploaded)
    MTL_OUT MTL_TEXTURE *phTex        // returns pointer to new texture handle
    ) MTL_PURE;

  // MTL_CONTEXT::mtlCreateRenderTexture
  //    Creates a new texture in video memory that can be used as rendering destination. This
  //    texture consists of a single mipmap level.
  MTL_FUNCTION(mtlCreateRenderTexture) (MTL_THIS
    MTL_IN MTL_DWORD dwWidth,         // width in pixel (2..2048)
    MTL_IN MTL_DWORD dwHeight,        // height in pixel (2..2048)
    MTL_IN MTL_DWORD destFmt,         // format of texture
    MTL_OUT MTL_TEXTURE *phTex        // returns pointer to new texture handle
    ) MTL_PURE;

  // MTL_CONTEXT::mtlUpdateTexture
  //    Updates all or part of some texture data into texture memory.
  MTL_FUNCTION(mtlUpdateTexture) (MTL_THIS
    MTL_IN MTL_TEXTURE hTex,          // texture to be updated
    MTL_IN MTL_LOGTEX_FORMAT format,  // source format
    MTL_IN MTL_DWORD x,               // origin in destination texture
    MTL_IN MTL_DWORD y,
    MTL_IN MTL_DWORD dwWidth,         // size of source data
    MTL_IN MTL_DWORD dwStride,        // line stride in byte
    MTL_IN MTL_DWORD dwHeight,
    MTL_IN MTL_DWORD cStartMip,       // mipmap level to start updating at (0..cMip-1)
    MTL_IN MTL_DWORD cNrMip,          // number of mipmap levels in source data
    MTL_IN void *pData                // pointer to source data
    ) MTL_PURE;

  // MTL_CONTEXT::mtlCopyTexture
  //    Copies given source to given destination texture. Both textures must be of the same
  //    size and color format. If they have different mimpap counts the missing mipmap levels
  //    will be auto-created (not possible for palettized format).
  MTL_FUNCTION(mtlCopyTexture) (MTL_THIS
    MTL_IN MTL_TEXTURE hDest,         // destination texture
    MTL_IN MTL_TEXTURE hSrc           // source texture
    ) MTL_PURE;

  // MTL_CONTEXT::mtlSetRenderTexture
  //    Defines the new rendering buffer used by all MeTaL rendering functions using an existing
  //    texture. The texture must have been allocated using mtlCreateRenderTexture.
  MTL_FUNCTION(mtlSetRenderTexture) (MTL_THIS
    MTL_IN MTL_TEXTURE hTex           // handle of texture that should be rendered into
    ) MTL_PURE;

  // MTL_CONTEXT::mtlDestroyTexture
  //    Destroys the given texture.
  MTL_FUNCTION(mtlDestroyTexture) (MTL_THIS
    MTL_IN MTL_TEXTURE hTex           // texture to be destroyed
    ) MTL_PURE;

  // Rendering
  // ---------

  // MTL_CONTEXT::mtlBeginDraw
  //    Initializes rendering to the current rendering buffer. Must be called before any 
  //    other rendering functions are being called.
  MTL_FUNCTION(mtlBeginDraw) (MTL_THIS_) MTL_PURE;

  // MTL_CONTEXT::mtlEndDraw
  //    Finishes drawing to current rendering buffer. Should be called as soon as possible.
  MTL_FUNCTION(mtlEndDraw) (MTL_THIS_) MTL_PURE;

  // MTL_CONTEXT::mtlPrimitiveDraw
  //    Draws the given primitive.
  MTL_FUNCTION(mtlPrimitiveDraw) (MTL_THIS
    MTL_IN MTL_PRIMITIVE prim,        // type of primitive to be rendered and type of vertices
    MTL_IN MTL_DWORD cVertex,         // number of vertices
    MTL_IN void *pVertex              // vertices
    ) MTL_PURE;

  // MTL_CONTEXT::mtlIndexedDraw
  //    Like mtlPrimitiveDraw but uses indices in given vertex list.
  MTL_FUNCTION(mtlIndexedDraw) (MTL_THIS
    MTL_IN MTL_PRIMITIVE prim,        // type of primitive to be rendered and type of vertices
    MTL_IN MTL_DWORD cIndex,          // number of indices
    MTL_IN MTL_WORD *pIndex,          // index list
    MTL_IN void *pVertex              // vertices
    ) MTL_PURE;

  // MTL_CONTEXT::mtlVBPrimitiveDraw
  //    Like mtlPrimitiveDraw but uses vertex buffer instead of vertex list pointer.
  MTL_FUNCTION(mtlVBPrimitiveDraw) (MTL_THIS
    MTL_IN MTL_PRIMITIVE prim,        // type of primitive to be rendered
    MTL_IN MTL_DWORD cStartIndex,     // index of first vertex in list to be rendered
    MTL_IN MTL_DWORD cVertex,         // number of vertices to be rendered
    MTL_IN MTL_VERTEXBUFFER hVertex   // vertex buffer handle
    ) MTL_PURE;

  // MTL_CONTEXT::mtlVBIndexedDraw
  //    Like mtlIndexedDraw but uses vertex buffer instead of vertex list pointer.
  MTL_FUNCTION(mtlVBIndexedDraw) (MTL_THIS
    MTL_IN MTL_PRIMITIVE prim,        // type of primitive to be rendered
    MTL_IN MTL_DWORD cIndex,          // number of indices
    MTL_IN MTL_WORD *pIndex,          // index list
    MTL_IN MTL_VERTEXBUFFER hVertex   // vertex buffer handle
    ) MTL_PURE;

  // States
  // ------

  // MTL_CONTEXT::mtlSetState
  //    Generic function for changing the MTL_CONTEXT state.
  MTL_FUNCTION(mtlSetState) (MTL_THIS
    MTL_IN MTL_STATE state,           // state to change
    MTL_IN MTL_DWORD value            // new value (interpretation depends
                                      //   on .state)
    ) MTL_PURE;

  // MTL_CONTEXT::mtlSetStateEx
  //    like before, but function gets a list of states and values
  //    to be changed
  MTL_FUNCTION(mtlSetStateEx) (MTL_THIS
    MTL_IN MTL_DWORD cStateCount,     // number of states to change
    MTL_IN MTL_DWORD *pStVal          // list of state/value combinations
    ) MTL_PURE;

  // MTL_CONTEXT::mtlGetState
  //    Retrieves a current MTL_CONTEXT state entry.
  MTL_FUNCTION(mtlGetState) (MTL_THIS
    MTL_IN MTL_STATE state,           // state to retrieve
    MTL_INOUT void *pBuf              // buffer that receives state value,
                                      // sometimes used for passing extra data
                                      // to mtlGetState
    ) MTL_PURE;

  // Misc
  // ----

  // MTL_CONTEXT::mtlGetCaps
  //    Retrieves the capabilities of the current chip.
  MTL_FUNCTION(mtlGetCaps) (MTL_THIS
    MTL_INOUT MTL_CAPS *pCaps         // pointer to caps structure; size must be set to
                                      // sizeof(MTL_CAPS) before calling mtlGetCaps
    ) MTL_PURE;

  // Advanced functions
  // ------------------

  // PLEASE NOTICE: All advanced functions are marked by being preceded by mtlX instead of mtl. 
  // These functions SHOULD NOT BE USED unless you really know what you are doing. Most of the 
  // advanced functions are of use for driver level software only.

  // MTL_CONTEXT::mtlXenterCritical
  //    Enters a critical region, ensures exclusive access to the GX3 BCI and MMIO area. 
  //    Calls can be nested.
  MTL_FUNCTION_(MTL_DWORD,mtlXenterCritical) (MTL_THIS_) MTL_PURE;

  // MTL_CONTEXT::mtlXleaveCritical
  //    Releases critical region.
  MTL_FUNCTION_(void,mtlXleaveCritical) (MTL_THIS_) MTL_PURE;

  // MTL_CONTEXT::mtlXsetRegister
  //    Sets one or more registers using the BCI.
  MTL_FUNCTION(mtlXsetRegister) (MTL_THIS
    MTL_IN MTL_DWORD start,           // index of first register that will be written to
    MTL_IN MTL_DWORD count,           // number of 32 bit registers to write to
    MTL_IN MTL_DWORD *pValue          // list of values that will be written
    ) MTL_PURE;

  // MTL_CONTEXT::mtlXwriteBCI
  //    Writes the given values to the BCI command queue. This function may be highly 
  //    chip version dependent.
  MTL_FUNCTION(mtlXwriteBCI) (MTL_THIS
    MTL_IN MTL_DWORD count,           // number of 32 bit values to write
    MTL_IN MTL_DWORD *pValue          // list of values that will be written
    ) MTL_PURE;

  // MTL_CONTEXT::mtlXgetMMIOptr
  //    Retrieves the linear address of the MMIO area.
  MTL_FUNCTION(mtlXgetMMIOptr) (MTL_THIS
    MTL_OUT void **ppMMIO             // contains pointer to MMIO area on success
    ) MTL_PURE;

  // MTL_CONTEXT::mtlGamma
  // Sets the gamma correction ramp
  MTL_FUNCTION(mtlGamma) (MTL_THIS
	  MTL_IN MTL_GAMMA_CONTROL *gamma
	  ) MTL_PURE;

  // MTL_CONTEXT::mtlCRCTexture
  // Sets the gamma correction ramp
  MTL_FUNCTION(mtlCRCTexture) (MTL_THIS
    MTL_IN MTL_TEXTURE hTex           // handle of texture that should be CRC'd
    ) MTL_PURE;
};

#endif // __METAL_H
