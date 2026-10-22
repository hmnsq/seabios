/*
  LzmaDecode.c
  LZMA Decoder (optimized for Speed version)
  
  LZMA SDK 4.40 Copyright (c) 1999-2006 Igor Pavlov (2006-05-01)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and 
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this Code, expressly permits you to 
  statically or dynamically link your Code (or bind by name) to the 
  interfaces of this file without subjecting your linked Code to the 
  terms of the CPL or GNU LGPL. Any modifications or additions 
  to this file, however, are subject to the LGPL or CPL terms.
*/

#अन्तर्भाव "lzmadecode.h"

#define k_अंक_शीर्ष_Bits 24
#define k_शीर्ष_मूल्य ((UInt32)1 << k_अंक_शीर्ष_Bits)

#define k_अंक_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_अंक_Bit_Model_Total_Bits)
#define k_अंक_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { पूर्णांक i; साठी(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { जर (Buffer == BufferLim) परत LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE जर (Range < k_शीर्ष_मूल्य) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_अंक_Bit_Model_Total_Bits) * *(p); जर (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_अंक_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_अंक_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } अन्यथा \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_वृक्ष_Decode(probs, अंक_Levels, res) \
  { पूर्णांक i = अंक_Levels; res = 1; \
  करा { CProb *cp = probs + res; RC_GET_BIT(cp, res) } पर्यंत(--i != 0); \
  res -= (1 << अंक_Levels); }


#define k_अंक_Pos_Bits_Max 4
#define k_अंक_Pos_States_Max (1 << k_अंक_Pos_Bits_Max)

#define k_Len_अंक_खालचा_Bits 3
#define k_Len_अंक_खालचा_Symbols (1 << k_Len_अंक_खालचा_Bits)
#define k_Len_अंक_Mid_Bits 3
#define k_Len_अंक_Mid_Symbols (1 << k_Len_अंक_Mid_Bits)
#define k_Len_अंक_वरचा_Bits 8
#define k_Len_अंक_वरचा_Symbols (1 << k_Len_अंक_वरचा_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_खालचा (LenChoice2 + 1)
#define LenMid (Len_खालचा + (k_अंक_Pos_States_Max << k_Len_अंक_खालचा_Bits))
#define Len_वरचा (LenMid + (k_अंक_Pos_States_Max << k_Len_अंक_Mid_Bits))
#define k_अंक_Len_Probs (Len_वरचा + k_Len_अंक_वरचा_Symbols) 


#define k_अंक_States 12
#define k_अंक_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_अंक_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_अंक_Pos_Slot_Bits 6
#define k_अंक_Len_To_Pos_States 4

#define k_अंक_Align_Bits 4
#define kAlignTableSize (1 << k_अंक_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_अंक_States << k_अंक_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_अंक_States)
#define IsRepG1 (IsRepG0 + k_अंक_States)
#define IsRepG2 (IsRepG1 + k_अंक_States)
#define IsRep0Long (IsRepG2 + k_अंक_States)
#define PosSlot (IsRep0Long + (k_अंक_States << k_अंक_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_अंक_Len_To_Pos_States << k_अंक_Pos_Slot_Bits))
#define Align (SpecPos + k_अंक_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_अंक_Len_Probs)
#define Literal (RepLenCoder + k_अंक_Len_Probs)

#जर Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

