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

#define k_अङ्कः_शिखरम्_Bits 24
#define k_शिखरम्_मानम् ((UInt32)1 << k_अङ्कः_शिखरम्_Bits)

#define k_अङ्कः_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_अङ्कः_Bit_Model_Total_Bits)
#define k_अङ्कः_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { पूर्णाङ्क i; अत्थाय(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { सचे (Buffer == BufferLim) पटिनिवत्त LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE सचे (Range < k_शिखरम्_मानम्) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_अङ्कः_Bit_Model_Total_Bits) * *(p); सचे (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_अङ्कः_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_अङ्कः_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } अञ्ञथा \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_वृक्षः_Decode(probs, अङ्कः_Levels, res) \
  { पूर्णाङ्क i = अङ्कः_Levels; res = 1; \
  करोहि { CProb *cp = probs + res; RC_GET_BIT(cp, res) } याव(--i != 0); \
  res -= (1 << अङ्कः_Levels); }


#define k_अङ्कः_Pos_Bits_Max 4
#define k_अङ्कः_Pos_States_Max (1 << k_अङ्कः_Pos_Bits_Max)

#define k_Len_अङ्कः_नीचः_Bits 3
#define k_Len_अङ्कः_नीचः_Symbols (1 << k_Len_अङ्कः_नीचः_Bits)
#define k_Len_अङ्कः_Mid_Bits 3
#define k_Len_अङ्कः_Mid_Symbols (1 << k_Len_अङ्कः_Mid_Bits)
#define k_Len_अङ्कः_उच्चः_Bits 8
#define k_Len_अङ्कः_उच्चः_Symbols (1 << k_Len_अङ्कः_उच्चः_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_नीचः (LenChoice2 + 1)
#define LenMid (Len_नीचः + (k_अङ्कः_Pos_States_Max << k_Len_अङ्कः_नीचः_Bits))
#define Len_उच्चः (LenMid + (k_अङ्कः_Pos_States_Max << k_Len_अङ्कः_Mid_Bits))
#define k_अङ्कः_Len_Probs (Len_उच्चः + k_Len_अङ्कः_उच्चः_Symbols) 


#define k_अङ्कः_States 12
#define k_अङ्कः_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_अङ्कः_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_अङ्कः_Pos_Slot_Bits 6
#define k_अङ्कः_Len_To_Pos_States 4

#define k_अङ्कः_Align_Bits 4
#define kAlignTableSize (1 << k_अङ्कः_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_अङ्कः_States << k_अङ्कः_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_अङ्कः_States)
#define IsRepG1 (IsRepG0 + k_अङ्कः_States)
#define IsRepG2 (IsRepG1 + k_अङ्कः_States)
#define IsRep0Long (IsRepG2 + k_अङ्कः_States)
#define PosSlot (IsRep0Long + (k_अङ्कः_States << k_अङ्कः_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_अङ्कः_Len_To_Pos_States << k_अङ्कः_Pos_Slot_Bits))
#define Align (SpecPos + k_अङ्कः_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_अङ्कः_Len_Probs)
#define Literal (RepLenCoder + k_अङ्कः_Len_Probs)

#सचे Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

