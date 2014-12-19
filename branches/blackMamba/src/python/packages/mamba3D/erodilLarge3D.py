"""
Erosion and dilation 3D operators for large structuring elements.

This module provides a set of functions and class to perform erosions and
dilations with large structuring elements.
"""

# Contributors : Serge BEUCHER

import mamba3D as m3D
import mamba
import mamba.core as core

def fastShift3D(imIn, imOut, d, amp, fill, grid=m3D.DEFAULT_GRID3D):
    """
    Shifts 3D image 'imIn' in direction 'd' of the 'grid' over an amplitude of
    'amp'. The emptied space is filled with 'fill' value.
    This implementation is faster than the previous one.
    The result is put in 'imOut'.
    """
    (width,height,length) = imIn.getSize()
    if length!=len(imOut):
        mamba.raiseExceptionOnError(core.MB_ERR_BAD_SIZE)
    # Cubic grid case (the simplest one).
    if grid.getCValue() == m3D.CUBIC.getCValue():
        # The above statement is an (ugly) way to get the grid in use.
        if d < 9:
            # Horizontal shift.
            for i in range(length):
                mamba.shift( imIn[i], imOut[i], d, amp, fill, grid=mamba.SQUARE)
            start = 0
            end = 0
        elif d < 18:
            # Downwards shift.
            hd = d - 9
            for i in range(amp, length):
                j = i - amp
                mamba.shift( imIn[i], imOut[j], hd, amp, fill, grid=mamba.SQUARE)
            start = max(length - amp, 0)
            end = length
        else:
            # Upwards shift.
            hd = d - 18
            for i in range(length - amp - 1, -1, -1):
                j = i + amp
                mamba.shift( imIn[i], imOut[j], hd, amp, fill, grid=mamba.SQUARE)
            start = 0
            end = min(amp, length)
    # Centered cubic grid.
    elif grid.getCValue() == m3D.CENTER_CUBIC.getCValue():
        if d < 9:
            # Horizontal shift.
            for i in range(length):
                mamba.shift( imIn[i], imOut[i], d, amp, fill, grid=mamba.SQUARE)
            start = 0
            end = 0
        elif d < 13:
            # Downwards shift.
            hd1 = m3D.CENTER_CUBIC.convertFromDir(d, 0)[1]
            hd2 = m3D.CENTER_CUBIC.convertFromDir(d, 1)[1]
            amp1 = amp//2 + amp%2
            for i in range(amp, length):
                j = i - amp
                amp2 = amp//2 + i%2
                mamba.shift( imIn[i], imOut[j], hd1, amp1, fill, grid=mamba.SQUARE)
                if hd2 <> 0:
                    mamba.shift(imOut[j], imOut[j], hd2, amp2, fill, grid=mamba.SQUARE) 
            start = max(length - amp, 0)
            end = length
        else:
            # Upwards shift.
            hd1 = m3D.CENTER_CUBIC.convertFromDir(d, 0)[1]
            hd2 = m3D.CENTER_CUBIC.convertFromDir(d, 1)[1]
            amp1 = amp//2 + amp%2
            for i in range(length - amp - 1, -1, -1):
                j = i + amp
                amp2 = amp//2 + i%2
                mamba.shift( imIn[i], imOut[j], hd1, amp1, fill, grid=mamba.SQUARE)
                if hd2 <> 0:
                    mamba.shift(imOut[j], imOut[j], hd2, amp2, fill, grid=mamba.SQUARE)                 
            start = 0
            end = min(amp, length)
    # Face centered cubic grid.
    else:
        if d < 7:
            # Horizontal shift.
            for i in range(length):
                mamba.shift( imIn[i], imOut[i], d, amp, fill, grid=mamba.HEXAGONAL)
            start = 0
            end = 0
        elif d < 9:
            # Downwards shift.
            extraS = (((0,0,0),(1,0,0),(1,0,1)),((0,0,0),(0,1,0),(1,1,0)),((0,0,0),(0,0,1),(1,1,1)))
            hdList = [m3D.FACE_CENTER_CUBIC.convertFromDir(d, i)[1] for i in range(3)]
            dirUse = [0, 1, 2]
            v = hdList.index(0)
            del dirUse[v]
            for i in range(amp, length):
                j = i - amp
                amp1 = amp//3 + extraS[i%3][amp%3][dirUse[0]]
                mamba.shift( imIn[i], imOut[j], hdList[dirUse[0]], amp1, fill, grid=mamba.HEXAGONAL)
                amp1 = amp//3 + extraS[i%3][amp%3][dirUse[1]]
                mamba.shift(imOut[j], imOut[j], hdList[dirUse[1]], amp1, fill, grid=mamba.HEXAGONAL) 
            start = max(length - amp, 0)
            end = length
        elif d == 9:
            # Specific algorithm is setup for direction 9 to avoid edge effects.
            extraS = (((0,0),(0,1),(1,0)),((0,0),(0,0),(0,1)),((0,0),(0,1),(0,1)))
            hdList = [m3D.FACE_CENTER_CUBIC.convertFromDir(d, i)[1] for i in range(3)]
            for i in range(amp, length):
                j = i - amp
                (sc, sh) = extraS[i%3][amp%3]
                nc = (amp//3 +sc) * 2
                mamba.shift( imIn[i], imOut[j], 1, nc, fill, grid=mamba.SQUARE)
                if sh <> 0:
                    if (i%3) == 2:
                        hd = 1
                    else:
                        hd = 6
                    mamba.shift(imOut[j], imOut[j], hd, 1, fill, grid=mamba.HEXAGONAL) 
            start = length - amp
            end = length            
        elif d < 12:
            # Upwards shift.
            extraS = (((0,0,0),(1,0,0),(1,1,0)),((0,0,0),(0,1,0),(0,1,1)),((0,0,0),(0,0,1),(1,0,1)))
            hdList = [m3D.FACE_CENTER_CUBIC.convertFromDir(d, i)[1] for i in range(3)]
            dirUse = [0, 1, 2]
            v = hdList.index(0)
            del dirUse[v]
            for i in range(length - amp - 1, -1, -1):
                j = i + amp
                amp1 = amp//3 + extraS[i%3][amp%3][dirUse[0]]
                mamba.shift( imIn[i], imOut[j], hdList[dirUse[0]], amp1, fill, grid=mamba.HEXAGONAL)
                amp1 = amp//3 + extraS[i%3][amp%3][dirUse[1]]
                mamba.shift(imOut[j], imOut[j], hdList[dirUse[1]], amp1, fill, grid=mamba.HEXAGONAL) 
            start = 0
            end = min(amp, length)
        else:
            # Specific algorithm for direction 12.
            extraS = (((0,0),(0,0),(0,1)),((0,0),(0,1),(1,0)),((0,0),(0,1),(0,1)))
            hdList = [m3D.FACE_CENTER_CUBIC.convertFromDir(d, i)[1] for i in range(3)]
            for i in range(length - amp - 1, -1, -1):
                j = i + amp
                (sc, sh) = extraS[i%3][amp%3]
                nc = (amp//3 +sc) * 2
                mamba.shift( imIn[i], imOut[j], 1, nc, fill, grid=mamba.SQUARE)
                if sh <> 0:
                    if (i%3) == 2:
                        hd = 3
                    else:
                        hd = 4
                    mamba.shift(imOut[j], imOut[j], hd, 1, fill, grid=mamba.HEXAGONAL) 
            start = 0
            end = min(amp, length)           
    for i in range(start, end):
        imOut[i].fill(fill)
        
 