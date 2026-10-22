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

#ਅਨ੍ਤਰ੍ਭਾਵ "lzmadecode.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_ਸਿਖਰ_ਬਿੱਟ 24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਸਿਖਰ_ਮੁੱਲ ((UInt32)1 << k_ਅੰਕ_ਸਿਖਰ_ਬਿੱਟ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_ਦੋਆਧਾਰੀ_ਅੰਕ_Model_ਕੁੱਲ_ਬਿੱਟ 11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਦੋਆਧਾਰੀ_ਅੰਕ_Model_ਕੁੱਲ (1 << k_ਅੰਕ_ਦੋਆਧਾਰੀ_ਅੰਕ_Model_ਕੁੱਲ_ਬਿੱਟ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Move_ਬਿੱਟ 5

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_READ_BYTE (*ਬਫਰ_2++)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_INIT2 ਕੋਡ_2 = 0; Range = 0xFFFFFFFF; \
  { ਪੂਰਨ i; ਲਈ(i = 0; i < 5; i++) { RC_TEST; ਕੋਡ_2 = (ਕੋਡ_2 << 8) | RC_READ_BYTE; }}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_TEST { ਜੇ (ਬਫਰ_2 == ਬਫਰ_Lim) ਵਾਪਸ LZMA_ਨਤੀਜਾ_ਅੰਕੜੇ_ERROR; }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_INIT(ਬਫਰ, ਬਫਰ_ਮਾਪ) ਬਫਰ_2 = ਬਫਰ; ਬਫਰ_Lim = ਬਫਰ + ਬਫਰ_ਮਾਪ; RC_INIT2
 

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_NORMALIZE ਜੇ (Range < k_ਸਿਖਰ_ਮੁੱਲ) { RC_TEST; Range <<= 8; ਕੋਡ_2 = (ਕੋਡ_2 << 8) | RC_READ_BYTE; }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IfBit0(p) RC_NORMALIZE; bound = (Range >> k_ਅੰਕ_ਦੋਆਧਾਰੀ_ਅੰਕ_Model_ਕੁੱਲ_ਬਿੱਟ) * *(p); ਜੇ (ਕੋਡ_2 < bound)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UpdateBit0(p) Range = bound; *(p) += (k_ਦੋਆਧਾਰੀ_ਅੰਕ_Model_ਕੁੱਲ - *(p)) >> k_ਅੰਕ_Move_ਬਿੱਟ;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UpdateBit1(p) Range -= bound; ਕੋਡ_2 -= bound; *(p) -= (*(p)) >> k_ਅੰਕ_Move_ਬਿੱਟ;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } ਨਹੀਂਤਾਂ \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_GET_ਦੋਆਧਾਰੀ_ਅੰਕ(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Range_Decoder_ਦੋਆਧਾਰੀ_ਅੰਕ_ਰੁੱਖ_Decode(probs, ਅੰਕ_ਪੱਧਰ, res) \
  { ਪੂਰਨ i = ਅੰਕ_ਪੱਧਰ; res = 1; \
  ਕਰੋ { CProb *cp = probs + res; RC_GET_ਦੋਆਧਾਰੀ_ਅੰਕ(cp, res) } ਜਦੋਂ(--i != 0); \
  res -= (1 << ਅੰਕ_ਪੱਧਰ); }


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Pos_ਬਿੱਟ_Max 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Pos_ਹਾਲਤਾਂ_Max (1 << k_ਅੰਕ_Pos_ਬਿੱਟ_Max)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Len_ਅੰਕ_ਨੀਵਾਂ_ਬਿੱਟ 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Len_ਅੰਕ_ਨੀਵਾਂ_Symbols (1 << k_Len_ਅੰਕ_ਨੀਵਾਂ_ਬਿੱਟ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Len_ਅੰਕ_Mid_ਬਿੱਟ 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Len_ਅੰਕ_Mid_Symbols (1 << k_Len_ਅੰਕ_Mid_ਬਿੱਟ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Len_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Len_ਅੰਕ_ਉੱਚਾ_Symbols (1 << k_Len_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LenChoice 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LenChoice2 (LenChoice + 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Len_ਨੀਵਾਂ (LenChoice2 + 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LenMid (Len_ਨੀਵਾਂ + (k_ਅੰਕ_Pos_ਹਾਲਤਾਂ_Max << k_Len_ਅੰਕ_ਨੀਵਾਂ_ਬਿੱਟ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Len_ਉੱਚਾ (LenMid + (k_ਅੰਕ_Pos_ਹਾਲਤਾਂ_Max << k_Len_ਅੰਕ_Mid_ਬਿੱਟ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Len_Probs (Len_ਉੱਚਾ + k_Len_ਅੰਕ_ਉੱਚਾ_Symbols) 


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_ਹਾਲਤਾਂ 12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Lit_ਹਾਲਤਾਂ 7

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਸ਼ੁਰੂ_Pos_Model_ਸੂਚਕਾਂਕ 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਸਿਰਾ_Pos_Model_ਸੂਚਕਾਂਕ 14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_ਭਰਿਆ_Distances (1 << (k_ਸਿਰਾ_Pos_Model_ਸੂਚਕਾਂਕ >> 1))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Pos_ਥਾਂ_ਬਿੱਟ 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Len_ਮੰਜ਼ਿਲ_Pos_ਹਾਲਤਾਂ 4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਅੰਕ_Align_ਬਿੱਟ 4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Align_ਸਾਰਣੀ_ਮਾਪ (1 << k_ਅੰਕ_Align_ਬਿੱਟ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_ਮੇਲ_Min_Len 2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Is_ਮੇਲ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsRep (Is_ਮੇਲ + (k_ਅੰਕ_ਹਾਲਤਾਂ << k_ਅੰਕ_Pos_ਬਿੱਟ_Max))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsRepG0 (IsRep + k_ਅੰਕ_ਹਾਲਤਾਂ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsRepG1 (IsRepG0 + k_ਅੰਕ_ਹਾਲਤਾਂ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsRepG2 (IsRepG1 + k_ਅੰਕ_ਹਾਲਤਾਂ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsRep0Long (IsRepG2 + k_ਅੰਕ_ਹਾਲਤਾਂ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Pos_ਥਾਂ (IsRep0Long + (k_ਅੰਕ_ਹਾਲਤਾਂ << k_ਅੰਕ_Pos_ਬਿੱਟ_Max))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SpecPos (Pos_ਥਾਂ + (k_ਅੰਕ_Len_ਮੰਜ਼ਿਲ_Pos_ਹਾਲਤਾਂ << k_ਅੰਕ_Pos_ਥਾਂ_ਬਿੱਟ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Align (SpecPos + k_ਅੰਕ_ਭਰਿਆ_Distances - k_ਸਿਰਾ_Pos_Model_ਸੂਚਕਾਂਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LenCoder (Align + k_Align_ਸਾਰਣੀ_ਮਾਪ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RepLenCoder (LenCoder + k_ਅੰਕ_Len_Probs)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Literal (RepLenCoder + k_ਅੰਕ_Len_Probs)

#ਜੇ Literal != LZMA_ਅਧਾਰ_ਮਾਪ
StopCompilingDueBUG
#ਜੇ_ਅੰਤ

ਪੂਰਨ LzmaDecodeProperties(CLzmaProperties *propsRes, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *props_ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ prop0;
  ਜੇ (ਮਾਪ < LZMA_PROPERTIES_ਮਾਪ)
    ਵਾਪਸ LZMA_ਨਤੀਜਾ_ਅੰਕੜੇ_ERROR;
  prop0 = props_ਅੰਕੜੇ[0];
  ਜੇ (prop0 >= (9 * 5 * 5))
    ਵਾਪਸ LZMA_ਨਤੀਜਾ_ਅੰਕੜੇ_ERROR;
  {
    ਲਈ (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    ਲਈ (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  ਵਾਪਸ LZMA_ਨਤੀਜਾ_OK;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ k_Lzma_ਧਾਰਾ_Was_ਮੁਕੰਮਲ_Id (-1)

ਪੂਰਨ LzmaDecode(CLzmaDecoderState *vs,
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ_ਧਾਰਾ, ਮਾਪ_T ਵਿੱਚ_ਮਾਪ, ਮਾਪ_T *ਵਿੱਚ_ਮਾਪ_Processed,
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਾਹਰਲਾ_ਧਾਰਾ, ਮਾਪ_T ਬਾਹਰਲਾ_ਮਾਪ, ਮਾਪ_T *ਬਾਹਰਲਾ_ਮਾਪ_Processed)
{
  CProb *p = vs->Probs;
  ਮਾਪ_T nowPos = 0;
  Byte ਪਿਛਲਾ_Byte = 0;
  UInt32 pos_State_ਨਕਾਬ = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_ਨਕਾਬ = (1 << (vs->Properties.lp)) - 1;
  ਪੂਰਨ lc = vs->Properties.lc;


  ਪੂਰਨ state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  ਪੂਰਨ len = 0;
  ਅਚਲ Byte *ਬਫਰ_2;
  ਅਚਲ Byte *ਬਫਰ_Lim;
  UInt32 Range;
  UInt32 ਕੋਡ_2;

  *ਵਿੱਚ_ਮਾਪ_Processed = 0;
  *ਬਾਹਰਲਾ_ਮਾਪ_Processed = 0;

  {
    UInt32 i;
    UInt32 ਅੰਕ_Probs = Literal + ((UInt32)LZMA_LIT_ਮਾਪ << (lc + vs->Properties.lp));
    ਲਈ (i = 0; i < ਅੰਕ_Probs; i++)
      p[i] = k_ਦੋਆਧਾਰੀ_ਅੰਕ_Model_ਕੁੱਲ >> 1;
  }
  
  RC_INIT(ਵਿੱਚ_ਧਾਰਾ, ਵਿੱਚ_ਮਾਪ);


  ਜਦੋਂ(nowPos < ਬਾਹਰਲਾ_ਮਾਪ)
  {
    CProb *prob;
    UInt32 bound;
    ਪੂਰਨ posState = (ਪੂਰਨ)(
        (nowPos 
        )
        & pos_State_ਨਕਾਬ);

    prob = p + Is_ਮੇਲ + (state << k_ਅੰਕ_Pos_ਬਿੱਟ_Max) + posState;
    IfBit0(prob)
    {
      ਪੂਰਨ ਚਿੰਨ੍ਹ_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_ਮਾਪ * 
        (((
        (nowPos 
        )
        & literal_Pos_ਨਕਾਬ) << lc) + (ਪਿਛਲਾ_Byte >> (8 - lc))));

      ਜੇ (state >= k_ਅੰਕ_Lit_ਹਾਲਤਾਂ)
      {
        ਪੂਰਨ ਮੇਲ_Byte;
        ਮੇਲ_Byte = ਬਾਹਰਲਾ_ਧਾਰਾ[nowPos - rep0];
        ਕਰੋ
        {
          ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ;
          CProb *probLit;
          ਮੇਲ_Byte <<= 1;
          ਦੋਆਧਾਰੀ_ਅੰਕ = (ਮੇਲ_Byte & 0x100);
          probLit = prob + 0x100 + ਦੋਆਧਾਰੀ_ਅੰਕ + ਚਿੰਨ੍ਹ_2;
          RC_GET_BIT2(probLit, ਚਿੰਨ੍ਹ_2, ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ != 0) ਰੋਕੋ, ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ == 0) ਰੋਕੋ)
        }
        ਜਦੋਂ (ਚਿੰਨ੍ਹ_2 < 0x100);
      }
      ਜਦੋਂ (ਚਿੰਨ੍ਹ_2 < 0x100)
      {
        CProb *probLit = prob + ਚਿੰਨ੍ਹ_2;
        RC_GET_ਦੋਆਧਾਰੀ_ਅੰਕ(probLit, ਚਿੰਨ੍ਹ_2)
      }
      ਪਿਛਲਾ_Byte = (Byte)ਚਿੰਨ੍ਹ_2;

      ਬਾਹਰਲਾ_ਧਾਰਾ[nowPos++] = ਪਿਛਲਾ_Byte;
      ਜੇ (state < 4) state = 0;
      ਨਹੀਂਤਾਂ ਜੇ (state < 10) state -= 3;
      ਨਹੀਂਤਾਂ state -= 6;
    }
    ਨਹੀਂਤਾਂ             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_ਅੰਕ_Lit_ਹਾਲਤਾਂ ? 0 : 3;
        prob = p + LenCoder;
      }
      ਨਹੀਂਤਾਂ
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_ਅੰਕ_Pos_ਬਿੱਟ_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            ਜੇ (nowPos == 0)
              ਵਾਪਸ LZMA_ਨਤੀਜਾ_ਅੰਕੜੇ_ERROR;
            
            state = state < k_ਅੰਕ_Lit_ਹਾਲਤਾਂ ? 9 : 11;
            ਪਿਛਲਾ_Byte = ਬਾਹਰਲਾ_ਧਾਰਾ[nowPos - rep0];
            ਬਾਹਰਲਾ_ਧਾਰਾ[nowPos++] = ਪਿਛਲਾ_Byte;

            ਜਾਰੀ;
          }
          ਨਹੀਂਤਾਂ
          {
            UpdateBit1(prob);
          }
        }
        ਨਹੀਂਤਾਂ
        {
          UInt32 ਦੂਰੀ;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            ਦੂਰੀ = rep1;
          }
          ਨਹੀਂਤਾਂ 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              ਦੂਰੀ = rep2;
            }
            ਨਹੀਂਤਾਂ
            {
              UpdateBit1(prob);
              ਦੂਰੀ = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = ਦੂਰੀ;
        }
        state = state < k_ਅੰਕ_Lit_ਹਾਲਤਾਂ ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        ਪੂਰਨ ਅੰਕ_ਬਿੱਟ, ਵਿੱਥ_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_ਨੀਵਾਂ + (posState << k_Len_ਅੰਕ_ਨੀਵਾਂ_ਬਿੱਟ);
          ਵਿੱਥ_2 = 0;
          ਅੰਕ_ਬਿੱਟ = k_Len_ਅੰਕ_ਨੀਵਾਂ_ਬਿੱਟ;
        }
        ਨਹੀਂਤਾਂ
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_ਅੰਕ_Mid_ਬਿੱਟ);
            ਵਿੱਥ_2 = k_Len_ਅੰਕ_ਨੀਵਾਂ_Symbols;
            ਅੰਕ_ਬਿੱਟ = k_Len_ਅੰਕ_Mid_ਬਿੱਟ;
          }
          ਨਹੀਂਤਾਂ
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ਉੱਚਾ;
            ਵਿੱਥ_2 = k_Len_ਅੰਕ_ਨੀਵਾਂ_Symbols + k_Len_ਅੰਕ_Mid_Symbols;
            ਅੰਕ_ਬਿੱਟ = k_Len_ਅੰਕ_ਉੱਚਾ_ਬਿੱਟ;
          }
        }
        Range_Decoder_ਦੋਆਧਾਰੀ_ਅੰਕ_ਰੁੱਖ_Decode(probLen, ਅੰਕ_ਬਿੱਟ, len);
        len += ਵਿੱਥ_2;
      }

      ਜੇ (state < 4)
      {
        ਪੂਰਨ pos_ਥਾਂ;
        state += k_ਅੰਕ_Lit_ਹਾਲਤਾਂ;
        prob = p + Pos_ਥਾਂ +
            ((len < k_ਅੰਕ_Len_ਮੰਜ਼ਿਲ_Pos_ਹਾਲਤਾਂ ? len : k_ਅੰਕ_Len_ਮੰਜ਼ਿਲ_Pos_ਹਾਲਤਾਂ - 1) << 
            k_ਅੰਕ_Pos_ਥਾਂ_ਬਿੱਟ);
        Range_Decoder_ਦੋਆਧਾਰੀ_ਅੰਕ_ਰੁੱਖ_Decode(prob, k_ਅੰਕ_Pos_ਥਾਂ_ਬਿੱਟ, pos_ਥਾਂ);
        ਜੇ (pos_ਥਾਂ >= k_ਸ਼ੁਰੂ_Pos_Model_ਸੂਚਕਾਂਕ)
        {
          ਪੂਰਨ ਅੰਕ_ਸਿੱਧਾ_ਬਿੱਟ = ((pos_ਥਾਂ >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_ਥਾਂ & 1));
          ਜੇ (pos_ਥਾਂ < k_ਸਿਰਾ_Pos_Model_ਸੂਚਕਾਂਕ)
          {
            rep0 <<= ਅੰਕ_ਸਿੱਧਾ_ਬਿੱਟ;
            prob = p + SpecPos + rep0 - pos_ਥਾਂ - 1;
          }
          ਨਹੀਂਤਾਂ
          {
            ਅੰਕ_ਸਿੱਧਾ_ਬਿੱਟ -= k_ਅੰਕ_Align_ਬਿੱਟ;
            ਕਰੋ
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              ਜੇ (ਕੋਡ_2 >= Range)
              {
                ਕੋਡ_2 -= Range;
                rep0 |= 1;
              }
            }
            ਜਦੋਂ (--ਅੰਕ_ਸਿੱਧਾ_ਬਿੱਟ != 0);
            prob = p + Align;
            rep0 <<= k_ਅੰਕ_Align_ਬਿੱਟ;
            ਅੰਕ_ਸਿੱਧਾ_ਬਿੱਟ = k_ਅੰਕ_Align_ਬਿੱਟ;
          }
          {
            ਪੂਰਨ i = 1;
            ਪੂਰਨ mi = 1;
            ਕਰੋ
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            ਜਦੋਂ(--ਅੰਕ_ਸਿੱਧਾ_ਬਿੱਟ != 0);
          }
        }
        ਨਹੀਂਤਾਂ
          rep0 = pos_ਥਾਂ;
        ਜੇ (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_ਧਾਰਾ_Was_ਮੁਕੰਮਲ_Id;
          ਰੋਕੋ;
        }
      }

      len += k_ਮੇਲ_Min_Len;
      ਜੇ (rep0 > nowPos)
        ਵਾਪਸ LZMA_ਨਤੀਜਾ_ਅੰਕੜੇ_ERROR;


      ਕਰੋ
      {
        ਪਿਛਲਾ_Byte = ਬਾਹਰਲਾ_ਧਾਰਾ[nowPos - rep0];
        len--;
        ਬਾਹਰਲਾ_ਧਾਰਾ[nowPos++] = ਪਿਛਲਾ_Byte;
      }
      ਜਦੋਂ(len != 0 && nowPos < ਬਾਹਰਲਾ_ਮਾਪ);
    }
  }
  RC_NORMALIZE;


  *ਵਿੱਚ_ਮਾਪ_Processed = (ਮਾਪ_T)(ਬਫਰ_2 - ਵਿੱਚ_ਧਾਰਾ);
  *ਬਾਹਰਲਾ_ਮਾਪ_Processed = nowPos;
  ਵਾਪਸ LZMA_ਨਤੀਜਾ_OK;
}