पूर्णाङ्क LzmaDecodeProperties(CLzmaProperties *propsRes, ध्रुव unsigned वर्ण *propsData, पूर्णाङ्क size)
{
  unsigned वर्ण prop0;
  सचे (size < LZMA_PROPERTIES_SIZE)
    पटिनिवत्त LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  सचे (prop0 >= (9 * 5 * 5))
    पटिनिवत्त LZMA_RESULT_DATA_ERROR;
  {
    अत्थाय (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    अत्थाय (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  पटिनिवत्त LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

पूर्णाङ्क LzmaDecode(CLzmaDecoderState *vs,
    ध्रुव unsigned वर्ण *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned वर्ण *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  पूर्णाङ्क lc = vs->Properties.lc;


  पूर्णाङ्क state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  पूर्णाङ्क len = 0;
  ध्रुव Byte *Buffer;
  ध्रुव Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 अङ्कः_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    अत्थाय (i = 0; i < अङ्कः_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  याव(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    पूर्णाङ्क posState = (पूर्णाङ्क)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_अङ्कः_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      पूर्णाङ्क चिह्नम्_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      सचे (state >= k_अङ्कः_Lit_States)
      {
        पूर्णाङ्क matchByte;
        matchByte = outStream[nowPos - rep0];
        करोहि
        {
          पूर्णाङ्क bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + चिह्नम्_2;
          RC_GET_BIT2(probLit, चिह्नम्_2, सचे (bit != 0) भङ्ग, सचे (bit == 0) भङ्ग)
        }
        याव (चिह्नम्_2 < 0x100);
      }
      याव (चिह्नम्_2 < 0x100)
      {
        CProb *probLit = prob + चिह्नम्_2;
        RC_GET_BIT(probLit, चिह्नम्_2)
      }
      previousByte = (Byte)चिह्नम्_2;

      outStream[nowPos++] = previousByte;
      सचे (state < 4) state = 0;
      अञ्ञथा सचे (state < 10) state -= 3;
      अञ्ञथा state -= 6;
    }
    अञ्ञथा             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_अङ्कः_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      अञ्ञथा
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_अङ्कः_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            सचे (nowPos == 0)
              पटिनिवत्त LZMA_RESULT_DATA_ERROR;
            
            state = state < k_अङ्कः_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            अनुवर्त;
          }
          अञ्ञथा
          {
            UpdateBit1(prob);
          }
        }
        अञ्ञथा
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          अञ्ञथा 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            अञ्ञथा
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
        state = state < k_अङ्कः_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        पूर्णाङ्क अङ्कः_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_नीचः + (posState << k_Len_अङ्कः_नीचः_Bits);
          offset = 0;
          अङ्कः_Bits = k_Len_अङ्कः_नीचः_Bits;
        }
        अञ्ञथा
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_अङ्कः_Mid_Bits);
            offset = k_Len_अङ्कः_नीचः_Symbols;
            अङ्कः_Bits = k_Len_अङ्कः_Mid_Bits;
          }
          अञ्ञथा
          {
            UpdateBit1(probLen);
            probLen = prob + Len_उच्चः;
            offset = k_Len_अङ्कः_नीचः_Symbols + k_Len_अङ्कः_Mid_Symbols;
            अङ्कः_Bits = k_Len_अङ्कः_उच्चः_Bits;
          }
        }
        Range_Decoder_Bit_वृक्षः_Decode(probLen, अङ्कः_Bits, len);
        len += offset;
      }

      सचे (state < 4)
      {
        पूर्णाङ्क posSlot;
        state += k_अङ्कः_Lit_States;
        prob = p + PosSlot +
            ((len < k_अङ्कः_Len_To_Pos_States ? len : k_अङ्कः_Len_To_Pos_States - 1) << 
            k_अङ्कः_Pos_Slot_Bits);
        Range_Decoder_Bit_वृक्षः_Decode(prob, k_अङ्कः_Pos_Slot_Bits, posSlot);
        सचे (posSlot >= kStartPosModelIndex)
        {
          पूर्णाङ्क अङ्कः_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          सचे (posSlot < kEndPosModelIndex)
          {
            rep0 <<= अङ्कः_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          अञ्ञथा
          {
            अङ्कः_Direct_Bits -= k_अङ्कः_Align_Bits;
            करोहि
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              सचे (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            याव (--अङ्कः_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_अङ्कः_Align_Bits;
            अङ्कः_Direct_Bits = k_अङ्कः_Align_Bits;
          }
          {
            पूर्णाङ्क i = 1;
            पूर्णाङ्क mi = 1;
            करोहि
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            याव(--अङ्कः_Direct_Bits != 0);
          }
        }
        अञ्ञथा
          rep0 = posSlot;
        सचे (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          भङ्ग;
        }
      }

      len += kMatchMinLen;
      सचे (rep0 > nowPos)
        पटिनिवत्त LZMA_RESULT_DATA_ERROR;


      करोहि
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      याव(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  पटिनिवत्त LZMA_RESULT_OK;
}
