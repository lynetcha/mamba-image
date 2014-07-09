/*
 * Copyright (c) <2011>, <Nicolas BEUCHER>
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
#include "mambaApi_vector.h"

/****************************************/
/* Base functions                       */
/****************************************/
/* The functions described here realise the basic operations */
/* needed to shift pixels in any directions */

/*
 * Used to displace a complete line in an y direction.
 * \param p_out pointer on the destination image pixel line
 * \param p_in pointer on the source image pixel line
 * \param bytes_in number of bytes inside the line
 */
static INLINE void SHIFT_LINE(PLINE *p_out, PLINE *p_in, Uint32 bytes_in)
{
    Uint32 i;
    
#ifdef MB_VECTORIZATION_8
    MB_Vector8 vec1, vec2;
    MB_Vector8 *pin = (MB_Vector8*) (*p_in);
    MB_Vector8 *pinout = (MB_Vector8*) (*p_out);
    
    for(i=0;i<bytes_in;i+=sizeof(MB_Vector8),pin++,pinout++) {
        vec1 = MB_vec8_load(pin);
        vec2 = MB_vec8_load(pinout);
        vec1 = MB_vec8_max(vec2,vec1);
        MB_vec8_store(pinout, vec1);
    }
    
#else
    PLINE pin = (PLINE) (*p_in);
    PLINE pinout = (PLINE) (*p_out);
    
    for(i=0;i<bytes_in;i++,pin++,pinout++) {
        (*pinout) = (*pinout)>(*pin) ? (*pinout) : (*pin);
    }
#endif
}

/*
 * Used to fill a complete line with a given value (used to fill voided lines following
 * a displacement in y).
 * \param p_out pointer on the destination image pixel line
 * \param bytes_in number of bytes inside the line
 * \param fill_val the value used to fill the line
 */
static INLINE void SHIFT_EDGE_LINE(PLINE *p_out, Uint32 bytes_in, Uint32 fill_val )
{
    Uint32 i;

#ifdef MB_VECTORIZATION_8
    MB_Vector8 vec1;
    MB_Vector8 edge = MB_vec8_set32(fill_val);
    MB_Vector8 *pinout = (MB_Vector8*) (*p_out);
    
    for(i=0;i<bytes_in;i+=sizeof(MB_Vector8),pinout++) {
        vec1 = MB_vec8_load(pinout);
        vec1 = MB_vec8_max(vec1,edge);
        MB_vec8_store(pinout, vec1);
    }
#else
    PLINE pinout = (PLINE) (*p_out);
    
    for(i=0;i<bytes_in;i++,pinout++) {
        (*pinout) = (*pinout)>(fill_val) ? (*pinout) : (fill_val);
    }
#endif
}

/*
 * Used to displace a complete line in the left direction.
 * \param p_out pointer on the destination image pixel line
 * \param p_in pointer on the source image pixel line
 * \param bytes_in number of bytes inside the line
 * \param count the shift amplitude
 * \param fill_val the value used to fill the line
 */
