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

#تعریف_کرو k_عدد_چوٹی_بٹ 24
#تعریف_کرو k_چوٹی_قدر ((UInt32)1 << k_عدد_چوٹی_بٹ)

#تعریف_کرو k_عدد_ثنائی_ہندسہ_Model_کل_بٹ 11
#تعریف_کرو k_ثنائی_ہندسہ_Model_کل (1 << k_عدد_ثنائی_ہندسہ_Model_کل_بٹ)
#تعریف_کرو k_عدد_Move_بٹ 5

#تعریف_کرو RC_READ_BYTE (*بفر_2++)

#تعریف_کرو RC_INIT2 کوڈ_2 = 0; Range = 0xFFFFFFFF; \
  { صحيح i; لكل(i = 0; i < 5; i++) { RC_TEST; کوڈ_2 = (کوڈ_2 << 8) | RC_READ_BYTE; }}


#تعریف_کرو RC_TEST { إذا (بفر_2 == بفر_Lim) أرجع LZMA_نتیجہ_انکڑے_ERROR; }

#تعریف_کرو RC_INIT(بفر, بفر_ناپ) بفر_2 = بفر; بفر_Lim = بفر + بفر_ناپ; RC_INIT2
 

#تعریف_کرو RC_NORMALIZE إذا (Range < k_چوٹی_قدر) { RC_TEST; Range <<= 8; کوڈ_2 = (کوڈ_2 << 8) | RC_READ_BYTE; }

#تعریف_کرو IfBit0(p) RC_NORMALIZE; bound = (Range >> k_عدد_ثنائی_ہندسہ_Model_کل_بٹ) * *(p); إذا (کوڈ_2 < bound)
#تعریف_کرو UpdateBit0(p) Range = bound; *(p) += (k_ثنائی_ہندسہ_Model_کل - *(p)) >> k_عدد_Move_بٹ;
#تعریف_کرو UpdateBit1(p) Range -= bound; کوڈ_2 -= bound; *(p) -= (*(p)) >> k_عدد_Move_بٹ;

#تعریف_کرو RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } وإلا \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#تعریف_کرو RC_GET_ثنائی_ہندسہ(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#تعریف_کرو Range_Decoder_ثنائی_ہندسہ_درخت_Decode(probs, عدد_پدھراں, res) \
  { صحيح i = عدد_پدھراں; res = 1; \
  افعل { CProb *cp = probs + res; RC_GET_ثنائی_ہندسہ(cp, res) } طالما(--i != 0); \
  res -= (1 << عدد_پدھراں); }


#تعریف_کرو k_عدد_Pos_بٹ_Max 4
#تعریف_کرو k_عدد_Pos_حالتاں_Max (1 << k_عدد_Pos_بٹ_Max)

#تعریف_کرو k_Len_عدد_کم_بٹ 3
#تعریف_کرو k_Len_عدد_کم_Symbols (1 << k_Len_عدد_کم_بٹ)
#تعریف_کرو k_Len_عدد_Mid_بٹ 3
#تعریف_کرو k_Len_عدد_Mid_Symbols (1 << k_Len_عدد_Mid_بٹ)
#تعریف_کرو k_Len_عدد_بلند_بٹ 8
#تعریف_کرو k_Len_عدد_بلند_Symbols (1 << k_Len_عدد_بلند_بٹ)

#تعریف_کرو LenChoice 0
#تعریف_کرو LenChoice2 (LenChoice + 1)
#تعریف_کرو Len_کم (LenChoice2 + 1)
#تعریف_کرو LenMid (Len_کم + (k_عدد_Pos_حالتاں_Max << k_Len_عدد_کم_بٹ))
#تعریف_کرو Len_بلند (LenMid + (k_عدد_Pos_حالتاں_Max << k_Len_عدد_Mid_بٹ))
#تعریف_کرو k_عدد_Len_Probs (Len_بلند + k_Len_عدد_بلند_Symbols) 


#تعریف_کرو k_عدد_حالتاں 12
#تعریف_کرو k_عدد_Lit_حالتاں 7

#تعریف_کرو k_شروع_Pos_Model_اشاریہ 4
#تعریف_کرو k_سرا_Pos_Model_اشاریہ 14
#تعریف_کرو k_عدد_بھریا_Distances (1 << (k_سرا_Pos_Model_اشاریہ >> 1))

#تعریف_کرو k_عدد_Pos_تھاں_بٹ 6
#تعریف_کرو k_عدد_Len_منزل_Pos_حالتاں 4

