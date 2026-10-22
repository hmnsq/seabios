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

#অন্তর্ভাব "lzmadecode.h"

#define k_অঙ্ক_শীর্ষ_Bits 24
#define k_শীর্ষ_মান ((UInt32)1 << k_অঙ্ক_শীর্ষ_Bits)

#define k_অঙ্ক_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_অঙ্ক_Bit_Model_Total_Bits)
#define k_অঙ্ক_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { পূর্ণাঙ্ক i; প্রতি(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { যদি (Buffer == BufferLim) প্রত্যাগম LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE যদি (Range < k_শীর্ষ_মান) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_অঙ্ক_Bit_Model_Total_Bits) * *(p); যদি (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_অঙ্ক_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_অঙ্ক_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } অন্যথা \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_বৃক্ষ_Decode(probs, অঙ্ক_Levels, res) \
  { পূর্ণাঙ্ক i = অঙ্ক_Levels; res = 1; \
  কুরু { CProb *cp = probs + res; RC_GET_BIT(cp, res) } যাবত্(--i != 0); \
  res -= (1 << অঙ্ক_Levels); }


#define k_অঙ্ক_Pos_Bits_Max 4
#define k_অঙ্ক_Pos_States_Max (1 << k_অঙ্ক_Pos_Bits_Max)

#define k_Len_অঙ্ক_নিচের_Bits 3
#define k_Len_অঙ্ক_নিচের_Symbols (1 << k_Len_অঙ্ক_নিচের_Bits)
#define k_Len_অঙ্ক_Mid_Bits 3
#define k_Len_অঙ্ক_Mid_Symbols (1 << k_Len_অঙ্ক_Mid_Bits)
#define k_Len_অঙ্ক_উপরের_Bits 8
#define k_Len_অঙ্ক_উপরের_Symbols (1 << k_Len_অঙ্ক_উপরের_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_নিচের (LenChoice2 + 1)
#define LenMid (Len_নিচের + (k_অঙ্ক_Pos_States_Max << k_Len_অঙ্ক_নিচের_Bits))
#define Len_উপরের (LenMid + (k_অঙ্ক_Pos_States_Max << k_Len_অঙ্ক_Mid_Bits))
#define k_অঙ্ক_Len_Probs (Len_উপরের + k_Len_অঙ্ক_উপরের_Symbols) 


#define k_অঙ্ক_States 12
#define k_অঙ্ক_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_অঙ্ক_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_অঙ্ক_Pos_Slot_Bits 6
#define k_অঙ্ক_Len_To_Pos_States 4

#define k_অঙ্ক_Align_Bits 4
#define kAlignTableSize (1 << k_অঙ্ক_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_অঙ্ক_States << k_অঙ্ক_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_অঙ্ক_States)
#define IsRepG1 (IsRepG0 + k_অঙ্ক_States)
#define IsRepG2 (IsRepG1 + k_অঙ্ক_States)
#define IsRep0Long (IsRepG2 + k_অঙ্ক_States)
#define PosSlot (IsRep0Long + (k_অঙ্ক_States << k_অঙ্ক_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_অঙ্ক_Len_To_Pos_States << k_অঙ্ক_Pos_Slot_Bits))
#define Align (SpecPos + k_অঙ্ক_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_অঙ্ক_Len_Probs)
#define Literal (RepLenCoder + k_অঙ্ক_Len_Probs)

#যদি Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

