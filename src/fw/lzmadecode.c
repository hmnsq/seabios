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

#وصف_ڪريو k_انگ_چوٽي_بٽ 24
#وصف_ڪريو k_چوٽي_قدر ((UInt32)1 << k_انگ_چوٽي_بٽ)

#وصف_ڪريو k_انگ_ٻٽو_انگ_Model_ڪل_بٽ 11
#وصف_ڪريو k_ٻٽو_انگ_Model_ڪل (1 << k_انگ_ٻٽو_انگ_Model_ڪل_بٽ)
#وصف_ڪريو k_انگ_Move_بٽ 5

#وصف_ڪريو RC_READ_BYTE (*بفر_2++)

#وصف_ڪريو RC_INIT2 ڪوڊ_2 = 0; Range = 0xFFFFFFFF; \
  { صحيح i; لكل(i = 0; i < 5; i++) { RC_TEST; ڪوڊ_2 = (ڪوڊ_2 << 8) | RC_READ_BYTE; }}


#وصف_ڪريو RC_TEST { إذا (بفر_2 == بفر_Lim) أرجع LZMA_نتيجو_معلومات_ERROR; }

#وصف_ڪريو RC_INIT(بفر, بفر_سائيز) بفر_2 = بفر; بفر_Lim = بفر + بفر_سائيز; RC_INIT2
 

#وصف_ڪريو RC_NORMALIZE إذا (Range < k_چوٽي_قدر) { RC_TEST; Range <<= 8; ڪوڊ_2 = (ڪوڊ_2 << 8) | RC_READ_BYTE; }

#وصف_ڪريو IfBit0(p) RC_NORMALIZE; bound = (Range >> k_انگ_ٻٽو_انگ_Model_ڪل_بٽ) * *(p); إذا (ڪوڊ_2 < bound)
#وصف_ڪريو UpdateBit0(p) Range = bound; *(p) += (k_ٻٽو_انگ_Model_ڪل - *(p)) >> k_انگ_Move_بٽ;
#وصف_ڪريو UpdateBit1(p) Range -= bound; ڪوڊ_2 -= bound; *(p) -= (*(p)) >> k_انگ_Move_بٽ;

#وصف_ڪريو RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } وإلا \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#وصف_ڪريو RC_GET_ٻٽو_انگ(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#وصف_ڪريو Range_Decoder_ٻٽو_انگ_وڻ_Decode(probs, انگ_سطحون, res) \
  { صحيح i = انگ_سطحون; res = 1; \
  افعل { CProb *cp = probs + res; RC_GET_ٻٽو_انگ(cp, res) } طالما(--i != 0); \
  res -= (1 << انگ_سطحون); }


#وصف_ڪريو k_انگ_Pos_بٽ_Max 4
#وصف_ڪريو k_انگ_Pos_حالتون_Max (1 << k_انگ_Pos_بٽ_Max)

#وصف_ڪريو k_Len_انگ_هيٺيون_بٽ 3
#وصف_ڪريو k_Len_انگ_هيٺيون_Symbols (1 << k_Len_انگ_هيٺيون_بٽ)
#وصف_ڪريو k_Len_انگ_Mid_بٽ 3
#وصف_ڪريو k_Len_انگ_Mid_Symbols (1 << k_Len_انگ_Mid_بٽ)
#وصف_ڪريو k_Len_انگ_مٿاهون_بٽ 8
#وصف_ڪريو k_Len_انگ_مٿاهون_Symbols (1 << k_Len_انگ_مٿاهون_بٽ)

#وصف_ڪريو LenChoice 0
#وصف_ڪريو LenChoice2 (LenChoice + 1)
#وصف_ڪريو Len_هيٺيون (LenChoice2 + 1)
#وصف_ڪريو LenMid (Len_هيٺيون + (k_انگ_Pos_حالتون_Max << k_Len_انگ_هيٺيون_بٽ))
#وصف_ڪريو Len_مٿاهون (LenMid + (k_انگ_Pos_حالتون_Max << k_Len_انگ_Mid_بٽ))
#وصف_ڪريو k_انگ_Len_Probs (Len_مٿاهون + k_Len_انگ_مٿاهون_Symbols) 


#وصف_ڪريو k_انگ_حالتون 12
#وصف_ڪريو k_انگ_Lit_حالتون 7

#وصف_ڪريو k_شروعات_Pos_Model_اشاريو 4
#وصف_ڪريو k_پڇاڙي_Pos_Model_اشاريو 14
#وصف_ڪريو k_انگ_ڀريل_Distances (1 << (k_پڇاڙي_Pos_Model_اشاريو >> 1))

#وصف_ڪريو k_انگ_Pos_جڳهه_بٽ 6
#وصف_ڪريو k_انگ_Len_منزل_Pos_حالتون 4

