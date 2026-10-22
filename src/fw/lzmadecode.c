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

#समावेश "lzmadecode.h"

#घोषणा k_अंक_शीर्ष_Bits 24
#घोषणा k_शीर्ष_मूल्य ((UInt32)1 << k_अंक_शीर्ष_Bits)

#घोषणा k_अंक_Bit_Model_योग_Bits 11
#घोषणा k_Bit_Model_योग (1 << k_अंक_Bit_Model_योग_Bits)
#घोषणा k_अंक_Move_Bits 5

#घोषणा RC_READ_BYTE (*बफर_2++)

#घोषणा RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { पूर्णांक i; क्रम(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#घोषणा RC_TEST { अगर (बफर_2 == बफर_Lim) वापस LZMA_परिणाम_तथ्य_ERROR; }

#घोषणा RC_INIT(बफर, बफर_आकार) बफर_2 = बफर; बफर_Lim = बफर + बफर_आकार; RC_INIT2
 

#घोषणा RC_NORMALIZE अगर (Range < k_शीर्ष_मूल्य) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#घोषणा IfBit0(p) RC_NORMALIZE; bound = (Range >> k_अंक_Bit_Model_योग_Bits) * *(p); अगर (Code < bound)
#घोषणा UpdateBit0(p) Range = bound; *(p) += (k_Bit_Model_योग - *(p)) >> k_अंक_Move_Bits;
#घोषणा UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_अंक_Move_Bits;

#घोषणा RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } अन्यथा \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#घोषणा RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#घोषणा Range_Decoder_Bit_वृक्ष_Decode(probs, अंक_Levels, res) \
  { पूर्णांक i = अंक_Levels; res = 1; \
  करो { CProb *cp = probs + res; RC_GET_BIT(cp, res) } जबतक(--i != 0); \
  res -= (1 << अंक_Levels); }


#घोषणा k_अंक_Pos_Bits_Max 4
#घोषणा k_अंक_Pos_States_Max (1 << k_अंक_Pos_Bits_Max)

#घोषणा k_Len_अंक_निचला_Bits 3
#घोषणा k_Len_अंक_निचला_Symbols (1 << k_Len_अंक_निचला_Bits)
#घोषणा k_Len_अंक_Mid_Bits 3
#घोषणा k_Len_अंक_Mid_Symbols (1 << k_Len_अंक_Mid_Bits)
#घोषणा k_Len_अंक_ऊपरी_Bits 8
#घोषणा k_Len_अंक_ऊपरी_Symbols (1 << k_Len_अंक_ऊपरी_Bits)

#घोषणा LenChoice 0
#घोषणा LenChoice2 (LenChoice + 1)
#घोषणा Len_निचला (LenChoice2 + 1)
#घोषणा LenMid (Len_निचला + (k_अंक_Pos_States_Max << k_Len_अंक_निचला_Bits))
#घोषणा Len_ऊपरी (LenMid + (k_अंक_Pos_States_Max << k_Len_अंक_Mid_Bits))
#घोषणा k_अंक_Len_Probs (Len_ऊपरी + k_Len_अंक_ऊपरी_Symbols) 


#घोषणा k_अंक_States 12
#घोषणा k_अंक_Lit_States 7

#घोषणा k_Start_Pos_Model_सूचकांक 4
#घोषणा k_End_Pos_Model_सूचकांक 14
#घोषणा k_अंक_Full_Distances (1 << (k_End_Pos_Model_सूचकांक >> 1))

#घोषणा k_अंक_Pos_Slot_Bits 6
#घोषणा k_अंक_Len_To_Pos_States 4

#घोषणा k_अंक_Align_Bits 4
#घोषणा k_Align_Table_आकार (1 << k_अंक_Align_Bits)

#घोषणा kMatchMinLen 2

#घोषणा IsMatch 0
#घोषणा IsRep (IsMatch + (k_अंक_States << k_अंक_Pos_Bits_Max))
#घोषणा IsRepG0 (IsRep + k_अंक_States)
#घोषणा IsRepG1 (IsRepG0 + k_अंक_States)
#घोषणा IsRepG2 (IsRepG1 + k_अंक_States)
#घोषणा IsRep0Long (IsRepG2 + k_अंक_States)
#घोषणा PosSlot (IsRep0Long + (k_अंक_States << k_अंक_Pos_Bits_Max))
#घोषणा SpecPos (PosSlot + (k_अंक_Len_To_Pos_States << k_अंक_Pos_Slot_Bits))
#घोषणा Align (SpecPos + k_अंक_Full_Distances - k_End_Pos_Model_सूचकांक)
#घोषणा LenCoder (Align + k_Align_Table_आकार)
#घोषणा RepLenCoder (LenCoder + k_अंक_Len_Probs)
#घोषणा Literal (RepLenCoder + k_अंक_Len_Probs)

#अगर Literal != LZMA_BASE_आकार
StopCompilingDueBUG
#पूर्ण_अगर