static INLINE void SHIFT_LINE_LEFT(PLINE *p_out, PLINE *p_in,
                                   Uint32 bytes_in, Sint32 count, Uint32 fill_val)
{
    Uint32 i;

#ifdef MB_VECTORIZATION_8

#define CASE_INTRA_SHIFT(VAL) \
                case VAL: \
                    reg1 = MB_vec8_shrgt(reg1, VAL); \
                    reg2 = MB_vec8_shlft(reg2, MB_vec8_size-VAL); \
                    break;
                    
    Uint32 reg_dec, ins_reg_dec;
    MB_Vector8 reg1, reg2;
    MB_Vector8 edge, *pin, *pout;

    /* count cannot exceed the number of pixel in a line */
    reg_dec = ((Uint32) count)<bytes_in ? count/MB_vec8_size : bytes_in/MB_vec8_size;
    ins_reg_dec = count % MB_vec8_size;

    edge = MB_vec8_set32(fill_val);
    pin = (MB_Vector8 *) (*p_in + reg_dec*sizeof(MB_Vector8));
    pout = (MB_Vector8 *) (*p_out);
    
    if (ins_reg_dec==0) {
        /* no intra register shifting */
        for(i=0;i<(bytes_in-reg_dec*sizeof(MB_Vector8));i+=sizeof(MB_Vector8),pin++,pout++) {
            reg1 = MB_vec8_load(pin);
            reg2 = MB_vec8_load(pout);
            reg2 = MB_vec8_max(reg2, reg1);
            MB_vec8_store(pout,reg2);
        }
    } else {
        /* intra register shifting */
        for(i=0;i<(bytes_in-reg_dec*sizeof(MB_Vector8));i+=sizeof(MB_Vector8),pin++,pout++) {
            reg1 = MB_vec8_load(pin);
            reg2 = (i==(bytes_in-(reg_dec+1)*MB_vec8_size)) ? edge : MB_vec8_load(pin+1);
            switch(ins_reg_dec) {
                CASE_INTRA_SHIFT(1)
                CASE_INTRA_SHIFT(2)
                CASE_INTRA_SHIFT(3)
                CASE_INTRA_SHIFT(4)
                CASE_INTRA_SHIFT(5)
                CASE_INTRA_SHIFT(6)
                CASE_INTRA_SHIFT(7)
                CASE_INTRA_SHIFT(8)
                CASE_INTRA_SHIFT(9)
                CASE_INTRA_SHIFT(10)
                CASE_INTRA_SHIFT(11)
                CASE_INTRA_SHIFT(12)
                CASE_INTRA_SHIFT(13)
                CASE_INTRA_SHIFT(14)
                CASE_INTRA_SHIFT(15)
                default:
                case 0:
                    break;
            }
            reg1 = MB_vec8_or(reg1, reg2);
            reg2 = MB_vec8_load(pout);
            reg2 = MB_vec8_max(reg2,reg1);
            MB_vec8_store(pout,reg2);
        }
    }
    
    /* The created space is filled with the fill value */
    for(i=0;i<reg_dec;i++,pout++) {
        reg2 = MB_vec8_load(pout);
        reg2 = MB_vec8_max(reg2,edge);
        MB_vec8_store(pout,reg2);
    }
    
#undef CASE_INTRA_SHIFT

#else
    PLINE pin, pout;

    /* count cannot exceed the number of pixels in a line */
    count = ((Uint32) count)<bytes_in ? count : bytes_in;
    
    pin = (PLINE) (*p_in + count);
    pout = (PLINE) (*p_out);
    
    for(i=0; i<(bytes_in-((Uint32) count)); i++,pout++,pin++) {
        (*pout) = (*pout)>(*pin) ? (*pout) : (*pin);
    }
    for(i=0; i<((Uint32) count); i++,pout++) {
        (*pout) = (*pout)>(fill_val) ? (*pout) : (fill_val);
    }
#endif
}

/*
 * Used to displace a complete line in the right direction.
 * \param p_out pointer on the destination image pixel line
 * \param p_in pointer on the source image pixel line
 * \param bytes_in number of bytes inside the line
 * \param count the shift amplitude
 * \param fill_val the value used to fill the line
 */
