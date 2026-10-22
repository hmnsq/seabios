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

#شامل "lzmadecode.h"

#define k_رقم_بالاترین_Bits 24
#define k_بالاترین_مقدار ((UInt32)1 << k_رقم_بالاترین_Bits)

#define k_رقم_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_رقم_Bit_Model_Total_Bits)
#define k_رقم_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { صحیح i; برای(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { اگر (Buffer == BufferLim) بازگشت LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE اگر (Range < k_بالاترین_مقدار) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_رقم_Bit_Model_Total_Bits) * *(p); اگر (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_رقم_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_رقم_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } وگرنه \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_درخت_Decode(probs, رقم_Levels, res) \
  { صحیح i = رقم_Levels; res = 1; \
  انجامبده { CProb *cp = probs + res; RC_GET_BIT(cp, res) } وقتی(--i != 0); \
  res -= (1 << رقم_Levels); }


#define k_رقم_Pos_Bits_Max 4
#define k_رقم_Pos_States_Max (1 << k_رقم_Pos_Bits_Max)

#define k_Len_رقم_کم_Bits 3
#define k_Len_رقم_کم_Symbols (1 << k_Len_رقم_کم_Bits)
#define k_Len_رقم_Mid_Bits 3
#define k_Len_رقم_Mid_Symbols (1 << k_Len_رقم_Mid_Bits)
#define k_Len_رقم_بالا_Bits 8
#define k_Len_رقم_بالا_Symbols (1 << k_Len_رقم_بالا_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_کم (LenChoice2 + 1)
#define LenMid (Len_کم + (k_رقم_Pos_States_Max << k_Len_رقم_کم_Bits))
#define Len_بالا (LenMid + (k_رقم_Pos_States_Max << k_Len_رقم_Mid_Bits))
#define k_رقم_Len_Probs (Len_بالا + k_Len_رقم_بالا_Symbols) 


#define k_رقم_States 12
#define k_رقم_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_رقم_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_رقم_Pos_Slot_Bits 6
#define k_رقم_Len_To_Pos_States 4

#define k_رقم_Align_Bits 4
#define kAlignTableSize (1 << k_رقم_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_رقم_States << k_رقم_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_رقم_States)
#define IsRepG1 (IsRepG0 + k_رقم_States)
#define IsRepG2 (IsRepG1 + k_رقم_States)
#define IsRep0Long (IsRepG2 + k_رقم_States)
#define PosSlot (IsRep0Long + (k_رقم_States << k_رقم_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_رقم_Len_To_Pos_States << k_رقم_Pos_Slot_Bits))
#define Align (SpecPos + k_رقم_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_رقم_Len_Probs)
#define Literal (RepLenCoder + k_رقم_Len_Probs)

#اگر Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

صحیح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت unsigned نویسه *propsData, صحیح size)
{
  unsigned نویسه prop0;
  اگر (size < LZMA_PROPERTIES_SIZE)
    بازگشت LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  اگر (prop0 >= (9 * 5 * 5))
    بازگشت LZMA_RESULT_DATA_ERROR;
  {
    برای (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    برای (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  بازگشت LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

صحیح LzmaDecode(CLzmaDecoderState *vs,
    ثابت unsigned نویسه *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned نویسه *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  صحیح lc = vs->Properties.lc;


  صحیح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحیح len = 0;
  ثابت Byte *Buffer;
  ثابت Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 رقم_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    برای (i = 0; i < رقم_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  وقتی(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    صحیح posState = (صحیح)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_رقم_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      صحیح نماد_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      اگر (state >= k_رقم_Lit_States)
      {
        صحیح matchByte;
        matchByte = outStream[nowPos - rep0];
        انجامبده
        {
          صحیح bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + نماد_2;
          RC_GET_BIT2(probLit, نماد_2, اگر (bit != 0) بشکن, اگر (bit == 0) بشکن)
        }
        وقتی (نماد_2 < 0x100);
      }
      وقتی (نماد_2 < 0x100)
      {
        CProb *probLit = prob + نماد_2;
        RC_GET_BIT(probLit, نماد_2)
      }
      previousByte = (Byte)نماد_2;

      outStream[nowPos++] = previousByte;
      اگر (state < 4) state = 0;
      وگرنه اگر (state < 10) state -= 3;
      وگرنه state -= 6;
    }
    وگرنه             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_رقم_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      وگرنه
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_رقم_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            اگر (nowPos == 0)
              بازگشت LZMA_RESULT_DATA_ERROR;
            
            state = state < k_رقم_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            ادامه;
          }
          وگرنه
          {
            UpdateBit1(prob);
          }
        }
        وگرنه
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          وگرنه 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            وگرنه
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
        state = state < k_رقم_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحیح رقم_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_کم + (posState << k_Len_رقم_کم_Bits);
          offset = 0;
          رقم_Bits = k_Len_رقم_کم_Bits;
        }
        وگرنه
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_رقم_Mid_Bits);
            offset = k_Len_رقم_کم_Symbols;
            رقم_Bits = k_Len_رقم_Mid_Bits;
          }
          وگرنه
          {
            UpdateBit1(probLen);
            probLen = prob + Len_بالا;
            offset = k_Len_رقم_کم_Symbols + k_Len_رقم_Mid_Symbols;
            رقم_Bits = k_Len_رقم_بالا_Bits;
          }
        }
        Range_Decoder_Bit_درخت_Decode(probLen, رقم_Bits, len);
        len += offset;
      }

      اگر (state < 4)
      {
        صحیح posSlot;
        state += k_رقم_Lit_States;
        prob = p + PosSlot +
            ((len < k_رقم_Len_To_Pos_States ? len : k_رقم_Len_To_Pos_States - 1) << 
            k_رقم_Pos_Slot_Bits);
        Range_Decoder_Bit_درخت_Decode(prob, k_رقم_Pos_Slot_Bits, posSlot);
        اگر (posSlot >= kStartPosModelIndex)
        {
          صحیح رقم_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          اگر (posSlot < kEndPosModelIndex)
          {
            rep0 <<= رقم_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          وگرنه
          {
            رقم_Direct_Bits -= k_رقم_Align_Bits;
            انجامبده
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              اگر (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            وقتی (--رقم_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_رقم_Align_Bits;
            رقم_Direct_Bits = k_رقم_Align_Bits;
          }
          {
            صحیح i = 1;
            صحیح mi = 1;
            انجامبده
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            وقتی(--رقم_Direct_Bits != 0);
          }
        }
        وگرنه
          rep0 = posSlot;
        اگر (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          بشکن;
        }
      }

      len += kMatchMinLen;
      اگر (rep0 > nowPos)
        بازگشت LZMA_RESULT_DATA_ERROR;


      انجامبده
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      وقتی(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  بازگشت LZMA_RESULT_OK;
}
