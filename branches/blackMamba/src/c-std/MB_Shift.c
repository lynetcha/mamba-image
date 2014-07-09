/*
 * Copyright (c) <2014>, <Nicolas BEUCHER and ARMINES for the Centre de 
 * Morphologie Mathématique(CMM), common research center to ARMINES and MINES 
 * Paristech>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following 
 * conditions: The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * Except as contained in this notice, the names of the above copyright 
 * holders shall not be used in advertising or otherwise to promote the sale, 
 * use or other dealings in this Software without their prior written 
 * authorization.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "mambaApi_loc.h"

extern MB_errcode MB_Shiftb(MB_Image *src, MB_Image *dest, Uint32 dirnum, Uint32 count, Uint32 long_filler_pix, enum MB_grid_t grid);
extern MB_errcode MB_Shift8(MB_Image *src, MB_Image *dest, Uint32 dirnum, Uint32 count, Uint32 long_filler_pix, enum MB_grid_t grid);
extern MB_errcode MB_Shift32(MB_Image *src, MB_Image *dest, Uint32 dirnum, Uint32 count, Uint32 long_filler_pix, enum MB_grid_t grid);

/****************************************/
/* Main function                        */
/****************************************/

/*
 * Shifts the contents of an image in a given direction with a given amplitude
 * The direction depends on the grid used.
 *
 * \param src source image
 * \param dest destination image
 * \param dirnum the direction index
 * \param count the amplitude of the shift
 * \param long_filler_pix the value used to fill the created space
 * \param grid the grid used (either square or hexagonal)
 *
 * \return An error code (NO_ERR if successful)
 */
MB_errcode MB_Shift(MB_Image *src, MB_Image *dest, Uint32 dirnum, Uint32 count, Uint32 long_filler_pix, enum MB_grid_t grid)
{

    switch(src->depth) {
    case 1:
        return MB_Shiftb(src, dest, dirnum, count, long_filler_pix, grid);
        break;
    case 8:
        return MB_Shift8(src, dest, dirnum, count, long_filler_pix, grid);
        break;
    case 32:
        return MB_Shift32(src, dest, dirnum, count, long_filler_pix, grid);
        break;
    default:
        break;
    }
    
    return ERR_BAD_DEPTH;
}

