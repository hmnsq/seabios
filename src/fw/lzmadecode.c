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

#تعریف_کن k_رقم_بالاترین_بیتها 24
#تعریف_کن k_بالاترین_مقدار ((UInt32)1 << k_رقم_بالاترین_بیتها)

#تعریف_کن k_رقم_بیت_Model_مجموع_بیتها 11
#تعریف_کن k_بیت_Model_مجموع (1 << k_رقم_بیت_Model_مجموع_بیتها)
#تعریف_کن k_رقم_Move_بیتها 5

#تعریف_کن RC_READ_BYTE (*میانگیر_2++)

#تعریف_کن RC_INIT2 کد_2 = 0; Range = 0xFFFFFFFF; \
  { صحيح i; لكل(i = 0; i < 5; i++) { RC_TEST; کد_2 = (کد_2 << 8) | RC_READ_BYTE; }}


#تعریف_کن RC_TEST { إذا (میانگیر_2 == میانگیر_Lim) أرجع LZMA_نتیجه_داده_ERROR; }

#تعریف_کن RC_INIT(میانگیر, میانگیر_اندازه) میانگیر_2 = میانگیر; میانگیر_Lim = میانگیر + میانگیر_اندازه; RC_INIT2
 

#تعریف_کن RC_NORMALIZE إذا (Range < k_بالاترین_مقدار) { RC_TEST; Range <<= 8; کد_2 = (کد_2 << 8) | RC_READ_BYTE; }

#تعریف_کن IfBit0(p) RC_NORMALIZE; bound = (Range >> k_رقم_بیت_Model_مجموع_بیتها) * *(p); إذا (کد_2 < bound)
#تعریف_کن UpdateBit0(p) Range = bound; *(p) += (k_بیت_Model_مجموع - *(p)) >> k_رقم_Move_بیتها;
#تعریف_کن UpdateBit1(p) Range -= bound; کد_2 -= bound; *(p) -= (*(p)) >> k_رقم_Move_بیتها;

#تعریف_کن RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } وإلا \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#تعریف_کن RC_GET_بیت(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#تعریف_کن Range_Decoder_بیت_درخت_Decode(probs, رقم_سطوح, res) \
  { صحيح i = رقم_سطوح; res = 1; \
  افعل { CProb *cp = probs + res; RC_GET_بیت(cp, res) } طالما(--i != 0); \
  res -= (1 << رقم_سطوح); }


#تعریف_کن k_رقم_Pos_بیتها_Max 4
#تعریف_کن k_رقم_Pos_حالتها_Max (1 << k_رقم_Pos_بیتها_Max)

#تعریف_کن k_Len_رقم_کم_بیتها 3
#تعریف_کن k_Len_رقم_کم_Symbols (1 << k_Len_رقم_کم_بیتها)
#تعریف_کن k_Len_رقم_Mid_بیتها 3
#تعریف_کن k_Len_رقم_Mid_Symbols (1 << k_Len_رقم_Mid_بیتها)
#تعریف_کن k_Len_رقم_بالا_بیتها 8
#تعریف_کن k_Len_رقم_بالا_Symbols (1 << k_Len_رقم_بالا_بیتها)

#تعریف_کن LenChoice 0
#تعریف_کن LenChoice2 (LenChoice + 1)
#تعریف_کن Len_کم (LenChoice2 + 1)
#تعریف_کن LenMid (Len_کم + (k_رقم_Pos_حالتها_Max << k_Len_رقم_کم_بیتها))
#تعریف_کن Len_بالا (LenMid + (k_رقم_Pos_حالتها_Max << k_Len_رقم_Mid_بیتها))
#تعریف_کن k_رقم_Len_Probs (Len_بالا + k_Len_رقم_بالا_Symbols) 


#تعریف_کن k_رقم_حالتها 12
#تعریف_کن k_رقم_Lit_حالتها 7

#تعریف_کن k_شروع_Pos_Model_شاخص 4
#تعریف_کن k_انتها_Pos_Model_شاخص 14
#تعریف_کن k_رقم_پر_Distances (1 << (k_انتها_Pos_Model_شاخص >> 1))

#تعریف_کن k_رقم_Pos_جایگاه_بیتها 6
#تعریف_کن k_رقم_Len_مقصد_Pos_حالتها 4

#تعریف_کن k_رقم_Align_بیتها 4
#تعریف_کن k_Align_جدول_اندازه (1 << k_رقم_Align_بیتها)