पूर्णांक LzmaDecodeProperties(CLzmaProperties *propsRes, स्थिरांक अचिन्हित अक्षर *propsData, पूर्णांक size)
{
  अचिन्हित अक्षर prop0;
  जर (size < LZMA_PROPERTIES_SIZE)
    परत LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  जर (prop0 >= (9 * 5 * 5))
    परत LZMA_RESULT_DATA_ERROR;
  {
    साठी (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    साठी (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  परत LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

पूर्णांक LzmaDecode(CLzmaDecoderState *vs,
    स्थिरांक अचिन्हित अक्षर *inStream, SizeT inSize, SizeT *inSizeProcessed,
    अचिन्हित अक्षर *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  पूर्णांक lc = vs->Properties.lc;


  पूर्णांक state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  पूर्णांक len = 0;
  स्थिरांक Byte *Buffer;
  स्थिरांक Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 अंक_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    साठी (i = 0; i < अंक_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  पर्यंत(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    पूर्णांक posState = (पूर्णांक)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_अंक_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      पूर्णांक चिन्ह_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      जर (state >= k_अंक_Lit_States)
      {
        पूर्णांक matchByte;
        matchByte = outStream[nowPos - rep0];
        करा
        {
          पूर्णांक bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + चिन्ह_2;
          RC_GET_BIT2(probLit, चिन्ह_2, जर (bit != 0) थांबा, जर (bit == 0) थांबा)
        }
        पर्यंत (चिन्ह_2 < 0x100);
      }
      पर्यंत (चिन्ह_2 < 0x100)
      {
        CProb *probLit = prob + चिन्ह_2;
        RC_GET_BIT(probLit, चिन्ह_2)
      }
      previousByte = (Byte)चिन्ह_2;

      outStream[nowPos++] = previousByte;
      जर (state < 4) state = 0;
      अन्यथा जर (state < 10) state -= 3;
      अन्यथा state -= 6;
    }
    अन्यथा             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_अंक_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      अन्यथा
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_अंक_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            जर (nowPos == 0)
              परत LZMA_RESULT_DATA_ERROR;
            
            state = state < k_अंक_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            सुरूठेवा;
          }
          अन्यथा
          {
            UpdateBit1(prob);
          }
        }
        अन्यथा
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          अन्यथा 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            अन्यथा
            {
              UpdateBit1(prob);
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        state = state < k_अंक_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        पूर्णांक अंक_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_खालचा + (posState << k_Len_अंक_खालचा_Bits);
          offset = 0;
          अंक_Bits = k_Len_अंक_खालचा_Bits;
        }
        अन्यथा
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_अंक_Mid_Bits);
            offset = k_Len_अंक_खालचा_Symbols;
            अंक_Bits = k_Len_अंक_Mid_Bits;
          }
          अन्यथा
          {
            UpdateBit1(probLen);
            probLen = prob + Len_वरचा;
            offset = k_Len_अंक_खालचा_Symbols + k_Len_अंक_Mid_Symbols;
            अंक_Bits = k_Len_अंक_वरचा_Bits;
          }
        }
        Range_Decoder_Bit_वृक्ष_Decode(probLen, अंक_Bits, len);
        len += offset;
      }

      जर (state < 4)
      {
        पूर्णांक posSlot;
        state += k_अंक_Lit_States;
        prob = p + PosSlot +
            ((len < k_अंक_Len_To_Pos_States ? len : k_अंक_Len_To_Pos_States - 1) << 
            k_अंक_Pos_Slot_Bits);
        Range_Decoder_Bit_वृक्ष_Decode(prob, k_अंक_Pos_Slot_Bits, posSlot);
        जर (posSlot >= kStartPosModelIndex)
        {
          पूर्णांक अंक_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          जर (posSlot < kEndPosModelIndex)
          {
            rep0 <<= अंक_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          अन्यथा
          {
            अंक_Direct_Bits -= k_अंक_Align_Bits;
            करा
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              जर (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            पर्यंत (--अंक_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_अंक_Align_Bits;
            अंक_Direct_Bits = k_अंक_Align_Bits;
          }
          {
            पूर्णांक i = 1;
            पूर्णांक mi = 1;
            करा
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            पर्यंत(--अंक_Direct_Bits != 0);
          }
        }
        अन्यथा
          rep0 = posSlot;
        जर (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          थांबा;
        }
      }

      len += kMatchMinLen;
      जर (rep0 > nowPos)
        परत LZMA_RESULT_DATA_ERROR;


      करा
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      पर्यंत(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  परत LZMA_RESULT_OK;
}