पूर्णांक LzmaDecodeProperties(CLzmaProperties *propsRes, स्थिर अचिन्हित अक्षर *props_तथ्य, पूर्णांक आकार)
{
  अचिन्हित अक्षर prop0;
  अगर (आकार < LZMA_PROPERTIES_आकार)
    वापस LZMA_परिणाम_तथ्य_ERROR;
  prop0 = props_तथ्य[0];
  अगर (prop0 >= (9 * 5 * 5))
    वापस LZMA_परिणाम_तथ्य_ERROR;
  {
    क्रम (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    क्रम (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  वापस LZMA_परिणाम_OK;
}

#घोषणा kLzmaStreamWasFinishedId (-1)

पूर्णांक LzmaDecode(CLzmaDecoderState *vs,
    स्थिर अचिन्हित अक्षर *inStream, आकार_T in_आकार, आकार_T *in_आकार_Processed,
    अचिन्हित अक्षर *outStream, आकार_T out_आकार, आकार_T *out_आकार_Processed)
{
  CProb *p = vs->Probs;
  आकार_T nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  पूर्णांक lc = vs->Properties.lc;


  पूर्णांक state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  पूर्णांक len = 0;
  स्थिर Byte *बफर_2;
  स्थिर Byte *बफर_Lim;
  UInt32 Range;
  UInt32 Code;

  *in_आकार_Processed = 0;
  *out_आकार_Processed = 0;

  {
    UInt32 i;
    UInt32 अंक_Probs = Literal + ((UInt32)LZMA_LIT_आकार << (lc + vs->Properties.lp));
    क्रम (i = 0; i < अंक_Probs; i++)
      p[i] = k_Bit_Model_योग >> 1;
  }
  
  RC_INIT(inStream, in_आकार);


  जबतक(nowPos < out_आकार)
  {
    CProb *prob;
    UInt32 bound;
    पूर्णांक posState = (पूर्णांक)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_अंक_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      पूर्णांक प्रतीक_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_आकार * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      अगर (state >= k_अंक_Lit_States)
      {
        पूर्णांक matchByte;
        matchByte = outStream[nowPos - rep0];
        करो
        {
          पूर्णांक bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + प्रतीक_2;
          RC_GET_BIT2(probLit, प्रतीक_2, अगर (bit != 0) अवरोध, अगर (bit == 0) अवरोध)
        }
        जबतक (प्रतीक_2 < 0x100);
      }
      जबतक (प्रतीक_2 < 0x100)
      {
        CProb *probLit = prob + प्रतीक_2;
        RC_GET_BIT(probLit, प्रतीक_2)
      }
      previousByte = (Byte)प्रतीक_2;

      outStream[nowPos++] = previousByte;
      अगर (state < 4) state = 0;
      अन्यथा अगर (state < 10) state -= 3;
      अन्यथा state -= 6;
    }
    अन्यथा             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_अंक_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      अन्यथा
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_अंक_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            अगर (nowPos == 0)
              वापस LZMA_परिणाम_तथ्य_ERROR;
            
            state = state < k_अंक_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            जारी;
          }
          अन्यथा
          {
            UpdateBit1(prob);
          }
        }
        अन्यथा
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          अन्यथा 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            अन्यथा
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
        state = state < k_अंक_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        पूर्णांक अंक_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_निचला + (posState << k_Len_अंक_निचला_Bits);
          offset = 0;
          अंक_Bits = k_Len_अंक_निचला_Bits;
        }
        अन्यथा
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_अंक_Mid_Bits);
            offset = k_Len_अंक_निचला_Symbols;
            अंक_Bits = k_Len_अंक_Mid_Bits;
          }
          अन्यथा
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ऊपरी;
            offset = k_Len_अंक_निचला_Symbols + k_Len_अंक_Mid_Symbols;
            अंक_Bits = k_Len_अंक_ऊपरी_Bits;
          }
        }
        Range_Decoder_Bit_वृक्ष_Decode(probLen, अंक_Bits, len);
        len += offset;
      }

      अगर (state < 4)
      {
        पूर्णांक posSlot;
        state += k_अंक_Lit_States;
        prob = p + PosSlot +
            ((len < k_अंक_Len_To_Pos_States ? len : k_अंक_Len_To_Pos_States - 1) << 
            k_अंक_Pos_Slot_Bits);
        Range_Decoder_Bit_वृक्ष_Decode(prob, k_अंक_Pos_Slot_Bits, posSlot);
        अगर (posSlot >= k_Start_Pos_Model_सूचकांक)
        {
          पूर्णांक अंक_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          अगर (posSlot < k_End_Pos_Model_सूचकांक)
          {
            rep0 <<= अंक_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          अन्यथा
          {
            अंक_Direct_Bits -= k_अंक_Align_Bits;
            करो
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              अगर (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            जबतक (--अंक_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_अंक_Align_Bits;
            अंक_Direct_Bits = k_अंक_Align_Bits;
          }
          {
            पूर्णांक i = 1;
            पूर्णांक mi = 1;
            करो
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            जबतक(--अंक_Direct_Bits != 0);
          }
        }
        अन्यथा
          rep0 = posSlot;
        अगर (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          अवरोध;
        }
      }

      len += kMatchMinLen;
      अगर (rep0 > nowPos)
        वापस LZMA_परिणाम_तथ्य_ERROR;


      करो
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      जबतक(len != 0 && nowPos < out_आकार);
    }
  }
  RC_NORMALIZE;


  *in_आकार_Processed = (आकार_T)(बफर_2 - inStream);
  *out_आकार_Processed = nowPos;
  वापस LZMA_परिणाम_OK;
}