#تعریف_کرو k_عدد_Align_بٹ 4
#تعریف_کرو k_Align_جدول_ناپ (1 << k_عدد_Align_بٹ)

#تعریف_کرو k_میل_Min_Len 2

#تعریف_کرو Is_میل 0
#تعریف_کرو IsRep (Is_میل + (k_عدد_حالتاں << k_عدد_Pos_بٹ_Max))
#تعریف_کرو IsRepG0 (IsRep + k_عدد_حالتاں)
#تعریف_کرو IsRepG1 (IsRepG0 + k_عدد_حالتاں)
#تعریف_کرو IsRepG2 (IsRepG1 + k_عدد_حالتاں)
#تعریف_کرو IsRep0Long (IsRepG2 + k_عدد_حالتاں)
#تعریف_کرو Pos_تھاں (IsRep0Long + (k_عدد_حالتاں << k_عدد_Pos_بٹ_Max))
#تعریف_کرو SpecPos (Pos_تھاں + (k_عدد_Len_منزل_Pos_حالتاں << k_عدد_Pos_تھاں_بٹ))
#تعریف_کرو Align (SpecPos + k_عدد_بھریا_Distances - k_سرا_Pos_Model_اشاریہ)
#تعریف_کرو LenCoder (Align + k_Align_جدول_ناپ)
#تعریف_کرو RepLenCoder (LenCoder + k_عدد_Len_Probs)
#تعریف_کرو Literal (RepLenCoder + k_عدد_Len_Probs)

#إذا Literal != LZMA_ادھار_ناپ
StopCompilingDueBUG
#جے_انت

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بنانشانی حرف *props_انکڑے, صحيح ناپ)
{
  بنانشانی حرف prop0;
  إذا (ناپ < LZMA_PROPERTIES_ناپ)
    أرجع LZMA_نتیجہ_انکڑے_ERROR;
  prop0 = props_انکڑے[0];
  إذا (prop0 >= (9 * 5 * 5))
    أرجع LZMA_نتیجہ_انکڑے_ERROR;
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

  أرجع LZMA_نتیجہ_OK;
}

#تعریف_کرو k_Lzma_دھارا_Was_مکمل_Id (-1)

صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بنانشانی حرف *وچ_دھارا, ناپ_T وچ_ناپ, ناپ_T *وچ_ناپ_Processed,
    بنانشانی حرف *باہرلا_دھارا, ناپ_T باہرلا_ناپ, ناپ_T *باہرلا_ناپ_Processed)
{
  CProb *p = vs->Probs;
  ناپ_T nowPos = 0;
  Byte پچھلا_Byte = 0;
  UInt32 pos_State_نقاب = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_نقاب = (1 << (vs->Properties.lp)) - 1;
  صحيح lc = vs->Properties.lc;


  صحيح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحيح len = 0;
  ثابت Byte *بفر_2;
  ثابت Byte *بفر_Lim;
  UInt32 Range;
  UInt32 کوڈ_2;

  *وچ_ناپ_Processed = 0;
  *باہرلا_ناپ_Processed = 0;

  {
    UInt32 i;
    UInt32 عدد_Probs = Literal + ((UInt32)LZMA_LIT_ناپ << (lc + vs->Properties.lp));
    لكل (i = 0; i < عدد_Probs; i++)
      p[i] = k_ثنائی_ہندسہ_Model_کل >> 1;
  }
  
  RC_INIT(وچ_دھارا, وچ_ناپ);


  طالما(nowPos < باہرلا_ناپ)
  {
    CProb *prob;
    UInt32 bound;
    صحيح posState = (صحيح)(
        (nowPos 
        )
        & pos_State_نقاب);

    prob = p + Is_میل + (state << k_عدد_Pos_بٹ_Max) + posState;
    IfBit0(prob)
    {
      صحيح علامت_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_ناپ * 
        (((
        (nowPos 
        )
        & literal_Pos_نقاب) << lc) + (پچھلا_Byte >> (8 - lc))));

      إذا (state >= k_عدد_Lit_حالتاں)
      {
        صحيح میل_Byte;
        میل_Byte = باہرلا_دھارا[nowPos - rep0];
        افعل
        {
          صحيح ثنائی_ہندسہ;
          CProb *probLit;
          میل_Byte <<= 1;
          ثنائی_ہندسہ = (میل_Byte & 0x100);
          probLit = prob + 0x100 + ثنائی_ہندسہ + علامت_2;
          RC_GET_BIT2(probLit, علامت_2, إذا (ثنائی_ہندسہ != 0) اكسر, إذا (ثنائی_ہندسہ == 0) اكسر)
        }
        طالما (علامت_2 < 0x100);
      }
      طالما (علامت_2 < 0x100)
      {
        CProb *probLit = prob + علامت_2;
        RC_GET_ثنائی_ہندسہ(probLit, علامت_2)
      }
      پچھلا_Byte = (Byte)علامت_2;

      باہرلا_دھارا[nowPos++] = پچھلا_Byte;
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
        state = state < k_عدد_Lit_حالتاں ? 0 : 3;
        prob = p + LenCoder;
      }
      وإلا
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
            
            إذا (nowPos == 0)
              أرجع LZMA_نتیجہ_انکڑے_ERROR;
            
            state = state < k_عدد_Lit_حالتاں ? 9 : 11;
            پچھلا_Byte = باہرلا_دھارا[nowPos - rep0];
            باہرلا_دھارا[nowPos++] = پچھلا_Byte;

            استمر;
          }
          وإلا
          {
            UpdateBit1(prob);
          }
        }
        وإلا
        {
          UInt32 فاصلہ;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            فاصلہ = rep1;
          }
          وإلا 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              فاصلہ = rep2;
            }
            وإلا
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
        state = state < k_عدد_Lit_حالتاں ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحيح عدد_بٹ, وتھ_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_کم + (posState << k_Len_عدد_کم_بٹ);
          وتھ_2 = 0;
          عدد_بٹ = k_Len_عدد_کم_بٹ;
        }
        وإلا
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_عدد_Mid_بٹ);
            وتھ_2 = k_Len_عدد_کم_Symbols;
            عدد_بٹ = k_Len_عدد_Mid_بٹ;
          }
          وإلا
          {
            UpdateBit1(probLen);
            probLen = prob + Len_بلند;
            وتھ_2 = k_Len_عدد_کم_Symbols + k_Len_عدد_Mid_Symbols;
            عدد_بٹ = k_Len_عدد_بلند_بٹ;
          }
        }
        Range_Decoder_ثنائی_ہندسہ_درخت_Decode(probLen, عدد_بٹ, len);
        len += وتھ_2;
      }

      إذا (state < 4)
      {
        صحيح pos_تھاں;
        state += k_عدد_Lit_حالتاں;
        prob = p + Pos_تھاں +
            ((len < k_عدد_Len_منزل_Pos_حالتاں ? len : k_عدد_Len_منزل_Pos_حالتاں - 1) << 
            k_عدد_Pos_تھاں_بٹ);
        Range_Decoder_ثنائی_ہندسہ_درخت_Decode(prob, k_عدد_Pos_تھاں_بٹ, pos_تھاں);
        إذا (pos_تھاں >= k_شروع_Pos_Model_اشاریہ)
        {
          صحيح عدد_سدھا_بٹ = ((pos_تھاں >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_تھاں & 1));
          إذا (pos_تھاں < k_سرا_Pos_Model_اشاریہ)
          {
            rep0 <<= عدد_سدھا_بٹ;
            prob = p + SpecPos + rep0 - pos_تھاں - 1;
          }
          وإلا
          {
            عدد_سدھا_بٹ -= k_عدد_Align_بٹ;
            افعل
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              إذا (کوڈ_2 >= Range)
              {
                کوڈ_2 -= Range;
                rep0 |= 1;
              }
            }
            طالما (--عدد_سدھا_بٹ != 0);
            prob = p + Align;
            rep0 <<= k_عدد_Align_بٹ;
            عدد_سدھا_بٹ = k_عدد_Align_بٹ;
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
            طالما(--عدد_سدھا_بٹ != 0);
          }
        }
        وإلا
          rep0 = pos_تھاں;
        إذا (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_دھارا_Was_مکمل_Id;
          اكسر;
        }
      }

      len += k_میل_Min_Len;
      إذا (rep0 > nowPos)
        أرجع LZMA_نتیجہ_انکڑے_ERROR;


      افعل
      {
        پچھلا_Byte = باہرلا_دھارا[nowPos - rep0];
        len--;
        باہرلا_دھارا[nowPos++] = پچھلا_Byte;
      }
      طالما(len != 0 && nowPos < باہرلا_ناپ);
    }
  }
  RC_NORMALIZE;


  *وچ_ناپ_Processed = (ناپ_T)(بفر_2 - وچ_دھارا);
  *باہرلا_ناپ_Processed = nowPos;
  أرجع LZMA_نتیجہ_OK;
}