static INLINE void SHIFT_LINE_RIGHT(PLINE *p_out, PLINE *p_in,
                                    Uint32 bytes_in, Sint32 count, Uint32 fill_val)
{
    Uint32 i;

#ifdef MB_VECTORIZATION_8

#define CASE_INTRA_SHIFT(VAL) \
                case VAL: \
                    reg1 = MB_vec8_shlft(reg1, VAL); \
                    reg2 = MB_vec8_shrgt(reg2, MB_vec8_size-VAL); \
                    break;
                    
    Uint32 reg_dec, ins_reg_dec;
    MB_Vector8 reg1, reg2;
    MB_Vector8 edge, *pin, *pout;

    /* count cannot exceed the number of pixel in a line */
    reg_dec = ((Uint32) count)<bytes_in ? count/MB_vec8_size : bytes_in/MB_vec8_size;
    ins_reg_dec = count % MB_vec8_size;

    edge = MB_vec8_set32(fill_val);
    pin = (MB_Vector8 *) (*p_in + bytes_in - (reg_dec+1)*sizeof(MB_Vector8));
    pout = (MB_Vector8 *) (*p_out + bytes_in - sizeof(MB_Vector8));
    
    if (ins_reg_dec==0) {
        /* no intra register shifting */
        for(i=0;i<(bytes_in-reg_dec*sizeof(MB_Vector8));i+=sizeof(MB_Vector8),pin--,pout--) {
            reg1 = MB_vec8_load(pin);
            reg2 = MB_vec8_load(pout);
            reg2 = MB_vec8_max(reg2, reg1);
            MB_vec8_store(pout,reg2);
        }
    } else {
        /* intra register shifting */
        for(i=0;i<(bytes_in-reg_dec*sizeof(MB_Vector8));i+=sizeof(MB_Vector8),pin--,pout--) {
            reg1 = MB_vec8_load(pin);
            reg2 = (i==(bytes_in-(reg_dec+1)*MB_vec8_size)) ? edge : MB_vec8_load(pin-1);
            switch(ins_reg_dec) {
                CASE_INTRA_SHIFT(1)
                CASE_INTRA_SHIFT(2)
                CASE_INTRA_SHIFT(3)
                CASE_INTRA_SHIFT(4)
                CASE_INTRA_SHIFT(5)
                CASE_INTRA_SHIFT(6)
                CASE_INTRA_SHIFT(7)
                CASE_INTRA_SHIFT(8)
                CASE_INTRA_SHIFT(9)
                CASE_INTRA_SHIFT(10)
                CASE_INTRA_SHIFT(11)
                CASE_INTRA_SHIFT(12)
                CASE_INTRA_SHIFT(13)
                CASE_INTRA_SHIFT(14)
                CASE_INTRA_SHIFT(15)
                default:
                case 0:
                    break;
            }
            reg1 = MB_vec8_or(reg1, reg2);
            reg2 = MB_vec8_load(pout);
            reg2 = MB_vec8_max(reg2,reg1);
            MB_vec8_store(pout,reg2);
        }
    }
    
    /* The created space is filled with the fill value */
    for(i=0;i<reg_dec;i++,pout--) {
        reg2 = MB_vec8_load(pout);
        reg2 = MB_vec8_max(reg2,edge);
        MB_vec8_store(pout,reg2);
    }
    
#undef CASE_INTRA_SHIFT

#else
    PLINE pin, pout;

    /* count cannot exceed the number of pixels in a line */
    count = ((Uint32) count)<bytes_in ? count : bytes_in;
    
    pin = (PLINE) (*p_in + bytes_in -1 - count);
    pout = (PLINE) (*p_out + bytes_in -1);
    
    for(i=0;i<bytes_in-((Uint32) count);i++,pin--,pout--) {
        (*pout) = (*pout)>(*pin) ? (*pout) : (*pin);
    }
    for(i=0; i<((Uint32) count); i++,pout--) {
        (*pout) = (*pout)>(fill_val) ? (*pout) : (fill_val);
    }
#endif
}

/****************************************/
/* Direction functions                  */
/****************************************/
/* The functions are described in a separate file to communalize with other */
/* shift functions */
/* Data type of the value used to represent the edge */
#define EDGE_TYPE Uint32
#include "MB_ShftVector.h"
#undef EDGE_TYPE

/****************************************/
/* Main function                        */
/****************************************/

/*
 * Looks for the maximum between two grey scale image pixels (a central pixel
 * and its neighbor in the other image previously shifted by the given vector)
 *
 * \param src source image in which the neighbor are taken
 * \param srcdest source of the central pixel and destination image
 * \param dx the vector amplitude in x
 * \param dy the vector amplitude in y
 * \param edge the kind of edge to use (behavior for pixels near edge depends on it)
 *
 * \return An error code (NO_ERR if successful)
 */
MB_errcode MB_SupVector8(MB_Image *src, MB_Image *srcdest, Sint32 dx, Sint32 dy, enum MB_edgemode_t edge)
{
    Uint32 bytes_in;
    PLINE *plines_in, *plines_out;
    VECFUNC *fn;

    /* error management */
    /* verification over image size compatibility */
    if (!MB_CHECK_SIZE_2(src, srcdest)) {
        return ERR_BAD_SIZE;
    }
    /* Only binary and greyscale images can be processed */
    switch (MB_PROBE_PAIR(src, srcdest)) {
    case MB_PAIR_8_8:
        break;
    default:
        return ERR_BAD_DEPTH;
    }

    /* setting up pointers */
    plines_in = src->plines;
    plines_out = srcdest->plines;
    bytes_in = MB_LINE_COUNT(src);

    /* Calling the corresponding function which depends on the orientation */
    /* of the vector */
    fn = orientationFunc[CODE_ORIENTATION(dx,dy)];
    fn(plines_out, plines_in, bytes_in, (Sint32) src->height, dx, dy, GREY_FILL_VALUE(edge));

    return NO_ERR;
}
