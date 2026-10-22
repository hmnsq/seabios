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

#ଅନ୍ତର୍ଭାଵ "lzmadecode.h"

#define k_ଅଙ୍କ_ଶୀର୍ଷ_Bits 24
#define k_ଶୀର୍ଷ_ମୂଲ୍ୟ ((UInt32)1 << k_ଅଙ୍କ_ଶୀର୍ଷ_Bits)

#define k_ଅଙ୍କ_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_ଅଙ୍କ_Bit_Model_Total_Bits)
#define k_ଅଙ୍କ_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { ପୂର୍ଣ୍ଣସଂଖ୍ୟା i; ପାଇଁ(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { ଯଦି (Buffer == BufferLim) ଫେରନ୍ତୁ LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE ଯଦି (Range < k_ଶୀର୍ଷ_ମୂଲ୍ୟ) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_ଅଙ୍କ_Bit_Model_Total_Bits) * *(p); ଯଦି (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_ଅଙ୍କ_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_ଅଙ୍କ_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } ଅନ୍ୟଥା \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_ବୃକ୍ଷ_Decode(probs, ଅଙ୍କ_Levels, res) \
  { ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = ଅଙ୍କ_Levels; res = 1; \
  କର { CProb *cp = probs + res; RC_GET_BIT(cp, res) } ଯେତେବେଳେ(--i != 0); \
  res -= (1 << ଅଙ୍କ_Levels); }


#define k_ଅଙ୍କ_Pos_Bits_Max 4
#define k_ଅଙ୍କ_Pos_States_Max (1 << k_ଅଙ୍କ_Pos_Bits_Max)

#define k_Len_ଅଙ୍କ_ତଳ_Bits 3
#define k_Len_ଅଙ୍କ_ତଳ_Symbols (1 << k_Len_ଅଙ୍କ_ତଳ_Bits)
#define k_Len_ଅଙ୍କ_Mid_Bits 3
#define k_Len_ଅଙ୍କ_Mid_Symbols (1 << k_Len_ଅଙ୍କ_Mid_Bits)
#define k_Len_ଅଙ୍କ_ଉପର_Bits 8
#define k_Len_ଅଙ୍କ_ଉପର_Symbols (1 << k_Len_ଅଙ୍କ_ଉପର_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_ତଳ (LenChoice2 + 1)
#define LenMid (Len_ତଳ + (k_ଅଙ୍କ_Pos_States_Max << k_Len_ଅଙ୍କ_ତଳ_Bits))
#define Len_ଉପର (LenMid + (k_ଅଙ୍କ_Pos_States_Max << k_Len_ଅଙ୍କ_Mid_Bits))
#define k_ଅଙ୍କ_Len_Probs (Len_ଉପର + k_Len_ଅଙ୍କ_ଉପର_Symbols) 


#define k_ଅଙ୍କ_States 12
#define k_ଅଙ୍କ_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_ଅଙ୍କ_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_ଅଙ୍କ_Pos_Slot_Bits 6
#define k_ଅଙ୍କ_Len_To_Pos_States 4

#define k_ଅଙ୍କ_Align_Bits 4
#define kAlignTableSize (1 << k_ଅଙ୍କ_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_ଅଙ୍କ_States << k_ଅଙ୍କ_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_ଅଙ୍କ_States)
#define IsRepG1 (IsRepG0 + k_ଅଙ୍କ_States)
#define IsRepG2 (IsRepG1 + k_ଅଙ୍କ_States)
#define IsRep0Long (IsRepG2 + k_ଅଙ୍କ_States)
#define PosSlot (IsRep0Long + (k_ଅଙ୍କ_States << k_ଅଙ୍କ_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_ଅଙ୍କ_Len_To_Pos_States << k_ଅଙ୍କ_Pos_Slot_Bits))
#define Align (SpecPos + k_ଅଙ୍କ_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_ଅଙ୍କ_Len_Probs)
#define Literal (RepLenCoder + k_ଅଙ୍କ_Len_Probs)

