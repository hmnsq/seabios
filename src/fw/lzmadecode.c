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

#ಅನ್ತರ್ಭಾವ "lzmadecode.h"

#define k_ಅಂಕೆ_ತುದಿ_Bits 24
#define k_ತುದಿ_ಮೌಲ್ಯ ((UInt32)1 << k_ಅಂಕೆ_ತುದಿ_Bits)

#define k_ಅಂಕೆ_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_ಅಂಕೆ_Bit_Model_Total_Bits)
#define k_ಅಂಕೆ_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { ಪೂರ್ಣಾಙ್ಕ i; ಪ್ರತಿ(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { ಯದಿ (Buffer == BufferLim) ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE ಯದಿ (Range < k_ತುದಿ_ಮೌಲ್ಯ) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_ಅಂಕೆ_Bit_Model_Total_Bits) * *(p); ಯದಿ (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_ಅಂಕೆ_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_ಅಂಕೆ_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } ಅನ್ಯಥಾ \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_ವೃಕ್ಷ_Decode(probs, ಅಂಕೆ_Levels, res) \
  { ಪೂರ್ಣಾಙ್ಕ i = ಅಂಕೆ_Levels; res = 1; \
  ಕುರು { CProb *cp = probs + res; RC_GET_BIT(cp, res) } ಯಾವತ್(--i != 0); \
  res -= (1 << ಅಂಕೆ_Levels); }


#define k_ಅಂಕೆ_Pos_Bits_Max 4
#define k_ಅಂಕೆ_Pos_States_Max (1 << k_ಅಂಕೆ_Pos_Bits_Max)

#define k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Bits 3
#define k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Symbols (1 << k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Bits)
#define k_Len_ಅಂಕೆ_Mid_Bits 3
#define k_Len_ಅಂಕೆ_Mid_Symbols (1 << k_Len_ಅಂಕೆ_Mid_Bits)
#define k_Len_ಅಂಕೆ_ಮೇಲಿನ_Bits 8
#define k_Len_ಅಂಕೆ_ಮೇಲಿನ_Symbols (1 << k_Len_ಅಂಕೆ_ಮೇಲಿನ_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_ಕೆಳಗಿನ (LenChoice2 + 1)
#define LenMid (Len_ಕೆಳಗಿನ + (k_ಅಂಕೆ_Pos_States_Max << k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Bits))
#define Len_ಮೇಲಿನ (LenMid + (k_ಅಂಕೆ_Pos_States_Max << k_Len_ಅಂಕೆ_Mid_Bits))
#define k_ಅಂಕೆ_Len_Probs (Len_ಮೇಲಿನ + k_Len_ಅಂಕೆ_ಮೇಲಿನ_Symbols) 


#define k_ಅಂಕೆ_States 12
#define k_ಅಂಕೆ_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_ಅಂಕೆ_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_ಅಂಕೆ_Pos_Slot_Bits 6
#define k_ಅಂಕೆ_Len_To_Pos_States 4

#define k_ಅಂಕೆ_Align_Bits 4
#define kAlignTableSize (1 << k_ಅಂಕೆ_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_ಅಂಕೆ_States << k_ಅಂಕೆ_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_ಅಂಕೆ_States)
#define IsRepG1 (IsRepG0 + k_ಅಂಕೆ_States)
#define IsRepG2 (IsRepG1 + k_ಅಂಕೆ_States)
#define IsRep0Long (IsRepG2 + k_ಅಂಕೆ_States)
#define PosSlot (IsRep0Long + (k_ಅಂಕೆ_States << k_ಅಂಕೆ_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_ಅಂಕೆ_Len_To_Pos_States << k_ಅಂಕೆ_Pos_Slot_Bits))
#define Align (SpecPos + k_ಅಂಕೆ_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_ಅಂಕೆ_Len_Probs)
#define Literal (RepLenCoder + k_ಅಂಕೆ_Len_Probs)

#ಯದಿ Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

