//
// Fast Heckbert quantiser for creating paletted textures from
// high/true colour input textures
//
// Alpha weighting is going to be a tricky thing - unlike the RGB weightings
// which are well defined, the importance of the alpha channel probably has to
// have the highest weighting of all. However, because it affects all the other
// components its correct weighting is almost certainly some product of the alpha
// value itself and the colours it encompasses, rather than a fixed value...
//
// Some code based on source from the Independent JPEG group
// Copyright (C) 1991-1998, Thomas G. Lane.
//
//
// A. Pomianowski
//

__declspec( dllexport ) int   HeckbertQuantize(void            *srcdata, 
                       unsigned char   *dstdata,
                       unsigned short  *paldata,
                       int             srcformat,
                       int             nentries,
                       int             width,
                       int             height,
                       int             has_colourkey,
                       unsigned long   keyval,
                       int             generate_mipmaps);


// Source data formats

#define  ARGB_8888 0
#define  XRGB_8888 1
#define  RGB_888   2
#define  RGB_565   3
#define  ARGB_1555 4
#define  ARGB_4444 5
