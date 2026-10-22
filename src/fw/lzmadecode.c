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

#ضمن "lzmadecode.h"

#define k_ژمارە_لووتکە_Bits 24
#define k_لووتکە_بەها ((UInt32)1 << k_ژمارە_لووتکە_Bits)

#define k_ژمارە_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_ژمارە_Bit_Model_Total_Bits)
#define k_ژمارە_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { صحيح i; لكل(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { إذا (Buffer == BufferLim) أرجع LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE إذا (Range < k_لووتکە_بەها) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_ژمارە_Bit_Model_Total_Bits) * *(p); إذا (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_ژمارە_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_ژمارە_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } وإلا \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_درەخت_Decode(probs, ژمارە_Levels, res) \
  { صحيح i = ژمارە_Levels; res = 1; \
  افعل { CProb *cp = probs + res; RC_GET_BIT(cp, res) } طالما(--i != 0); \
  res -= (1 << ژمارە_Levels); }


#define k_ژمارە_Pos_Bits_Max 4
#define k_ژمارە_Pos_States_Max (1 << k_ژمارە_Pos_Bits_Max)

#define k_Len_ژمارە_نزم_Bits 3
#define k_Len_ژمارە_نزم_Symbols (1 << k_Len_ژمارە_نزم_Bits)
#define k_Len_ژمارە_Mid_Bits 3
#define k_Len_ژمارە_Mid_Symbols (1 << k_Len_ژمارە_Mid_Bits)
#define k_Len_ژمارە_بەرز_Bits 8
#define k_Len_ژمارە_بەرز_Symbols (1 << k_Len_ژمارە_بەرز_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_نزم (LenChoice2 + 1)
#define LenMid (Len_نزم + (k_ژمارە_Pos_States_Max << k_Len_ژمارە_نزم_Bits))
#define Len_بەرز (LenMid + (k_ژمارە_Pos_States_Max << k_Len_ژمارە_Mid_Bits))
#define k_ژمارە_Len_Probs (Len_بەرز + k_Len_ژمارە_بەرز_Symbols) 


#define k_ژمارە_States 12
#define k_ژمارە_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_ژمارە_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_ژمارە_Pos_Slot_Bits 6
#define k_ژمارە_Len_To_Pos_States 4

#define k_ژمارە_Align_Bits 4
#define kAlignTableSize (1 << k_ژمارە_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_ژمارە_States << k_ژمارە_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_ژمارە_States)
#define IsRepG1 (IsRepG0 + k_ژمارە_States)
#define IsRepG2 (IsRepG1 + k_ژمارە_States)
#define IsRep0Long (IsRepG2 + k_ژمارە_States)
#define PosSlot (IsRep0Long + (k_ژمارە_States << k_ژمارە_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_ژمارە_Len_To_Pos_States << k_ژمارە_Pos_Slot_Bits))
#define Align (SpecPos + k_ژمارە_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_ژمارە_Len_Probs)
#define Literal (RepLenCoder + k_ژمارە_Len_Probs)

#إذا Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت unsigned حرف *propsData, صحيح size)
{
  unsigned حرف prop0;
  إذا (size < LZMA_PROPERTIES_SIZE)
    أرجع LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  إذا (prop0 >= (9 * 5 * 5))
    أرجع LZMA_RESULT_DATA_ERROR;
  {
    لكل (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    لكل (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  أرجع LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت unsigned حرف *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned حرف *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  صحيح lc = vs->Properties.lc;


  صحيح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحيح len = 0;
  ثابت Byte *Buffer;
  ثابت Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 ژمارە_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    لكل (i = 0; i < ژمارە_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  طالما(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    صحيح posState = (صحيح)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_ژمارە_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      صحيح هێما_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      إذا (state >= k_ژمارە_Lit_States)
      {
        صحيح matchByte;
        matchByte = outStream[nowPos - rep0];
        افعل
        {
          صحيح bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + هێما_2;
          RC_GET_BIT2(probLit, هێما_2, إذا (bit != 0) اكسر, إذا (bit == 0) اكسر)
        }
        طالما (هێما_2 < 0x100);
      }
      طالما (هێما_2 < 0x100)
      {
        CProb *probLit = prob + هێما_2;
        RC_GET_BIT(probLit, هێما_2)
      }
      previousByte = (Byte)هێما_2;

      outStream[nowPos++] = previousByte;
      إذا (state < 4) state = 0;
      وإلا إذا (state < 10) state -= 3;
      وإلا state -= 6;
    }
    وإلا             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_ژمارە_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      وإلا
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_ژمارە_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            إذا (nowPos == 0)
              أرجع LZMA_RESULT_DATA_ERROR;
            
            state = state < k_ژمارە_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            استمر;
          }
          وإلا
          {
            UpdateBit1(prob);
          }
        }
        وإلا
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          وإلا 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            وإلا
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
        state = state < k_ژمارە_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحيح ژمارە_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_نزم + (posState << k_Len_ژمارە_نزم_Bits);
          offset = 0;
          ژمارە_Bits = k_Len_ژمارە_نزم_Bits;
        }
        وإلا
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_ژمارە_Mid_Bits);
            offset = k_Len_ژمارە_نزم_Symbols;
            ژمارە_Bits = k_Len_ژمارە_Mid_Bits;
          }
          وإلا
          {
            UpdateBit1(probLen);
            probLen = prob + Len_بەرز;
            offset = k_Len_ژمارە_نزم_Symbols + k_Len_ژمارە_Mid_Symbols;
            ژمارە_Bits = k_Len_ژمارە_بەرز_Bits;
          }
        }
        Range_Decoder_Bit_درەخت_Decode(probLen, ژمارە_Bits, len);
        len += offset;
      }

      إذا (state < 4)
      {
        صحيح posSlot;
        state += k_ژمارە_Lit_States;
        prob = p + PosSlot +
            ((len < k_ژمارە_Len_To_Pos_States ? len : k_ژمارە_Len_To_Pos_States - 1) << 
            k_ژمارە_Pos_Slot_Bits);
        Range_Decoder_Bit_درەخت_Decode(prob, k_ژمارە_Pos_Slot_Bits, posSlot);
        إذا (posSlot >= kStartPosModelIndex)
        {
          صحيح ژمارە_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          إذا (posSlot < kEndPosModelIndex)
          {
            rep0 <<= ژمارە_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          وإلا
          {
            ژمارە_Direct_Bits -= k_ژمارە_Align_Bits;
            افعل
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              إذا (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            طالما (--ژمارە_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_ژمارە_Align_Bits;
            ژمارە_Direct_Bits = k_ژمارە_Align_Bits;
          }
          {
            صحيح i = 1;
            صحيح mi = 1;
            افعل
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            طالما(--ژمارە_Direct_Bits != 0);
          }
        }
        وإلا
          rep0 = posSlot;
        إذا (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          اكسر;
        }
      }

      len += kMatchMinLen;
      إذا (rep0 > nowPos)
        أرجع LZMA_RESULT_DATA_ERROR;


      افعل
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      طالما(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  أرجع LZMA_RESULT_OK;
}
