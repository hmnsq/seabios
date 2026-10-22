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

#ಅನ್ತರ್ಭಾವ "lzmadecode.h"

#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_ತುದಿ_ಬಿಟ್ಟುಗಳು 24
#ವ್ಯಾಖ್ಯಾನಿಸು k_ತುದಿ_ಮೌಲ್ಯ ((UInt32)1 << k_ಅಂಕೆ_ತುದಿ_ಬಿಟ್ಟುಗಳು)

#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_ಬಿಟ್_Model_ಒಟ್ಟು_ಬಿಟ್ಟುಗಳು 11
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಬಿಟ್_Model_ಒಟ್ಟು (1 << k_ಅಂಕೆ_ಬಿಟ್_Model_ಒಟ್ಟು_ಬಿಟ್ಟುಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Move_ಬಿಟ್ಟುಗಳು 5

#ವ್ಯಾಖ್ಯಾನಿಸು RC_READ_BYTE (*ಬಫರ್_2++)

#ವ್ಯಾಖ್ಯಾನಿಸು RC_INIT2 ಸಂಕೇತ_3 = 0; Range = 0xFFFFFFFF; \
  { ಪೂರ್ಣಾಂಕ i; ಗಾಗಿ(i = 0; i < 5; i++) { RC_TEST; ಸಂಕೇತ_3 = (ಸಂಕೇತ_3 << 8) | RC_READ_BYTE; }}


#ವ್ಯಾಖ್ಯಾನಿಸು RC_TEST { ಆದರೆ (ಬಫರ್_2 == ಬಫರ್_Lim) ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_ದತ್ತಾಂಶ_ERROR; }

#ವ್ಯಾಖ್ಯಾನಿಸು RC_INIT(ಬಫರ್, ಬಫರ್_ಗಾತ್ರ) ಬಫರ್_2 = ಬಫರ್; ಬಫರ್_Lim = ಬಫರ್ + ಬಫರ್_ಗಾತ್ರ; RC_INIT2
 

#ವ್ಯಾಖ್ಯಾನಿಸು RC_NORMALIZE ಆದರೆ (Range < k_ತುದಿ_ಮೌಲ್ಯ) { RC_TEST; Range <<= 8; ಸಂಕೇತ_3 = (ಸಂಕೇತ_3 << 8) | RC_READ_BYTE; }

#ವ್ಯಾಖ್ಯಾನಿಸು IfBit0(p) RC_NORMALIZE; bound = (Range >> k_ಅಂಕೆ_ಬಿಟ್_Model_ಒಟ್ಟು_ಬಿಟ್ಟುಗಳು) * *(p); ಆದರೆ (ಸಂಕೇತ_3 < bound)
#ವ್ಯಾಖ್ಯಾನಿಸು UpdateBit0(p) Range = bound; *(p) += (k_ಬಿಟ್_Model_ಒಟ್ಟು - *(p)) >> k_ಅಂಕೆ_Move_ಬಿಟ್ಟುಗಳು;
#ವ್ಯಾಖ್ಯಾನಿಸು UpdateBit1(p) Range -= bound; ಸಂಕೇತ_3 -= bound; *(p) -= (*(p)) >> k_ಅಂಕೆ_Move_ಬಿಟ್ಟುಗಳು;

#ವ್ಯಾಖ್ಯಾನಿಸು RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } ಇಲ್ಲದಿದ್ದರೆ \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#ವ್ಯಾಖ್ಯಾನಿಸು RC_GET_ಬಿಟ್(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#ವ್ಯಾಖ್ಯಾನಿಸು Range_Decoder_ಬಿಟ್_ವೃಕ್ಷ_Decode(probs, ಅಂಕೆ_ಮಟ್ಟಗಳು, res) \
  { ಪೂರ್ಣಾಂಕ i = ಅಂಕೆ_ಮಟ್ಟಗಳು; res = 1; \
  ಮಾಡು { CProb *cp = probs + res; RC_GET_ಬಿಟ್(cp, res) } ಆಗ(--i != 0); \
  res -= (1 << ಅಂಕೆ_ಮಟ್ಟಗಳು); }


#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Pos_ಬಿಟ್ಟುಗಳು_Max 4
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Pos_ಸ್ಥಿತಿಗಳು_Max (1 << k_ಅಂಕೆ_Pos_ಬಿಟ್ಟುಗಳು_Max)

#ವ್ಯಾಖ್ಯಾನಿಸು k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು 3
#ವ್ಯಾಖ್ಯಾನಿಸು k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Symbols (1 << k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು k_Len_ಅಂಕೆ_Mid_ಬಿಟ್ಟುಗಳು 3
#ವ್ಯಾಖ್ಯಾನಿಸು k_Len_ಅಂಕೆ_Mid_Symbols (1 << k_Len_ಅಂಕೆ_Mid_ಬಿಟ್ಟುಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು k_Len_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು 8
#ವ್ಯಾಖ್ಯಾನಿಸು k_Len_ಅಂಕೆ_ಮೇಲಿನ_Symbols (1 << k_Len_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು)

#ವ್ಯಾಖ್ಯಾನಿಸು LenChoice 0
#ವ್ಯಾಖ್ಯಾನಿಸು LenChoice2 (LenChoice + 1)
#ವ್ಯಾಖ್ಯಾನಿಸು Len_ಕೆಳಗಿನ (LenChoice2 + 1)
#ವ್ಯಾಖ್ಯಾನಿಸು LenMid (Len_ಕೆಳಗಿನ + (k_ಅಂಕೆ_Pos_ಸ್ಥಿತಿಗಳು_Max << k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು))
#ವ್ಯಾಖ್ಯಾನಿಸು Len_ಮೇಲಿನ (LenMid + (k_ಅಂಕೆ_Pos_ಸ್ಥಿತಿಗಳು_Max << k_Len_ಅಂಕೆ_Mid_ಬಿಟ್ಟುಗಳು))
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Len_Probs (Len_ಮೇಲಿನ + k_Len_ಅಂಕೆ_ಮೇಲಿನ_Symbols) 


#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು 12
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Lit_ಸ್ಥಿತಿಗಳು 7

#ವ್ಯಾಖ್ಯಾನಿಸು k_ಆರಂಭ_Pos_Model_ಸೂಚ್ಯಂಕ 4
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಕೊನೆ_Pos_Model_ಸೂಚ್ಯಂಕ 14
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_ತುಂಬಿದ_Distances (1 << (k_ಕೊನೆ_Pos_Model_ಸೂಚ್ಯಂಕ >> 1))

#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Pos_ಸ್ಥಾನ_ಬಿಟ್ಟುಗಳು 6
#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Len_ಗಮ್ಯ_Pos_ಸ್ಥಿತಿಗಳು 4

#ವ್ಯಾಖ್ಯಾನಿಸು k_ಅಂಕೆ_Align_ಬಿಟ್ಟುಗಳು 4
#ವ್ಯಾಖ್ಯಾನಿಸು k_Align_ಕೋಷ್ಟಕ_ಗಾತ್ರ (1 << k_ಅಂಕೆ_Align_ಬಿಟ್ಟುಗಳು)

#ವ್ಯಾಖ್ಯಾನಿಸು k_ಹೊಂದಿಕೆ_Min_Len 2

#ವ್ಯಾಖ್ಯಾನಿಸು Is_ಹೊಂದಿಕೆ 0
#ವ್ಯಾಖ್ಯಾನಿಸು IsRep (Is_ಹೊಂದಿಕೆ + (k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು << k_ಅಂಕೆ_Pos_ಬಿಟ್ಟುಗಳು_Max))
#ವ್ಯಾಖ್ಯಾನಿಸು IsRepG0 (IsRep + k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು IsRepG1 (IsRepG0 + k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು IsRepG2 (IsRepG1 + k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು IsRep0Long (IsRepG2 + k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು)
#ವ್ಯಾಖ್ಯಾನಿಸು Pos_ಸ್ಥಾನ (IsRep0Long + (k_ಅಂಕೆ_ಸ್ಥಿತಿಗಳು << k_ಅಂಕೆ_Pos_ಬಿಟ್ಟುಗಳು_Max))
#ವ್ಯಾಖ್ಯಾನಿಸು SpecPos (Pos_ಸ್ಥಾನ + (k_ಅಂಕೆ_Len_ಗಮ್ಯ_Pos_ಸ್ಥಿತಿಗಳು << k_ಅಂಕೆ_Pos_ಸ್ಥಾನ_ಬಿಟ್ಟುಗಳು))
#ವ್ಯಾಖ್ಯಾನಿಸು Align (SpecPos + k_ಅಂಕೆ_ತುಂಬಿದ_Distances - k_ಕೊನೆ_Pos_Model_ಸೂಚ್ಯಂಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು LenCoder (Align + k_Align_ಕೋಷ್ಟಕ_ಗಾತ್ರ)
#ವ್ಯಾಖ್ಯಾನಿಸು RepLenCoder (LenCoder + k_ಅಂಕೆ_Len_Probs)
#ವ್ಯಾಖ್ಯಾನಿಸು Literal (RepLenCoder + k_ಅಂಕೆ_Len_Probs)

#ಆದರೆ Literal != LZMA_ತಳ_ಗಾತ್ರ
StopCompilingDueBUG
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ LzmaDecodeProperties(CLzmaProperties *propsRes, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *props_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ prop0;
  ಆದರೆ (ಗಾತ್ರ_2 < LZMA_PROPERTIES_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_ದತ್ತಾಂಶ_ERROR;
  prop0 = props_ದತ್ತಾಂಶ[0];
  ಆದರೆ (prop0 >= (9 * 5 * 5))
    ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_ದತ್ತಾಂಶ_ERROR;
  {
    ಗಾಗಿ (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    ಗಾಗಿ (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_OK;
}

#ವ್ಯಾಖ್ಯಾನಿಸು k_Lzma_ಹರಿವು_Was_ಪೂರ್ಣಗೊಂಡ_Id (-1)

ಪೂರ್ಣಾಂಕ LzmaDecode(CLzmaDecoderState *vs,
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ_ಹರಿವು, ಗಾತ್ರ_T ಒಳ_ಗಾತ್ರ, ಗಾತ್ರ_T *ಒಳ_ಗಾತ್ರ_Processed,
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಹೊರಹೋಗು_ಹರಿವು, ಗಾತ್ರ_T ಹೊರಹೋಗು_ಗಾತ್ರ, ಗಾತ್ರ_T *ಹೊರಹೋಗು_ಗಾತ್ರ_Processed)
{
  CProb *p = vs->Probs;
  ಗಾತ್ರ_T nowPos = 0;
  Byte ಹಿಂದಿನ_Byte = 0;
  UInt32 pos_State_ಮುಖವಾಡ = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_ಮುಖವಾಡ = (1 << (vs->Properties.lp)) - 1;
  ಪೂರ್ಣಾಂಕ lc = vs->Properties.lc;


  ಪೂರ್ಣಾಂಕ state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  ಪೂರ್ಣಾಂಕ len = 0;
  ಸ್ಥಿರಾಂಕ Byte *ಬಫರ್_2;
  ಸ್ಥಿರಾಂಕ Byte *ಬಫರ್_Lim;
  UInt32 Range;
  UInt32 ಸಂಕೇತ_3;

  *ಒಳ_ಗಾತ್ರ_Processed = 0;
  *ಹೊರಹೋಗು_ಗಾತ್ರ_Processed = 0;

  {
    UInt32 i;
    UInt32 ಅಂಕೆ_Probs = Literal + ((UInt32)LZMA_LIT_ಗಾತ್ರ << (lc + vs->Properties.lp));
    ಗಾಗಿ (i = 0; i < ಅಂಕೆ_Probs; i++)
      p[i] = k_ಬಿಟ್_Model_ಒಟ್ಟು >> 1;
  }
  
  RC_INIT(ಒಳ_ಹರಿವು, ಒಳ_ಗಾತ್ರ);


  ಆಗ(nowPos < ಹೊರಹೋಗು_ಗಾತ್ರ)
  {
    CProb *prob;
    UInt32 bound;
    ಪೂರ್ಣಾಂಕ posState = (ಪೂರ್ಣಾಂಕ)(
        (nowPos 
        )
        & pos_State_ಮುಖವಾಡ);

    prob = p + Is_ಹೊಂದಿಕೆ + (state << k_ಅಂಕೆ_Pos_ಬಿಟ್ಟುಗಳು_Max) + posState;
    IfBit0(prob)
    {
      ಪೂರ್ಣಾಂಕ ಚಿಹ್ನೆ_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_ಗಾತ್ರ * 
        (((
        (nowPos 
        )
        & literal_Pos_ಮುಖವಾಡ) << lc) + (ಹಿಂದಿನ_Byte >> (8 - lc))));

      ಆದರೆ (state >= k_ಅಂಕೆ_Lit_ಸ್ಥಿತಿಗಳು)
      {
        ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ_Byte;
        ಹೊಂದಿಕೆ_Byte = ಹೊರಹೋಗು_ಹರಿವು[nowPos - rep0];
        ಮಾಡು
        {
          ಪೂರ್ಣಾಂಕ ಬಿಟ್;
          CProb *probLit;
          ಹೊಂದಿಕೆ_Byte <<= 1;
          ಬಿಟ್ = (ಹೊಂದಿಕೆ_Byte & 0x100);
          probLit = prob + 0x100 + ಬಿಟ್ + ಚಿಹ್ನೆ_2;
          RC_GET_BIT2(probLit, ಚಿಹ್ನೆ_2, ಆದರೆ (ಬಿಟ್ != 0) ನಿಲ್ಲಿಸು, ಆದರೆ (ಬಿಟ್ == 0) ನಿಲ್ಲಿಸು)
        }
        ಆಗ (ಚಿಹ್ನೆ_2 < 0x100);
      }
      ಆಗ (ಚಿಹ್ನೆ_2 < 0x100)
      {
        CProb *probLit = prob + ಚಿಹ್ನೆ_2;
        RC_GET_ಬಿಟ್(probLit, ಚಿಹ್ನೆ_2)
      }
      ಹಿಂದಿನ_Byte = (Byte)ಚಿಹ್ನೆ_2;

      ಹೊರಹೋಗು_ಹರಿವು[nowPos++] = ಹಿಂದಿನ_Byte;
      ಆದರೆ (state < 4) state = 0;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state < 10) state -= 3;
      ಇಲ್ಲದಿದ್ದರೆ state -= 6;
    }
    ಇಲ್ಲದಿದ್ದರೆ             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_ಅಂಕೆ_Lit_ಸ್ಥಿತಿಗಳು ? 0 : 3;
        prob = p + LenCoder;
      }
      ಇಲ್ಲದಿದ್ದರೆ
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_ಅಂಕೆ_Pos_ಬಿಟ್ಟುಗಳು_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            ಆದರೆ (nowPos == 0)
              ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_ದತ್ತಾಂಶ_ERROR;
            
            state = state < k_ಅಂಕೆ_Lit_ಸ್ಥಿತಿಗಳು ? 9 : 11;
            ಹಿಂದಿನ_Byte = ಹೊರಹೋಗು_ಹರಿವು[nowPos - rep0];
            ಹೊರಹೋಗು_ಹರಿವು[nowPos++] = ಹಿಂದಿನ_Byte;

            ಮುಂದುವರಿ;
          }
          ಇಲ್ಲದಿದ್ದರೆ
          {
            UpdateBit1(prob);
          }
        }
        ಇಲ್ಲದಿದ್ದರೆ
        {
          UInt32 ದೂರ;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            ದೂರ = rep1;
          }
          ಇಲ್ಲದಿದ್ದರೆ 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              ದೂರ = rep2;
            }
            ಇಲ್ಲದಿದ್ದರೆ
            {
              UpdateBit1(prob);
              ದೂರ = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = ದೂರ;
        }
        state = state < k_ಅಂಕೆ_Lit_ಸ್ಥಿತಿಗಳು ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        ಪೂರ್ಣಾಂಕ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, ಸ್ಥಾನಾಂತರ_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_ಕೆಳಗಿನ + (posState << k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು);
          ಸ್ಥಾನಾಂತರ_2 = 0;
          ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು;
        }
        ಇಲ್ಲದಿದ್ದರೆ
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_ಅಂಕೆ_Mid_ಬಿಟ್ಟುಗಳು);
            ಸ್ಥಾನಾಂತರ_2 = k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Symbols;
            ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = k_Len_ಅಂಕೆ_Mid_ಬಿಟ್ಟುಗಳು;
          }
          ಇಲ್ಲದಿದ್ದರೆ
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ಮೇಲಿನ;
            ಸ್ಥಾನಾಂತರ_2 = k_Len_ಅಂಕೆ_ಕೆಳಗಿನ_Symbols + k_Len_ಅಂಕೆ_Mid_Symbols;
            ಅಂಕೆ_ಬಿಟ್ಟುಗಳು = k_Len_ಅಂಕೆ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು;
          }
        }
        Range_Decoder_ಬಿಟ್_ವೃಕ್ಷ_Decode(probLen, ಅಂಕೆ_ಬಿಟ್ಟುಗಳು, len);
        len += ಸ್ಥಾನಾಂತರ_2;
      }

      ಆದರೆ (state < 4)
      {
        ಪೂರ್ಣಾಂಕ pos_ಸ್ಥಾನ;
        state += k_ಅಂಕೆ_Lit_ಸ್ಥಿತಿಗಳು;
        prob = p + Pos_ಸ್ಥಾನ +
            ((len < k_ಅಂಕೆ_Len_ಗಮ್ಯ_Pos_ಸ್ಥಿತಿಗಳು ? len : k_ಅಂಕೆ_Len_ಗಮ್ಯ_Pos_ಸ್ಥಿತಿಗಳು - 1) << 
            k_ಅಂಕೆ_Pos_ಸ್ಥಾನ_ಬಿಟ್ಟುಗಳು);
        Range_Decoder_ಬಿಟ್_ವೃಕ್ಷ_Decode(prob, k_ಅಂಕೆ_Pos_ಸ್ಥಾನ_ಬಿಟ್ಟುಗಳು, pos_ಸ್ಥಾನ);
        ಆದರೆ (pos_ಸ್ಥಾನ >= k_ಆರಂಭ_Pos_Model_ಸೂಚ್ಯಂಕ)
        {
          ಪೂರ್ಣಾಂಕ ಅಂಕೆ_ನೇರ_ಬಿಟ್ಟುಗಳು = ((pos_ಸ್ಥಾನ >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_ಸ್ಥಾನ & 1));
          ಆದರೆ (pos_ಸ್ಥಾನ < k_ಕೊನೆ_Pos_Model_ಸೂಚ್ಯಂಕ)
          {
            rep0 <<= ಅಂಕೆ_ನೇರ_ಬಿಟ್ಟುಗಳು;
            prob = p + SpecPos + rep0 - pos_ಸ್ಥಾನ - 1;
          }
          ಇಲ್ಲದಿದ್ದರೆ
          {
            ಅಂಕೆ_ನೇರ_ಬಿಟ್ಟುಗಳು -= k_ಅಂಕೆ_Align_ಬಿಟ್ಟುಗಳು;
            ಮಾಡು
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              ಆದರೆ (ಸಂಕೇತ_3 >= Range)
              {
                ಸಂಕೇತ_3 -= Range;
                rep0 |= 1;
              }
            }
            ಆಗ (--ಅಂಕೆ_ನೇರ_ಬಿಟ್ಟುಗಳು != 0);
            prob = p + Align;
            rep0 <<= k_ಅಂಕೆ_Align_ಬಿಟ್ಟುಗಳು;
            ಅಂಕೆ_ನೇರ_ಬಿಟ್ಟುಗಳು = k_ಅಂಕೆ_Align_ಬಿಟ್ಟುಗಳು;
          }
          {
            ಪೂರ್ಣಾಂಕ i = 1;
            ಪೂರ್ಣಾಂಕ mi = 1;
            ಮಾಡು
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            ಆಗ(--ಅಂಕೆ_ನೇರ_ಬಿಟ್ಟುಗಳು != 0);
          }
        }
        ಇಲ್ಲದಿದ್ದರೆ
          rep0 = pos_ಸ್ಥಾನ;
        ಆದರೆ (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_ಹರಿವು_Was_ಪೂರ್ಣಗೊಂಡ_Id;
          ನಿಲ್ಲಿಸು;
        }
      }

      len += k_ಹೊಂದಿಕೆ_Min_Len;
      ಆದರೆ (rep0 > nowPos)
        ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_ದತ್ತಾಂಶ_ERROR;


      ಮಾಡು
      {
        ಹಿಂದಿನ_Byte = ಹೊರಹೋಗು_ಹರಿವು[nowPos - rep0];
        len--;
        ಹೊರಹೋಗು_ಹರಿವು[nowPos++] = ಹಿಂದಿನ_Byte;
      }
      ಆಗ(len != 0 && nowPos < ಹೊರಹೋಗು_ಗಾತ್ರ);
    }
  }
  RC_NORMALIZE;


  *ಒಳ_ಗಾತ್ರ_Processed = (ಗಾತ್ರ_T)(ಬಫರ್_2 - ಒಳ_ಹರಿವು);
  *ಹೊರಹೋಗು_ಗಾತ್ರ_Processed = nowPos;
  ಹಿಂತಿರುಗಿ LZMA_ಫಲಿತಾಂಶ_OK;
}