#تعریف_کن k_تطابق_Min_Len 2

#تعریف_کن Is_تطابق 0
#تعریف_کن IsRep (Is_تطابق + (k_رقم_حالتها << k_رقم_Pos_بیتها_Max))
#تعریف_کن IsRepG0 (IsRep + k_رقم_حالتها)
#تعریف_کن IsRepG1 (IsRepG0 + k_رقم_حالتها)
#تعریف_کن IsRepG2 (IsRepG1 + k_رقم_حالتها)
#تعریف_کن IsRep0Long (IsRepG2 + k_رقم_حالتها)
#تعریف_کن Pos_جایگاه (IsRep0Long + (k_رقم_حالتها << k_رقم_Pos_بیتها_Max))
#تعریف_کن SpecPos (Pos_جایگاه + (k_رقم_Len_مقصد_Pos_حالتها << k_رقم_Pos_جایگاه_بیتها))
#تعریف_کن Align (SpecPos + k_رقم_پر_Distances - k_انتها_Pos_Model_شاخص)
#تعریف_کن LenCoder (Align + k_Align_جدول_اندازه)
#تعریف_کن RepLenCoder (LenCoder + k_رقم_Len_Probs)
#تعریف_کن Literal (RepLenCoder + k_رقم_Len_Probs)

#إذا Literal != LZMA_پایه_اندازه
StopCompilingDueBUG
#پایان_اگر

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بیعلامت حرف *props_داده, صحيح اندازه)
{
  بیعلامت حرف prop0;
  إذا (اندازه < LZMA_PROPERTIES_اندازه)
    أرجع LZMA_نتیجه_داده_ERROR;
  prop0 = props_داده[0];
  إذا (prop0 >= (9 * 5 * 5))
    أرجع LZMA_نتیجه_داده_ERROR;
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

  أرجع LZMA_نتیجه_OK;
}

#تعریف_کن k_Lzma_جریان_Was_پایانیافته_Id (-1)

صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بیعلامت حرف *درون_جریان, اندازه_T درون_اندازه, اندازه_T *درون_اندازه_Processed,
    بیعلامت حرف *بیرون_جریان, اندازه_T بیرون_اندازه, اندازه_T *بیرون_اندازه_Processed)
{
  CProb *p = vs->Probs;
  اندازه_T nowPos = 0;
  Byte قبلی_Byte = 0;
  UInt32 pos_State_ماسک = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_ماسک = (1 << (vs->Properties.lp)) - 1;
  صحيح lc = vs->Properties.lc;


  صحيح state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  صحيح len = 0;
  ثابت Byte *میانگیر_2;
  ثابت Byte *میانگیر_Lim;
  UInt32 Range;
  UInt32 کد_2;

  *درون_اندازه_Processed = 0;
  *بیرون_اندازه_Processed = 0;

  {
    UInt32 i;
    UInt32 رقم_Probs = Literal + ((UInt32)LZMA_LIT_اندازه << (lc + vs->Properties.lp));
    لكل (i = 0; i < رقم_Probs; i++)
      p[i] = k_بیت_Model_مجموع >> 1;
  }
  
  RC_INIT(درون_جریان, درون_اندازه);


  طالما(nowPos < بیرون_اندازه)
  {
    CProb *prob;
    UInt32 bound;
    صحيح posState = (صحيح)(
        (nowPos 
        )
        & pos_State_ماسک);

    prob = p + Is_تطابق + (state << k_رقم_Pos_بیتها_Max) + posState;
    IfBit0(prob)
    {
      صحيح نماد_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_اندازه * 
        (((
        (nowPos 
        )
        & literal_Pos_ماسک) << lc) + (قبلی_Byte >> (8 - lc))));

      إذا (state >= k_رقم_Lit_حالتها)
      {
        صحيح تطابق_Byte;
        تطابق_Byte = بیرون_جریان[nowPos - rep0];
        افعل
        {
          صحيح بیت;
          CProb *probLit;
          تطابق_Byte <<= 1;
          بیت = (تطابق_Byte & 0x100);
          probLit = prob + 0x100 + بیت + نماد_2;
          RC_GET_BIT2(probLit, نماد_2, إذا (بیت != 0) اكسر, إذا (بیت == 0) اكسر)
        }
        طالما (نماد_2 < 0x100);
      }
      طالما (نماد_2 < 0x100)
      {
        CProb *probLit = prob + نماد_2;
        RC_GET_بیت(probLit, نماد_2)
      }
      قبلی_Byte = (Byte)نماد_2;

      بیرون_جریان[nowPos++] = قبلی_Byte;
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
        state = state < k_رقم_Lit_حالتها ? 0 : 3;
        prob = p + LenCoder;
      }
      وإلا
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_رقم_Pos_بیتها_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            إذا (nowPos == 0)
              أرجع LZMA_نتیجه_داده_ERROR;
            
            state = state < k_رقم_Lit_حالتها ? 9 : 11;
            قبلی_Byte = بیرون_جریان[nowPos - rep0];
            بیرون_جریان[nowPos++] = قبلی_Byte;

            استمر;
          }
          وإلا
          {
            UpdateBit1(prob);
          }
        }
        وإلا
        {
          UInt32 فاصله;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            فاصله = rep1;
          }
          وإلا 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              فاصله = rep2;
            }
            وإلا
            {
              UpdateBit1(prob);
              فاصله = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = فاصله;
        }
        state = state < k_رقم_Lit_حالتها ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        صحيح رقم_بیتها, جابه_جایی_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_کم + (posState << k_Len_رقم_کم_بیتها);
          جابه_جایی_2 = 0;
          رقم_بیتها = k_Len_رقم_کم_بیتها;
        }
        وإلا
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_رقم_Mid_بیتها);
            جابه_جایی_2 = k_Len_رقم_کم_Symbols;
            رقم_بیتها = k_Len_رقم_Mid_بیتها;
          }
          وإلا
          {
            UpdateBit1(probLen);
            probLen = prob + Len_بالا;
            جابه_جایی_2 = k_Len_رقم_کم_Symbols + k_Len_رقم_Mid_Symbols;
            رقم_بیتها = k_Len_رقم_بالا_بیتها;
          }
        }
        Range_Decoder_بیت_درخت_Decode(probLen, رقم_بیتها, len);
        len += جابه_جایی_2;
      }

      إذا (state < 4)
      {
        صحيح pos_جایگاه;
        state += k_رقم_Lit_حالتها;
        prob = p + Pos_جایگاه +
            ((len < k_رقم_Len_مقصد_Pos_حالتها ? len : k_رقم_Len_مقصد_Pos_حالتها - 1) << 
            k_رقم_Pos_جایگاه_بیتها);
        Range_Decoder_بیت_درخت_Decode(prob, k_رقم_Pos_جایگاه_بیتها, pos_جایگاه);
        إذا (pos_جایگاه >= k_شروع_Pos_Model_شاخص)
        {
          صحيح رقم_مستقیم_بیتها = ((pos_جایگاه >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_جایگاه & 1));
          إذا (pos_جایگاه < k_انتها_Pos_Model_شاخص)
          {
            rep0 <<= رقم_مستقیم_بیتها;
            prob = p + SpecPos + rep0 - pos_جایگاه - 1;
          }
          وإلا
          {
            رقم_مستقیم_بیتها -= k_رقم_Align_بیتها;
            افعل
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              إذا (کد_2 >= Range)
              {
                کد_2 -= Range;
                rep0 |= 1;
              }
            }
            طالما (--رقم_مستقیم_بیتها != 0);
            prob = p + Align;
            rep0 <<= k_رقم_Align_بیتها;
            رقم_مستقیم_بیتها = k_رقم_Align_بیتها;
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
            طالما(--رقم_مستقیم_بیتها != 0);
          }
        }
        وإلا
          rep0 = pos_جایگاه;
        إذا (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_جریان_Was_پایانیافته_Id;
          اكسر;
        }
      }

      len += k_تطابق_Min_Len;
      إذا (rep0 > nowPos)
        أرجع LZMA_نتیجه_داده_ERROR;


      افعل
      {
        قبلی_Byte = بیرون_جریان[nowPos - rep0];
        len--;
        بیرون_جریان[nowPos++] = قبلی_Byte;
      }
      طالما(len != 0 && nowPos < بیرون_اندازه);
    }
  }
  RC_NORMALIZE;


  *درون_اندازه_Processed = (اندازه_T)(میانگیر_2 - درون_جریان);
  *بیرون_اندازه_Processed = nowPos;
  أرجع LZMA_نتیجه_OK;
}