#وصف_ڪريو k_انگ_Align_بٽ 4
#وصف_ڪريو k_Align_جدول_سائيز (1 << k_انگ_Align_بٽ)

#وصف_ڪريو k_ميلاپ_Min_Len 2

#وصف_ڪريو Is_ميلاپ 0
#وصف_ڪريو IsRep (Is_ميلاپ + (k_انگ_حالتون << k_انگ_Pos_بٽ_Max))
#وصف_ڪريو IsRepG0 (IsRep + k_انگ_حالتون)
#وصف_ڪريو IsRepG1 (IsRepG0 + k_انگ_حالتون)
#وصف_ڪريو IsRepG2 (IsRepG1 + k_انگ_حالتون)
#وصف_ڪريو IsRep0Long (IsRepG2 + k_انگ_حالتون)
#وصف_ڪريو Pos_جڳهه (IsRep0Long + (k_انگ_حالتون << k_انگ_Pos_بٽ_Max))
#وصف_ڪريو SpecPos (Pos_جڳهه + (k_انگ_Len_منزل_Pos_حالتون << k_انگ_Pos_جڳهه_بٽ))
#وصف_ڪريو Align (SpecPos + k_انگ_ڀريل_Distances - k_پڇاڙي_Pos_Model_اشاريو)
#وصف_ڪريو LenCoder (Align + k_Align_جدول_سائيز)
#وصف_ڪريو RepLenCoder (LenCoder + k_انگ_Len_Probs)
#وصف_ڪريو Literal (RepLenCoder + k_انگ_Len_Probs)

#إذا Literal != LZMA_بنياد_سائيز
StopCompilingDueBUG
#جيڪڏهن_پورو

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بنانشان حرف *props_معلومات, صحيح سائيز)
{
  بنانشان حرف prop0;
  إذا (سائيز < LZMA_PROPERTIES_سائيز)
    أرجع LZMA_نتيجو_معلومات_ERROR;
  prop0 = props_معلومات[0];
  إذا (prop0 >= (9 * 5 * 5))
    أرجع LZMA_نتيجو_معلومات_ERROR;
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

  أرجع LZMA_نتيجو_OK;
}

#وصف_ڪريو k_Lzma_وهڪرو_Was_ختم_Id (-1)

صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بنانشان حرف *اندريون_وهڪرو, سائيز_T اندريون_سائيز, سائيز_T *اندريون_سائيز_Processed,
    بنانشان حرف *ٻاهريون_وهڪرو, سائيز_T ٻاهريون_سائيز, سائيز_T *ٻاهريون_سائيز_Processed)
{
  CProb *p = vs->Probs;
  سائيز_T nowPos = 0;
  Byte اڳيون_Byte = 0;
  UInt32 pos_State_نقاب = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_نقاب = (1 << (vs->Properties.lp)) - 1;
  صحيح lc = vs->Properties.lc;


  صحيح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحيح len = 0;
  ثابت Byte *بفر_2;
  ثابت Byte *بفر_Lim;
  UInt32 Range;
  UInt32 ڪوڊ_2;

  *اندريون_سائيز_Processed = 0;
  *ٻاهريون_سائيز_Processed = 0;

  {
    UInt32 i;
    UInt32 انگ_Probs = Literal + ((UInt32)LZMA_LIT_سائيز << (lc + vs->Properties.lp));
    لكل (i = 0; i < انگ_Probs; i++)
      p[i] = k_ٻٽو_انگ_Model_ڪل >> 1;
  }
  
  RC_INIT(اندريون_وهڪرو, اندريون_سائيز);


  طالما(nowPos < ٻاهريون_سائيز)
  {
    CProb *prob;
    UInt32 bound;
    صحيح posState = (صحيح)(
        (nowPos 
        )
        & pos_State_نقاب);

    prob = p + Is_ميلاپ + (state << k_انگ_Pos_بٽ_Max) + posState;
    IfBit0(prob)
    {
      صحيح نشان_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_سائيز * 
        (((
        (nowPos 
        )
        & literal_Pos_نقاب) << lc) + (اڳيون_Byte >> (8 - lc))));

      إذا (state >= k_انگ_Lit_حالتون)
      {
        صحيح ميلاپ_Byte;
        ميلاپ_Byte = ٻاهريون_وهڪرو[nowPos - rep0];
        افعل
        {
          صحيح ٻٽو_انگ;
          CProb *probLit;
          ميلاپ_Byte <<= 1;
          ٻٽو_انگ = (ميلاپ_Byte & 0x100);
          probLit = prob + 0x100 + ٻٽو_انگ + نشان_2;
          RC_GET_BIT2(probLit, نشان_2, إذا (ٻٽو_انگ != 0) اكسر, إذا (ٻٽو_انگ == 0) اكسر)
        }
        طالما (نشان_2 < 0x100);
      }
      طالما (نشان_2 < 0x100)
      {
        CProb *probLit = prob + نشان_2;
        RC_GET_ٻٽو_انگ(probLit, نشان_2)
      }
      اڳيون_Byte = (Byte)نشان_2;

      ٻاهريون_وهڪرو[nowPos++] = اڳيون_Byte;
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
        state = state < k_انگ_Lit_حالتون ? 0 : 3;
        prob = p + LenCoder;
      }
      وإلا
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_انگ_Pos_بٽ_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            إذا (nowPos == 0)
              أرجع LZMA_نتيجو_معلومات_ERROR;
            
            state = state < k_انگ_Lit_حالتون ? 9 : 11;
            اڳيون_Byte = ٻاهريون_وهڪرو[nowPos - rep0];
            ٻاهريون_وهڪرو[nowPos++] = اڳيون_Byte;

            استمر;
          }
          وإلا
          {
            UpdateBit1(prob);
          }
        }
        وإلا
        {
          UInt32 مفاصلو;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            مفاصلو = rep1;
          }
          وإلا 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              مفاصلو = rep2;
            }
            وإلا
            {
              UpdateBit1(prob);
              مفاصلو = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = مفاصلو;
        }
        state = state < k_انگ_Lit_حالتون ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحيح انگ_بٽ, وٿي_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_هيٺيون + (posState << k_Len_انگ_هيٺيون_بٽ);
          وٿي_2 = 0;
          انگ_بٽ = k_Len_انگ_هيٺيون_بٽ;
        }
        وإلا
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_انگ_Mid_بٽ);
            وٿي_2 = k_Len_انگ_هيٺيون_Symbols;
            انگ_بٽ = k_Len_انگ_Mid_بٽ;
          }
          وإلا
          {
            UpdateBit1(probLen);
            probLen = prob + Len_مٿاهون;
            وٿي_2 = k_Len_انگ_هيٺيون_Symbols + k_Len_انگ_Mid_Symbols;
            انگ_بٽ = k_Len_انگ_مٿاهون_بٽ;
          }
        }
        Range_Decoder_ٻٽو_انگ_وڻ_Decode(probLen, انگ_بٽ, len);
        len += وٿي_2;
      }

      إذا (state < 4)
      {
        صحيح pos_جڳهه;
        state += k_انگ_Lit_حالتون;
        prob = p + Pos_جڳهه +
            ((len < k_انگ_Len_منزل_Pos_حالتون ? len : k_انگ_Len_منزل_Pos_حالتون - 1) << 
            k_انگ_Pos_جڳهه_بٽ);
        Range_Decoder_ٻٽو_انگ_وڻ_Decode(prob, k_انگ_Pos_جڳهه_بٽ, pos_جڳهه);
        إذا (pos_جڳهه >= k_شروعات_Pos_Model_اشاريو)
        {
          صحيح انگ_سڌو_بٽ = ((pos_جڳهه >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_جڳهه & 1));
          إذا (pos_جڳهه < k_پڇاڙي_Pos_Model_اشاريو)
          {
            rep0 <<= انگ_سڌو_بٽ;
            prob = p + SpecPos + rep0 - pos_جڳهه - 1;
          }
          وإلا
          {
            انگ_سڌو_بٽ -= k_انگ_Align_بٽ;
            افعل
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              إذا (ڪوڊ_2 >= Range)
              {
                ڪوڊ_2 -= Range;
                rep0 |= 1;
              }
            }
            طالما (--انگ_سڌو_بٽ != 0);
            prob = p + Align;
            rep0 <<= k_انگ_Align_بٽ;
            انگ_سڌو_بٽ = k_انگ_Align_بٽ;
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
            طالما(--انگ_سڌو_بٽ != 0);
          }
        }
        وإلا
          rep0 = pos_جڳهه;
        إذا (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_وهڪرو_Was_ختم_Id;
          اكسر;
        }
      }

      len += k_ميلاپ_Min_Len;
      إذا (rep0 > nowPos)
        أرجع LZMA_نتيجو_معلومات_ERROR;


      افعل
      {
        اڳيون_Byte = ٻاهريون_وهڪرو[nowPos - rep0];
        len--;
        ٻاهريون_وهڪرو[nowPos++] = اڳيون_Byte;
      }
      طالما(len != 0 && nowPos < ٻاهريون_سائيز);
    }
  }
  RC_NORMALIZE;


  *اندريون_سائيز_Processed = (سائيز_T)(بفر_2 - اندريون_وهڪرو);
  *ٻاهريون_سائيز_Processed = nowPos;
  أرجع LZMA_نتيجو_OK;
}
