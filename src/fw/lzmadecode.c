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

#تعریف_کړه k_عدد_سر_بټونه 24
#تعریف_کړه k_سر_ارزښت ((UInt32)1 << k_عدد_سر_بټونه)

#تعریف_کړه k_عدد_بټ_Model_ټول_بټونه 11
#تعریف_کړه k_بټ_Model_ټول (1 << k_عدد_بټ_Model_ټول_بټونه)
#تعریف_کړه k_عدد_Move_بټونه 5

#تعریف_کړه RC_READ_BYTE (*بفر_2++)

#تعریف_کړه RC_INIT2 کوډ_2 = 0; Range = 0xFFFFFFFF; \
  { صحيح i; لكل(i = 0; i < 5; i++) { RC_TEST; کوډ_2 = (کوډ_2 << 8) | RC_READ_BYTE; }}


#تعریف_کړه RC_TEST { إذا (بفر_2 == بفر_Lim) أرجع LZMA_پایله_معلومات_ERROR; }

#تعریف_کړه RC_INIT(بفر, بفر_اندازه) بفر_2 = بفر; بفر_Lim = بفر + بفر_اندازه; RC_INIT2
 

#تعریف_کړه RC_NORMALIZE إذا (Range < k_سر_ارزښت) { RC_TEST; Range <<= 8; کوډ_2 = (کوډ_2 << 8) | RC_READ_BYTE; }

#تعریف_کړه IfBit0(p) RC_NORMALIZE; bound = (Range >> k_عدد_بټ_Model_ټول_بټونه) * *(p); إذا (کوډ_2 < bound)
#تعریف_کړه UpdateBit0(p) Range = bound; *(p) += (k_بټ_Model_ټول - *(p)) >> k_عدد_Move_بټونه;
#تعریف_کړه UpdateBit1(p) Range -= bound; کوډ_2 -= bound; *(p) -= (*(p)) >> k_عدد_Move_بټونه;

#تعریف_کړه RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } وإلا \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#تعریف_کړه RC_GET_بټ(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#تعریف_کړه Range_Decoder_بټ_ونه_Decode(probs, عدد_پوړونه, res) \
  { صحيح i = عدد_پوړونه; res = 1; \
  افعل { CProb *cp = probs + res; RC_GET_بټ(cp, res) } طالما(--i != 0); \
  res -= (1 << عدد_پوړونه); }


#تعریف_کړه k_عدد_Pos_بټونه_Max 4
#تعریف_کړه k_عدد_Pos_حالتونه_Max (1 << k_عدد_Pos_بټونه_Max)

#تعریف_کړه k_Len_عدد_ټیټ_بټونه 3
#تعریف_کړه k_Len_عدد_ټیټ_Symbols (1 << k_Len_عدد_ټیټ_بټونه)
#تعریف_کړه k_Len_عدد_Mid_بټونه 3
#تعریف_کړه k_Len_عدد_Mid_Symbols (1 << k_Len_عدد_Mid_بټونه)
#تعریف_کړه k_Len_عدد_لوړ_بټونه 8
#تعریف_کړه k_Len_عدد_لوړ_Symbols (1 << k_Len_عدد_لوړ_بټونه)

#تعریف_کړه LenChoice 0
#تعریف_کړه LenChoice2 (LenChoice + 1)
#تعریف_کړه Len_ټیټ (LenChoice2 + 1)
#تعریف_کړه LenMid (Len_ټیټ + (k_عدد_Pos_حالتونه_Max << k_Len_عدد_ټیټ_بټونه))
#تعریف_کړه Len_لوړ (LenMid + (k_عدد_Pos_حالتونه_Max << k_Len_عدد_Mid_بټونه))
#تعریف_کړه k_عدد_Len_Probs (Len_لوړ + k_Len_عدد_لوړ_Symbols) 


#تعریف_کړه k_عدد_حالتونه 12
#تعریف_کړه k_عدد_Lit_حالتونه 7

#تعریف_کړه k_پیل_Pos_Model_شاخص 4
#تعریف_کړه k_پای_ټکی_Pos_Model_شاخص 14
#تعریف_کړه k_عدد_ډک_Distances (1 << (k_پای_ټکی_Pos_Model_شاخص >> 1))

#تعریف_کړه k_عدد_Pos_خونه_بټونه 6
#تعریف_کړه k_عدد_Len_منزل_Pos_حالتونه 4

