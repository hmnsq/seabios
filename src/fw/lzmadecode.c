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

#define k_অঙ্ক_Bit_Model_যোগ_Bits 11
#define k_Bit_Model_যোগ (1 << k_অঙ্ক_Bit_Model_যোগ_Bits)
#define k_অঙ্ক_Move_Bits 5

#define RC_READ_BYTE (*বাফার_2++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { পূর্ণসংখ্যা i; জন্য(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { যদি (বাফার_2 == বাফার_Lim) ফেরত LZMA_ফলাফল_তথ্য_ERROR; }

#define RC_INIT(বাফার, বাফার_আকার) বাফার_2 = বাফার; বাফার_Lim = বাফার + বাফার_আকার; RC_INIT2
 

#define RC_NORMALIZE যদি (Range < k_শীর্ষ_মান) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_অঙ্ক_Bit_Model_যোগ_Bits) * *(p); যদি (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (k_Bit_Model_যোগ - *(p)) >> k_অঙ্ক_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_অঙ্ক_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } অন্যথা \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_বৃক্ষ_Decode(probs, অঙ্ক_Levels, res) \
  { পূর্ণসংখ্যা i = অঙ্ক_Levels; res = 1; \
  করো { CProb *cp = probs + res; RC_GET_BIT(cp, res) } যখন(--i != 0); \
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

#define k_Start_Pos_Model_সূচক 4
#define k_End_Pos_Model_সূচক 14
#define k_অঙ্ক_Full_Distances (1 << (k_End_Pos_Model_সূচক >> 1))

#define k_অঙ্ক_Pos_Slot_Bits 6
#define k_অঙ্ক_Len_To_Pos_States 4

#define k_অঙ্ক_Align_Bits 4
#define k_Align_Table_আকার (1 << k_অঙ্ক_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_অঙ্ক_States << k_অঙ্ক_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_অঙ্ক_States)
#define IsRepG1 (IsRepG0 + k_অঙ্ক_States)
#define IsRepG2 (IsRepG1 + k_অঙ্ক_States)
#define IsRep0Long (IsRepG2 + k_অঙ্ক_States)
#define PosSlot (IsRep0Long + (k_অঙ্ক_States << k_অঙ্ক_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_অঙ্ক_Len_To_Pos_States << k_অঙ্ক_Pos_Slot_Bits))
#define Align (SpecPos + k_অঙ্ক_Full_Distances - k_End_Pos_Model_সূচক)
#define LenCoder (Align + k_Align_Table_আকার)
#define RepLenCoder (LenCoder + k_অঙ্ক_Len_Probs)
#define Literal (RepLenCoder + k_অঙ্ক_Len_Probs)

#যদি Literal != LZMA_BASE_আকার
StopCompilingDueBUG
#endif

পূর্ণসংখ্যা LzmaDecodeProperties(CLzmaProperties *propsRes, ধ্রুবক unsigned অক্ষর *props_তথ্য, পূর্ণসংখ্যা আকার_2)
{
  unsigned অক্ষর prop0;
  যদি (আকার_2 < LZMA_PROPERTIES_আকার)
    ফেরত LZMA_ফলাফল_তথ্য_ERROR;
  prop0 = props_তথ্য[0];
  যদি (prop0 >= (9 * 5 * 5))
    ফেরত LZMA_ফলাফল_তথ্য_ERROR;
  {
    জন্য (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    জন্য (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  ফেরত LZMA_ফলাফল_OK;
}

#define kLzmaStreamWasFinishedId (-1)

পূর্ণসংখ্যা LzmaDecode(CLzmaDecoderState *vs,
    ধ্রুবক unsigned অক্ষর *inStream, আকার_T in_আকার, আকার_T *in_আকার_Processed,
    unsigned অক্ষর *outStream, আকার_T out_আকার, আকার_T *out_আকার_Processed)
{
  CProb *p = vs->Probs;
  আকার_T nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  পূর্ণসংখ্যা lc = vs->Properties.lc;


  পূর্ণসংখ্যা state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  পূর্ণসংখ্যা len = 0;
  ধ্রুবক Byte *বাফার_2;
  ধ্রুবক Byte *বাফার_Lim;
  UInt32 Range;
  UInt32 Code;

  *in_আকার_Processed = 0;
  *out_আকার_Processed = 0;

  {
    UInt32 i;
    UInt32 অঙ্ক_Probs = Literal + ((UInt32)LZMA_LIT_আকার << (lc + vs->Properties.lp));
    জন্য (i = 0; i < অঙ্ক_Probs; i++)
      p[i] = k_Bit_Model_যোগ >> 1;
  }
  
  RC_INIT(inStream, in_আকার);


  যখন(nowPos < out_আকার)
  {
    CProb *prob;
    UInt32 bound;
    পূর্ণসংখ্যা posState = (পূর্ণসংখ্যা)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_অঙ্ক_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      পূর্ণসংখ্যা প্রতীক_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_আকার * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      যদি (state >= k_অঙ্ক_Lit_States)
      {
        পূর্ণসংখ্যা matchByte;
        matchByte = outStream[nowPos - rep0];
        করো
        {
          পূর্ণসংখ্যা bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + প্রতীক_2;
          RC_GET_BIT2(probLit, প্রতীক_2, যদি (bit != 0) বিরতি, যদি (bit == 0) বিরতি)
        }
        যখন (প্রতীক_2 < 0x100);
      }
      যখন (প্রতীক_2 < 0x100)
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
              ফেরত LZMA_ফলাফল_তথ্য_ERROR;
            
            state = state < k_অঙ্ক_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            চালু;
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
        পূর্ণসংখ্যা অঙ্ক_Bits, offset;
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
        পূর্ণসংখ্যা posSlot;
        state += k_অঙ্ক_Lit_States;
        prob = p + PosSlot +
            ((len < k_অঙ্ক_Len_To_Pos_States ? len : k_অঙ্ক_Len_To_Pos_States - 1) << 
            k_অঙ্ক_Pos_Slot_Bits);
        Range_Decoder_Bit_বৃক্ষ_Decode(prob, k_অঙ্ক_Pos_Slot_Bits, posSlot);
        যদি (posSlot >= k_Start_Pos_Model_সূচক)
        {
          পূর্ণসংখ্যা অঙ্ক_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          যদি (posSlot < k_End_Pos_Model_সূচক)
          {
            rep0 <<= অঙ্ক_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          অন্যথা
          {
            অঙ্ক_Direct_Bits -= k_অঙ্ক_Align_Bits;
            করো
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
            যখন (--অঙ্ক_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_অঙ্ক_Align_Bits;
            অঙ্ক_Direct_Bits = k_অঙ্ক_Align_Bits;
          }
          {
            পূর্ণসংখ্যা i = 1;
            পূর্ণসংখ্যা mi = 1;
            করো
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            যখন(--অঙ্ক_Direct_Bits != 0);
          }
        }
        অন্যথা
          rep0 = posSlot;
        যদি (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          বিরতি;
        }
      }

      len += kMatchMinLen;
      যদি (rep0 > nowPos)
        ফেরত LZMA_ফলাফল_তথ্য_ERROR;


      করো
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      যখন(len != 0 && nowPos < out_আকার);
    }
  }
  RC_NORMALIZE;


  *in_আকার_Processed = (আকার_T)(বাফার_2 - inStream);
  *out_আকার_Processed = nowPos;
  ফেরত LZMA_ফলাফল_OK;
}