পূর্ণাঙ্ক LzmaDecodeProperties(CLzmaProperties *propsRes, ধ্রুব unsigned বর্ণ *propsData, পূর্ণাঙ্ক size)
{
  unsigned বর্ণ prop0;
  যদি (size < LZMA_PROPERTIES_SIZE)
    প্রত্যাগম LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  যদি (prop0 >= (9 * 5 * 5))
    প্রত্যাগম LZMA_RESULT_DATA_ERROR;
  {
    প্রতি (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    প্রতি (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  প্রত্যাগম LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

পূর্ণাঙ্ক LzmaDecode(CLzmaDecoderState *vs,
    ধ্রুব unsigned বর্ণ *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned বর্ণ *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  পূর্ণাঙ্ক lc = vs->Properties.lc;


  পূর্ণাঙ্ক state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  পূর্ণাঙ্ক len = 0;
  ধ্রুব Byte *Buffer;
  ধ্রুব Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 অঙ্ক_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    প্রতি (i = 0; i < অঙ্ক_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  যাবত্(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    পূর্ণাঙ্ক posState = (পূর্ণাঙ্ক)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_অঙ্ক_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      পূর্ণাঙ্ক প্রতীক_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      যদি (state >= k_অঙ্ক_Lit_States)
      {
        পূর্ণাঙ্ক matchByte;
        matchByte = outStream[nowPos - rep0];
        কুরু
        {
          পূর্ণাঙ্ক bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + প্রতীক_2;
          RC_GET_BIT2(probLit, প্রতীক_2, যদি (bit != 0) ভঙ্গ, যদি (bit == 0) ভঙ্গ)
        }
        যাবত্ (প্রতীক_2 < 0x100);
      }
      যাবত্ (প্রতীক_2 < 0x100)
      {
        CProb *probLit = prob + প্রতীক_2;
        RC_GET_BIT(probLit, প্রতীক_2)
      }
      previousByte = (Byte)প্রতীক_2;

      outStream[nowPos++] = previousByte;
      যদি (state < 4) state = 0;
      অন্যথা যদি (state < 10) state -= 3;
      অন্যথা state -= 6;
    }
    অন্যথা             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_অঙ্ক_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      অন্যথা
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_অঙ্ক_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            যদি (nowPos == 0)
              প্রত্যাগম LZMA_RESULT_DATA_ERROR;
            
            state = state < k_অঙ্ক_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            অনুবর্ত;
          }
          অন্যথা
          {
            UpdateBit1(prob);
          }
        }
        অন্যথা
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          অন্যথা 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            অন্যথা
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
        state = state < k_অঙ্ক_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        পূর্ণাঙ্ক অঙ্ক_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_নিচের + (posState << k_Len_অঙ্ক_নিচের_Bits);
          offset = 0;
          অঙ্ক_Bits = k_Len_অঙ্ক_নিচের_Bits;
        }
        অন্যথা
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_অঙ্ক_Mid_Bits);
            offset = k_Len_অঙ্ক_নিচের_Symbols;
            অঙ্ক_Bits = k_Len_অঙ্ক_Mid_Bits;
          }
          অন্যথা
          {
            UpdateBit1(probLen);
            probLen = prob + Len_উপরের;
            offset = k_Len_অঙ্ক_নিচের_Symbols + k_Len_অঙ্ক_Mid_Symbols;
            অঙ্ক_Bits = k_Len_অঙ্ক_উপরের_Bits;
          }
        }
        Range_Decoder_Bit_বৃক্ষ_Decode(probLen, অঙ্ক_Bits, len);
        len += offset;
      }

      যদি (state < 4)
      {
        পূর্ণাঙ্ক posSlot;
        state += k_অঙ্ক_Lit_States;
        prob = p + PosSlot +
            ((len < k_অঙ্ক_Len_To_Pos_States ? len : k_অঙ্ক_Len_To_Pos_States - 1) << 
            k_অঙ্ক_Pos_Slot_Bits);
        Range_Decoder_Bit_বৃক্ষ_Decode(prob, k_অঙ্ক_Pos_Slot_Bits, posSlot);
        যদি (posSlot >= kStartPosModelIndex)
        {
          পূর্ণাঙ্ক অঙ্ক_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          যদি (posSlot < kEndPosModelIndex)
          {
            rep0 <<= অঙ্ক_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          অন্যথা
          {
            অঙ্ক_Direct_Bits -= k_অঙ্ক_Align_Bits;
            কুরু
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              যদি (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            যাবত্ (--অঙ্ক_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_অঙ্ক_Align_Bits;
            অঙ্ক_Direct_Bits = k_অঙ্ক_Align_Bits;
          }
          {
            পূর্ণাঙ্ক i = 1;
            পূর্ণাঙ্ক mi = 1;
            কুরু
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            যাবত্(--অঙ্ক_Direct_Bits != 0);
          }
        }
        অন্যথা
          rep0 = posSlot;
        যদি (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          ভঙ্গ;
        }
      }

      len += kMatchMinLen;
      যদি (rep0 > nowPos)
        প্রত্যাগম LZMA_RESULT_DATA_ERROR;


      কুরু
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      যাবত্(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  প্রত্যাগম LZMA_RESULT_OK;
}