#ଯଦି Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା LzmaDecodeProperties(CLzmaProperties *propsRes, ଅଚଳ unsigned ଅକ୍ଷର *propsData, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
  unsigned ଅକ୍ଷର prop0;
  ଯଦି (size < LZMA_PROPERTIES_SIZE)
    ଫେରନ୍ତୁ LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  ଯଦି (prop0 >= (9 * 5 * 5))
    ଫେରନ୍ତୁ LZMA_RESULT_DATA_ERROR;
  {
    ପାଇଁ (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    ପାଇଁ (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  ଫେରନ୍ତୁ LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା LzmaDecode(CLzmaDecoderState *vs,
    ଅଚଳ unsigned ଅକ୍ଷର *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned ଅକ୍ଷର *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lc = vs->Properties.lc;


  ପୂର୍ଣ୍ଣସଂଖ୍ୟା state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = 0;
  ଅଚଳ Byte *Buffer;
  ଅଚଳ Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 ଅଙ୍କ_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    ପାଇଁ (i = 0; i < ଅଙ୍କ_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  ଯେତେବେଳେ(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା posState = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_ଅଙ୍କ_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପ୍ରତୀକ_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      ଯଦି (state >= k_ଅଙ୍କ_Lit_States)
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା matchByte;
        matchByte = outStream[nowPos - rep0];
        କର
        {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + ପ୍ରତୀକ_2;
          RC_GET_BIT2(probLit, ପ୍ରତୀକ_2, ଯଦି (bit != 0) ଭାଙ୍ଗ, ଯଦି (bit == 0) ଭାଙ୍ଗ)
        }
        ଯେତେବେଳେ (ପ୍ରତୀକ_2 < 0x100);
      }
      ଯେତେବେଳେ (ପ୍ରତୀକ_2 < 0x100)
      {
        CProb *probLit = prob + ପ୍ରତୀକ_2;
        RC_GET_BIT(probLit, ପ୍ରତୀକ_2)
      }
      previousByte = (Byte)ପ୍ରତୀକ_2;

      outStream[nowPos++] = previousByte;
      ଯଦି (state < 4) state = 0;
      ଅନ୍ୟଥା ଯଦି (state < 10) state -= 3;
      ଅନ୍ୟଥା state -= 6;
    }
    ଅନ୍ୟଥା             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_ଅଙ୍କ_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      ଅନ୍ୟଥା
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_ଅଙ୍କ_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            ଯଦି (nowPos == 0)
              ଫେରନ୍ତୁ LZMA_RESULT_DATA_ERROR;
            
            state = state < k_ଅଙ୍କ_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            ଜାରି;
          }
          ଅନ୍ୟଥା
          {
            UpdateBit1(prob);
          }
        }
        ଅନ୍ୟଥା
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          ଅନ୍ୟଥା 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            ଅନ୍ୟଥା
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
        state = state < k_ଅଙ୍କ_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_ତଳ + (posState << k_Len_ଅଙ୍କ_ତଳ_Bits);
          offset = 0;
          ଅଙ୍କ_Bits = k_Len_ଅଙ୍କ_ତଳ_Bits;
        }
        ଅନ୍ୟଥା
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_ଅଙ୍କ_Mid_Bits);
            offset = k_Len_ଅଙ୍କ_ତଳ_Symbols;
            ଅଙ୍କ_Bits = k_Len_ଅଙ୍କ_Mid_Bits;
          }
          ଅନ୍ୟଥା
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ଉପର;
            offset = k_Len_ଅଙ୍କ_ତଳ_Symbols + k_Len_ଅଙ୍କ_Mid_Symbols;
            ଅଙ୍କ_Bits = k_Len_ଅଙ୍କ_ଉପର_Bits;
          }
        }
        Range_Decoder_Bit_ବୃକ୍ଷ_Decode(probLen, ଅଙ୍କ_Bits, len);
        len += offset;
      }

      ଯଦି (state < 4)
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା posSlot;
        state += k_ଅଙ୍କ_Lit_States;
        prob = p + PosSlot +
            ((len < k_ଅଙ୍କ_Len_To_Pos_States ? len : k_ଅଙ୍କ_Len_To_Pos_States - 1) << 
            k_ଅଙ୍କ_Pos_Slot_Bits);
        Range_Decoder_Bit_ବୃକ୍ଷ_Decode(prob, k_ଅଙ୍କ_Pos_Slot_Bits, posSlot);
        ଯଦି (posSlot >= kStartPosModelIndex)
        {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          ଯଦି (posSlot < kEndPosModelIndex)
          {
            rep0 <<= ଅଙ୍କ_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          ଅନ୍ୟଥା
          {
            ଅଙ୍କ_Direct_Bits -= k_ଅଙ୍କ_Align_Bits;
            କର
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              ଯଦି (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            ଯେତେବେଳେ (--ଅଙ୍କ_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_ଅଙ୍କ_Align_Bits;
            ଅଙ୍କ_Direct_Bits = k_ଅଙ୍କ_Align_Bits;
          }
          {
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 1;
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା mi = 1;
            କର
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            ଯେତେବେଳେ(--ଅଙ୍କ_Direct_Bits != 0);
          }
        }
        ଅନ୍ୟଥା
          rep0 = posSlot;
        ଯଦି (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          ଭାଙ୍ଗ;
        }
      }

      len += kMatchMinLen;
      ଯଦି (rep0 > nowPos)
        ଫେରନ୍ତୁ LZMA_RESULT_DATA_ERROR;


      କର
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      ଯେତେବେଳେ(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  ଫେରନ୍ତୁ LZMA_RESULT_OK;
}