ಪೂರ್ಣಾಙ್ಕ LzmaDecodeProperties(CLzmaProperties *propsRes, ಧ್ರುವ unsigned ವರ್ಣ *propsData, ಪೂರ್ಣಾಙ್ಕ size)
{
  unsigned ವರ್ಣ prop0;
  ಯದಿ (size < LZMA_PROPERTIES_SIZE)
    ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  ಯದಿ (prop0 >= (9 * 5 * 5))
    ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_DATA_ERROR;
  {
    ಪ್ರತಿ (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    ಪ್ರತಿ (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

ಪೂರ್ಣಾಙ್ಕ LzmaDecode(CLzmaDecoderState *vs,
    ಧ್ರುವ unsigned ವರ್ಣ *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned ವರ್ಣ *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  ಪೂರ್ಣಾಙ್ಕ lc = vs->Properties.lc;


  ಪೂರ್ಣಾಙ್ಕ state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  ಪೂರ್ಣಾಙ್ಕ len = 0;
  ಧ್ರುವ Byte *Buffer;
  ಧ್ರುವ Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 ಅಂಕೆ_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    ಪ್ರತಿ (i = 0; i < ಅಂಕೆ_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  ಯಾವತ್(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    ಪೂರ್ಣಾಙ್ಕ posState = (ಪೂರ್ಣಾಙ್ಕ)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_ಅಂಕೆ_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      ಪೂರ್ಣಾಙ್ಕ ಚಿಹ್ನೆ_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      ಯದಿ (state >= k_ಅಂಕೆ_Lit_States)
      {
        ಪೂರ್ಣಾಙ್ಕ matchByte;
        matchByte = outStream[nowPos - rep0];
        ಕುರು
        {
          ಪೂರ್ಣಾಙ್ಕ bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + ಚಿಹ್ನೆ_2;
          RC_GET_BIT2(probLit, ಚಿಹ್ನೆ_2, ಯದಿ (bit != 0) ಭಙ್ಗ, ಯದಿ (bit == 0) ಭಙ್ಗ)
        }
        ಯಾವತ್ (ಚಿಹ್ನೆ_2 < 0x100);
      }
      ಯಾವತ್ (ಚಿಹ್ನೆ_2 < 0x100)
      {
        CProb *probLit = prob + ಚಿಹ್ನೆ_2;
        RC_GET_BIT(probLit, ಚಿಹ್ನೆ_2)
      }
      previousByte = (Byte)ಚಿಹ್ನೆ_2;

      outStream[nowPos++] = previousByte;
      ಯದಿ (state < 4) state = 0;
      ಅನ್ಯಥಾ ಯದಿ (state < 10) state -= 3;
      ಅನ್ಯಥಾ state -= 6;
    }
    ಅನ್ಯಥಾ             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_ಅಂಕೆ_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      ಅನ್ಯಥಾ
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_ಅಂಕೆ_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            ಯದಿ (nowPos == 0)
              ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_DATA_ERROR;
            
            state = state < k_ಅಂಕೆ_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            ಅನುವರ್ತ;
          }
          ಅನ್ಯಥಾ
          {
            UpdateBit1(prob);
          }
        }
        ಅನ್ಯಥಾ
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          ಅನ್ಯಥಾ 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            ಅನ್ಯಥಾ
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
        state = state < k_ಅಂಕೆ_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_ಕೆಳಗಿನ + (posState << k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Bits);
          offset = 0;
          ಅಂಕೆ_Bits = k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Bits;
        }
        ಅನ್ಯಥಾ
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_ಅಂಕೆ_Mid_Bits);
            offset = k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Symbols;
            ಅಂಕೆ_Bits = k_Len_ಅಂಕೆ_Mid_Bits;
          }
          ಅನ್ಯಥಾ
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ಮೇಲಿನ;
            offset = k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Symbols + k_Len_ಅಂಕೆ_Mid_Symbols;
            ಅಂಕೆ_Bits = k_Len_ಅಂಕೆ_ಮೇಲಿನ_Bits;
          }
        }
        Range_Decoder_Bit_ವೃಕ್ಷ_Decode(probLen, ಅಂಕೆ_Bits, len);
        len += offset;
      }

      ಯದಿ (state < 4)
      {
        ಪೂರ್ಣಾಙ್ಕ posSlot;
        state += k_ಅಂಕೆ_Lit_States;
        prob = p + PosSlot +
            ((len < k_ಅಂಕೆ_Len_To_Pos_States ? len : k_ಅಂಕೆ_Len_To_Pos_States - 1) << 
            k_ಅಂಕೆ_Pos_Slot_Bits);
        Range_Decoder_Bit_ವೃಕ್ಷ_Decode(prob, k_ಅಂಕೆ_Pos_Slot_Bits, posSlot);
        ಯದಿ (posSlot >= kStartPosModelIndex)
        {
          ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          ಯದಿ (posSlot < kEndPosModelIndex)
          {
            rep0 <<= ಅಂಕೆ_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          ಅನ್ಯಥಾ
          {
            ಅಂಕೆ_Direct_Bits -= k_ಅಂಕೆ_Align_Bits;
            ಕುರು
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              ಯದಿ (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            ಯಾವತ್ (--ಅಂಕೆ_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_ಅಂಕೆ_Align_Bits;
            ಅಂಕೆ_Direct_Bits = k_ಅಂಕೆ_Align_Bits;
          }
          {
            ಪೂರ್ಣಾಙ್ಕ i = 1;
            ಪೂರ್ಣಾಙ್ಕ mi = 1;
            ಕುರು
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            ಯಾವತ್(--ಅಂಕೆ_Direct_Bits != 0);
          }
        }
        ಅನ್ಯಥಾ
          rep0 = posSlot;
        ಯದಿ (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          ಭಙ್ಗ;
        }
      }

      len += kMatchMinLen;
      ಯದಿ (rep0 > nowPos)
        ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_DATA_ERROR;


      ಕುರು
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      ಯಾವತ್(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  ಪ್ರತ್ಯಾಗಮ LZMA_RESULT_OK;
}