#تعریف_کړه k_عدد_Align_بټونه 4
#تعریف_کړه k_Align_جدول_اندازه (1 << k_عدد_Align_بټونه)

#تعریف_کړه k_سمون_Min_Len 2

#تعریف_کړه Is_سمون 0
#تعریف_کړه IsRep (Is_سمون + (k_عدد_حالتونه << k_عدد_Pos_بټونه_Max))
#تعریف_کړه IsRepG0 (IsRep + k_عدد_حالتونه)
#تعریف_کړه IsRepG1 (IsRepG0 + k_عدد_حالتونه)
#تعریف_کړه IsRepG2 (IsRepG1 + k_عدد_حالتونه)
#تعریف_کړه IsRep0Long (IsRepG2 + k_عدد_حالتونه)
#تعریف_کړه Pos_خونه (IsRep0Long + (k_عدد_حالتونه << k_عدد_Pos_بټونه_Max))
#تعریف_کړه SpecPos (Pos_خونه + (k_عدد_Len_منزل_Pos_حالتونه << k_عدد_Pos_خونه_بټونه))
#تعریف_کړه Align (SpecPos + k_عدد_ډک_Distances - k_پای_ټکی_Pos_Model_شاخص)
#تعریف_کړه LenCoder (Align + k_Align_جدول_اندازه)
#تعریف_کړه RepLenCoder (LenCoder + k_عدد_Len_Probs)
#تعریف_کړه Literal (RepLenCoder + k_عدد_Len_Probs)

#إذا Literal != LZMA_بنسټ_اندازه
StopCompilingDueBUG
#د_که_پای

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بینښه حرف *props_معلومات, صحيح اندازه)
{
  بینښه حرف prop0;
  إذا (اندازه < LZMA_PROPERTIES_اندازه)
    أرجع LZMA_پایله_معلومات_ERROR;
  prop0 = props_معلومات[0];
  إذا (prop0 >= (9 * 5 * 5))
    أرجع LZMA_پایله_معلومات_ERROR;
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

  أرجع LZMA_پایله_OK;
}

#تعریف_کړه k_Lzma_بهیر_Was_پای_ته_رسېدلی_Id (-1)

صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بینښه حرف *دننه_بهیر, اندازه_T دننه_اندازه, اندازه_T *دننه_اندازه_Processed,
    بینښه حرف *بهر_بهیر, اندازه_T بهر_اندازه, اندازه_T *بهر_اندازه_Processed)
{
  CProb *p = vs->Probs;
  اندازه_T nowPos = 0;
  Byte پخوانی_Byte = 0;
  UInt32 pos_State_ماسک = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_ماسک = (1 << (vs->Properties.lp)) - 1;
  صحيح lc = vs->Properties.lc;


  صحيح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحيح len = 0;
  ثابت Byte *بفر_2;
  ثابت Byte *بفر_Lim;
  UInt32 Range;
  UInt32 کوډ_2;

  *دننه_اندازه_Processed = 0;
  *بهر_اندازه_Processed = 0;

  {
    UInt32 i;
    UInt32 عدد_Probs = Literal + ((UInt32)LZMA_LIT_اندازه << (lc + vs->Properties.lp));
    لكل (i = 0; i < عدد_Probs; i++)
      p[i] = k_بټ_Model_ټول >> 1;
  }
  
  RC_INIT(دننه_بهیر, دننه_اندازه);


  طالما(nowPos < بهر_اندازه)
  {
    CProb *prob;
    UInt32 bound;
    صحيح posState = (صحيح)(
        (nowPos 
        )
        & pos_State_ماسک);

    prob = p + Is_سمون + (state << k_عدد_Pos_بټونه_Max) + posState;
    IfBit0(prob)
    {
      صحيح نښه_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_اندازه * 
        (((
        (nowPos 
        )
        & literal_Pos_ماسک) << lc) + (پخوانی_Byte >> (8 - lc))));

      إذا (state >= k_عدد_Lit_حالتونه)
      {
        صحيح سمون_Byte;
        سمون_Byte = بهر_بهیر[nowPos - rep0];
        افعل
        {
          صحيح بټ;
          CProb *probLit;
          سمون_Byte <<= 1;
          بټ = (سمون_Byte & 0x100);
          probLit = prob + 0x100 + بټ + نښه_2;
          RC_GET_BIT2(probLit, نښه_2, إذا (بټ != 0) اكسر, إذا (بټ == 0) اكسر)
        }
        طالما (نښه_2 < 0x100);
      }
      طالما (نښه_2 < 0x100)
      {
        CProb *probLit = prob + نښه_2;
        RC_GET_بټ(probLit, نښه_2)
      }
      پخوانی_Byte = (Byte)نښه_2;

      بهر_بهیر[nowPos++] = پخوانی_Byte;
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
        state = state < k_عدد_Lit_حالتونه ? 0 : 3;
        prob = p + LenCoder;
      }
      وإلا
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_عدد_Pos_بټونه_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            إذا (nowPos == 0)
              أرجع LZMA_پایله_معلومات_ERROR;
            
            state = state < k_عدد_Lit_حالتونه ? 9 : 11;
            پخوانی_Byte = بهر_بهیر[nowPos - rep0];
            بهر_بهیر[nowPos++] = پخوانی_Byte;

            استمر;
          }
          وإلا
          {
            UpdateBit1(prob);
          }
        }
        وإلا
        {
          UInt32 واټن;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            واټن = rep1;
          }
          وإلا 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              واټن = rep2;
            }
            وإلا
            {
              UpdateBit1(prob);
              واټن = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = واټن;
        }
        state = state < k_عدد_Lit_حالتونه ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحيح عدد_بټونه, دننه_واټن_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_ټیټ + (posState << k_Len_عدد_ټیټ_بټونه);
          دننه_واټن_2 = 0;
          عدد_بټونه = k_Len_عدد_ټیټ_بټونه;
        }
        وإلا
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_عدد_Mid_بټونه);
            دننه_واټن_2 = k_Len_عدد_ټیټ_Symbols;
            عدد_بټونه = k_Len_عدد_Mid_بټونه;
          }
          وإلا
          {
            UpdateBit1(probLen);
            probLen = prob + Len_لوړ;
            دننه_واټن_2 = k_Len_عدد_ټیټ_Symbols + k_Len_عدد_Mid_Symbols;
            عدد_بټونه = k_Len_عدد_لوړ_بټونه;
          }
        }
        Range_Decoder_بټ_ونه_Decode(probLen, عدد_بټونه, len);
        len += دننه_واټن_2;
      }

      إذا (state < 4)
      {
        صحيح pos_خونه;
        state += k_عدد_Lit_حالتونه;
        prob = p + Pos_خونه +
            ((len < k_عدد_Len_منزل_Pos_حالتونه ? len : k_عدد_Len_منزل_Pos_حالتونه - 1) << 
            k_عدد_Pos_خونه_بټونه);
        Range_Decoder_بټ_ونه_Decode(prob, k_عدد_Pos_خونه_بټونه, pos_خونه);
        إذا (pos_خونه >= k_پیل_Pos_Model_شاخص)
        {
          صحيح عدد_مستقیم_بټونه = ((pos_خونه >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_خونه & 1));
          إذا (pos_خونه < k_پای_ټکی_Pos_Model_شاخص)
          {
            rep0 <<= عدد_مستقیم_بټونه;
            prob = p + SpecPos + rep0 - pos_خونه - 1;
          }
          وإلا
          {
            عدد_مستقیم_بټونه -= k_عدد_Align_بټونه;
            افعل
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              إذا (کوډ_2 >= Range)
              {
                کوډ_2 -= Range;
                rep0 |= 1;
              }
            }
            طالما (--عدد_مستقیم_بټونه != 0);
            prob = p + Align;
            rep0 <<= k_عدد_Align_بټونه;
            عدد_مستقیم_بټونه = k_عدد_Align_بټونه;
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
            طالما(--عدد_مستقیم_بټونه != 0);
          }
        }
        وإلا
          rep0 = pos_خونه;
        إذا (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_بهیر_Was_پای_ته_رسېدلی_Id;
          اكسر;
        }
      }

      len += k_سمون_Min_Len;
      إذا (rep0 > nowPos)
        أرجع LZMA_پایله_معلومات_ERROR;


      افعل
      {
        پخوانی_Byte = بهر_بهیر[nowPos - rep0];
        len--;
        بهر_بهیر[nowPos++] = پخوانی_Byte;
      }
      طالما(len != 0 && nowPos < بهر_اندازه);
    }
  }
  RC_NORMALIZE;


  *دننه_اندازه_Processed = (اندازه_T)(بفر_2 - دننه_بهیر);
  *بهر_اندازه_Processed = nowPos;
  أرجع LZMA_پایله_OK;
}
