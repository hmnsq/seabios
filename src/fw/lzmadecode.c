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

#تعریف_کرو k_عدد_چوٹی_بٹ 24
#تعریف_کرو k_چوٹی_قدر ((UInt32)1 << k_عدد_چوٹی_بٹ)

#تعریف_کرو k_عدد_ثنائی_ہندسہ_Model_کل_بٹ 11
#تعریف_کرو k_ثنائی_ہندسہ_Model_کل (1 << k_عدد_ثنائی_ہندسہ_Model_کل_بٹ)
#تعریف_کرو k_عدد_Move_بٹ 5

#تعریف_کرو RC_READ_BYTE (*بفر_2++)

#تعریف_کرو RC_INIT2 رمز_2 = 0; Range = 0xFFFFFFFF; \
  { صحیح i; برائے(i = 0; i < 5; i++) { RC_TEST; رمز_2 = (رمز_2 << 8) | RC_READ_BYTE; }}


#تعریف_کرو RC_TEST { اگر (بفر_2 == بفر_Lim) واپس LZMA_نتیجہ_ڈیٹا_ERROR; }

#تعریف_کرو RC_INIT(بفر, بفر_سائز) بفر_2 = بفر; بفر_Lim = بفر + بفر_سائز; RC_INIT2
 

#تعریف_کرو RC_NORMALIZE اگر (Range < k_چوٹی_قدر) { RC_TEST; Range <<= 8; رمز_2 = (رمز_2 << 8) | RC_READ_BYTE; }

#تعریف_کرو IfBit0(p) RC_NORMALIZE; bound = (Range >> k_عدد_ثنائی_ہندسہ_Model_کل_بٹ) * *(p); اگر (رمز_2 < bound)
#تعریف_کرو UpdateBit0(p) Range = bound; *(p) += (k_ثنائی_ہندسہ_Model_کل - *(p)) >> k_عدد_Move_بٹ;
#تعریف_کرو UpdateBit1(p) Range -= bound; رمز_2 -= bound; *(p) -= (*(p)) >> k_عدد_Move_بٹ;

#تعریف_کرو RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } ورنہ \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#تعریف_کرو RC_GET_ثنائی_ہندسہ(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#تعریف_کرو Range_Decoder_ثنائی_ہندسہ_درخت_Decode(probs, عدد_سطحیں, res) \
  { صحیح i = عدد_سطحیں; res = 1; \
  کرو { CProb *cp = probs + res; RC_GET_ثنائی_ہندسہ(cp, res) } جب(--i != 0); \
  res -= (1 << عدد_سطحیں); }


#تعریف_کرو k_عدد_Pos_بٹ_Max 4
#تعریف_کرو k_عدد_Pos_حالتیں_Max (1 << k_عدد_Pos_بٹ_Max)

#تعریف_کرو k_Len_عدد_کم_بٹ 3
#تعریف_کرو k_Len_عدد_کم_Symbols (1 << k_Len_عدد_کم_بٹ)
#تعریف_کرو k_Len_عدد_Mid_بٹ 3
#تعریف_کرو k_Len_عدد_Mid_Symbols (1 << k_Len_عدد_Mid_بٹ)
#تعریف_کرو k_Len_عدد_بلند_بٹ 8
#تعریف_کرو k_Len_عدد_بلند_Symbols (1 << k_Len_عدد_بلند_بٹ)

#تعریف_کرو LenChoice 0
#تعریف_کرو LenChoice2 (LenChoice + 1)
#تعریف_کرو Len_کم (LenChoice2 + 1)
#تعریف_کرو LenMid (Len_کم + (k_عدد_Pos_حالتیں_Max << k_Len_عدد_کم_بٹ))
#تعریف_کرو Len_بلند (LenMid + (k_عدد_Pos_حالتیں_Max << k_Len_عدد_Mid_بٹ))
#تعریف_کرو k_عدد_Len_Probs (Len_بلند + k_Len_عدد_بلند_Symbols) 


#تعریف_کرو k_عدد_حالتیں 12
#تعریف_کرو k_عدد_Lit_حالتیں 7

#تعریف_کرو k_آغاز_Pos_Model_اشاریہ 4
#تعریف_کرو k_انتہا_Pos_Model_اشاریہ 14
#تعریف_کرو k_عدد_بھرا_Distances (1 << (k_انتہا_Pos_Model_اشاریہ >> 1))

#تعریف_کرو k_عدد_Pos_خانہ_بٹ 6
#تعریف_کرو k_عدد_Len_مقصد_Pos_حالتیں 4

#تعریف_کرو k_عدد_Align_بٹ 4
#تعریف_کرو k_Align_جدول_سائز (1 << k_عدد_Align_بٹ)

#تعریف_کرو k_مطابقت_Min_Len 2

#تعریف_کرو Is_مطابقت 0
#تعریف_کرو IsRep (Is_مطابقت + (k_عدد_حالتیں << k_عدد_Pos_بٹ_Max))
#تعریف_کرو IsRepG0 (IsRep + k_عدد_حالتیں)
#تعریف_کرو IsRepG1 (IsRepG0 + k_عدد_حالتیں)
#تعریف_کرو IsRepG2 (IsRepG1 + k_عدد_حالتیں)
#تعریف_کرو IsRep0Long (IsRepG2 + k_عدد_حالتیں)
#تعریف_کرو Pos_خانہ (IsRep0Long + (k_عدد_حالتیں << k_عدد_Pos_بٹ_Max))
#تعریف_کرو SpecPos (Pos_خانہ + (k_عدد_Len_مقصد_Pos_حالتیں << k_عدد_Pos_خانہ_بٹ))
#تعریف_کرو Align (SpecPos + k_عدد_بھرا_Distances - k_انتہا_Pos_Model_اشاریہ)
#تعریف_کرو LenCoder (Align + k_Align_جدول_سائز)
#تعریف_کرو RepLenCoder (LenCoder + k_عدد_Len_Probs)
#تعریف_کرو Literal (RepLenCoder + k_عدد_Len_Probs)

#اگر Literal != LZMA_بنیاد_سائز
StopCompilingDueBUG
#اگر_ختم

صحیح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت غیرعلامتی حرف *props_ڈیٹا, صحیح سائز)
{
  غیرعلامتی حرف prop0;
  اگر (سائز < LZMA_PROPERTIES_سائز)
    واپس LZMA_نتیجہ_ڈیٹا_ERROR;
  prop0 = props_ڈیٹا[0];
  اگر (prop0 >= (9 * 5 * 5))
    واپس LZMA_نتیجہ_ڈیٹا_ERROR;
  {
    برائے (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    برائے (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  واپس LZMA_نتیجہ_OK;
}

#تعریف_کرو k_Lzma_دھارا_Was_ختم_شدہ_Id (-1)

صحیح LzmaDecode(CLzmaDecoderState *vs,
    ثابت غیرعلامتی حرف *اندر_دھارا, سائز_T اندر_سائز, سائز_T *اندر_سائز_Processed,
    غیرعلامتی حرف *برآمد_دھارا, سائز_T برآمد_سائز, سائز_T *برآمد_سائز_Processed)
{
  CProb *p = vs->Probs;
  سائز_T nowPos = 0;
  Byte پچھلا_Byte = 0;
  UInt32 pos_State_نقاب = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_نقاب = (1 << (vs->Properties.lp)) - 1;
  صحیح lc = vs->Properties.lc;


  صحیح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحیح len = 0;
  ثابت Byte *بفر_2;
  ثابت Byte *بفر_Lim;
  UInt32 Range;
  UInt32 رمز_2;

  *اندر_سائز_Processed = 0;
  *برآمد_سائز_Processed = 0;

  {
    UInt32 i;
    UInt32 عدد_Probs = Literal + ((UInt32)LZMA_LIT_سائز << (lc + vs->Properties.lp));
    برائے (i = 0; i < عدد_Probs; i++)
      p[i] = k_ثنائی_ہندسہ_Model_کل >> 1;
  }
  
  RC_INIT(اندر_دھارا, اندر_سائز);


  جب(nowPos < برآمد_سائز)
  {
    CProb *prob;
    UInt32 bound;
    صحیح posState = (صحیح)(
        (nowPos 
        )
        & pos_State_نقاب);

    prob = p + Is_مطابقت + (state << k_عدد_Pos_بٹ_Max) + posState;
    IfBit0(prob)
    {
      صحیح علامت_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_سائز * 
        (((
        (nowPos 
        )
        & literal_Pos_نقاب) << lc) + (پچھلا_Byte >> (8 - lc))));

      اگر (state >= k_عدد_Lit_حالتیں)
      {
        صحیح مطابقت_Byte;
        مطابقت_Byte = برآمد_دھارا[nowPos - rep0];
        کرو
        {
          صحیح ثنائی_ہندسہ;
          CProb *probLit;
          مطابقت_Byte <<= 1;
          ثنائی_ہندسہ = (مطابقت_Byte & 0x100);
          probLit = prob + 0x100 + ثنائی_ہندسہ + علامت_2;
          RC_GET_BIT2(probLit, علامت_2, اگر (ثنائی_ہندسہ != 0) توڑو, اگر (ثنائی_ہندسہ == 0) توڑو)
        }
        جب (علامت_2 < 0x100);
      }
      جب (علامت_2 < 0x100)
      {
        CProb *probLit = prob + علامت_2;
        RC_GET_ثنائی_ہندسہ(probLit, علامت_2)
      }
      پچھلا_Byte = (Byte)علامت_2;

      برآمد_دھارا[nowPos++] = پچھلا_Byte;
      اگر (state < 4) state = 0;
      ورنہ اگر (state < 10) state -= 3;
      ورنہ state -= 6;
    }
    ورنہ             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_عدد_Lit_حالتیں ? 0 : 3;
        prob = p + LenCoder;
      }
      ورنہ
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_عدد_Pos_بٹ_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            اگر (nowPos == 0)
              واپس LZMA_نتیجہ_ڈیٹا_ERROR;
            
            state = state < k_عدد_Lit_حالتیں ? 9 : 11;
            پچھلا_Byte = برآمد_دھارا[nowPos - rep0];
            برآمد_دھارا[nowPos++] = پچھلا_Byte;

            جاری;
          }
          ورنہ
          {
            UpdateBit1(prob);
          }
        }
        ورنہ
        {
          UInt32 فاصلہ;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            فاصلہ = rep1;
          }
          ورنہ 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              فاصلہ = rep2;
            }
            ورنہ
            {
              UpdateBit1(prob);
              فاصلہ = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = فاصلہ;
        }
        state = state < k_عدد_Lit_حالتیں ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحیح عدد_بٹ, انحراف_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_کم + (posState << k_Len_عدد_کم_بٹ);
          انحراف_2 = 0;
          عدد_بٹ = k_Len_عدد_کم_بٹ;
        }
        ورنہ
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_عدد_Mid_بٹ);
            انحراف_2 = k_Len_عدد_کم_Symbols;
            عدد_بٹ = k_Len_عدد_Mid_بٹ;
          }
          ورنہ
          {
            UpdateBit1(probLen);
            probLen = prob + Len_بلند;
            انحراف_2 = k_Len_عدد_کم_Symbols + k_Len_عدد_Mid_Symbols;
            عدد_بٹ = k_Len_عدد_بلند_بٹ;
          }
        }
        Range_Decoder_ثنائی_ہندسہ_درخت_Decode(probLen, عدد_بٹ, len);
        len += انحراف_2;
      }

      اگر (state < 4)
      {
        صحیح pos_خانہ;
        state += k_عدد_Lit_حالتیں;
        prob = p + Pos_خانہ +
            ((len < k_عدد_Len_مقصد_Pos_حالتیں ? len : k_عدد_Len_مقصد_Pos_حالتیں - 1) << 
            k_عدد_Pos_خانہ_بٹ);
        Range_Decoder_ثنائی_ہندسہ_درخت_Decode(prob, k_عدد_Pos_خانہ_بٹ, pos_خانہ);
        اگر (pos_خانہ >= k_آغاز_Pos_Model_اشاریہ)
        {
          صحیح عدد_براہ_راست_بٹ = ((pos_خانہ >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_خانہ & 1));
          اگر (pos_خانہ < k_انتہا_Pos_Model_اشاریہ)
          {
            rep0 <<= عدد_براہ_راست_بٹ;
            prob = p + SpecPos + rep0 - pos_خانہ - 1;
          }
          ورنہ
          {
            عدد_براہ_راست_بٹ -= k_عدد_Align_بٹ;
            کرو
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              اگر (رمز_2 >= Range)
              {
                رمز_2 -= Range;
                rep0 |= 1;
              }
            }
            جب (--عدد_براہ_راست_بٹ != 0);
            prob = p + Align;
            rep0 <<= k_عدد_Align_بٹ;
            عدد_براہ_راست_بٹ = k_عدد_Align_بٹ;
          }
          {
            صحیح i = 1;
            صحیح mi = 1;
            کرو
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            جب(--عدد_براہ_راست_بٹ != 0);
          }
        }
        ورنہ
          rep0 = pos_خانہ;
        اگر (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_دھارا_Was_ختم_شدہ_Id;
          توڑو;
        }
      }

      len += k_مطابقت_Min_Len;
      اگر (rep0 > nowPos)
        واپس LZMA_نتیجہ_ڈیٹا_ERROR;


      کرو
      {
        پچھلا_Byte = برآمد_دھارا[nowPos - rep0];
        len--;
        برآمد_دھارا[nowPos++] = پچھلا_Byte;
      }
      جب(len != 0 && nowPos < برآمد_سائز);
    }
  }
  RC_NORMALIZE;


  *اندر_سائز_Processed = (سائز_T)(بفر_2 - اندر_دھارا);
  *برآمد_سائز_Processed = nowPos;
  واپس LZMA_نتیجہ_OK;
}
