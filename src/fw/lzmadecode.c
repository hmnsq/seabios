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

#अन्तर्भाव "lzmadecode.h"

#परिभाषय k_अङ्कः_शिखरम्_द्वयङ्काः 24
#परिभाषय k_शिखरम्_मानम् ((UInt32)1 << k_अङ्कः_शिखरम्_द्वयङ्काः)

#परिभाषय k_अङ्कः_Bit_Model_योग_द्वयङ्काः 11
#परिभाषय k_Bit_Model_योग (1 << k_अङ्कः_Bit_Model_योग_द्वयङ्काः)
#परिभाषय k_अङ्कः_Move_द्वयङ्काः 5

#परिभाषय RC_READ_BYTE (*बफर_2++)

#परिभाषय RC_INIT2 संहिता_2 = 0; Range = 0xFFFFFFFF; \
  { पूर्णाङ्क i; प्रति(i = 0; i < 5; i++) { RC_TEST; संहिता_2 = (संहिता_2 << 8) | RC_READ_BYTE; }}


#परिभाषय RC_TEST { यदि (बफर_2 == बफर_Lim) प्रत्यागम LZMA_परिणाम_दत्त_ERROR; }

#परिभाषय RC_INIT(बफर, बफर_आकार) बफर_2 = बफर; बफर_Lim = बफर + बफर_आकार; RC_INIT2
 

#परिभाषय RC_NORMALIZE यदि (Range < k_शिखरम्_मानम्) { RC_TEST; Range <<= 8; संहिता_2 = (संहिता_2 << 8) | RC_READ_BYTE; }

#परिभाषय IfBit0(p) RC_NORMALIZE; bound = (Range >> k_अङ्कः_Bit_Model_योग_द्वयङ्काः) * *(p); यदि (संहिता_2 < bound)
#परिभाषय UpdateBit0(p) Range = bound; *(p) += (k_Bit_Model_योग - *(p)) >> k_अङ्कः_Move_द्वयङ्काः;
#परिभाषय UpdateBit1(p) Range -= bound; संहिता_2 -= bound; *(p) -= (*(p)) >> k_अङ्कः_Move_द्वयङ्काः;

#परिभाषय RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } अन्यथा \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#परिभाषय RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#परिभाषय Range_Decoder_Bit_वृक्षः_Decode(probs, अङ्कः_स्तराः, res) \
  { पूर्णाङ्क i = अङ्कः_स्तराः; res = 1; \
  कुरु { CProb *cp = probs + res; RC_GET_BIT(cp, res) } यावत्(--i != 0); \
  res -= (1 << अङ्कः_स्तराः); }


#परिभाषय k_अङ्कः_Pos_द्वयङ्काः_Max 4
#परिभाषय k_अङ्कः_Pos_अवस्थाः_Max (1 << k_अङ्कः_Pos_द्वयङ्काः_Max)

#परिभाषय k_Len_अङ्कः_नीचः_द्वयङ्काः 3
#परिभाषय k_Len_अङ्कः_नीचः_Symbols (1 << k_Len_अङ्कः_नीचः_द्वयङ्काः)
#परिभाषय k_Len_अङ्कः_Mid_द्वयङ्काः 3
#परिभाषय k_Len_अङ्कः_Mid_Symbols (1 << k_Len_अङ्कः_Mid_द्वयङ्काः)
#परिभाषय k_Len_अङ्कः_उच्चः_द्वयङ्काः 8
#परिभाषय k_Len_अङ्कः_उच्चः_Symbols (1 << k_Len_अङ्कः_उच्चः_द्वयङ्काः)

#परिभाषय LenChoice 0
#परिभाषय LenChoice2 (LenChoice + 1)
#परिभाषय Len_नीचः (LenChoice2 + 1)
#परिभाषय LenMid (Len_नीचः + (k_अङ्कः_Pos_अवस्थाः_Max << k_Len_अङ्कः_नीचः_द्वयङ्काः))
#परिभाषय Len_उच्चः (LenMid + (k_अङ्कः_Pos_अवस्थाः_Max << k_Len_अङ्कः_Mid_द्वयङ्काः))
#परिभाषय k_अङ्कः_Len_Probs (Len_उच्चः + k_Len_अङ्कः_उच्चः_Symbols) 


#परिभाषय k_अङ्कः_अवस्थाः 12
#परिभाषय k_अङ्कः_Lit_अवस्थाः 7

#परिभाषय k_Start_Pos_Model_सूचकांक 4
#परिभाषय k_अन्तः_Pos_Model_सूचकांक 14
#परिभाषय k_अङ्कः_पूर्णम्_Distances (1 << (k_अन्तः_Pos_Model_सूचकांक >> 1))

#परिभाषय k_अङ्कः_Pos_Slot_द्वयङ्काः 6
#परिभाषय k_अङ्कः_Len_गन्तव्यम्_Pos_अवस्थाः 4

#परिभाषय k_अङ्कः_Align_द्वयङ्काः 4
#परिभाषय k_Align_सारणी_आकार (1 << k_अङ्कः_Align_द्वयङ्काः)

#परिभाषय k_मेलनम्_Min_Len 2

#परिभाषय Is_मेलनम् 0
#परिभाषय IsRep (Is_मेलनम् + (k_अङ्कः_अवस्थाः << k_अङ्कः_Pos_द्वयङ्काः_Max))
#परिभाषय IsRepG0 (IsRep + k_अङ्कः_अवस्थाः)
#परिभाषय IsRepG1 (IsRepG0 + k_अङ्कः_अवस्थाः)
#परिभाषय IsRepG2 (IsRepG1 + k_अङ्कः_अवस्थाः)
#परिभाषय IsRep0Long (IsRepG2 + k_अङ्कः_अवस्थाः)
#परिभाषय PosSlot (IsRep0Long + (k_अङ्कः_अवस्थाः << k_अङ्कः_Pos_द्वयङ्काः_Max))
#परिभाषय SpecPos (PosSlot + (k_अङ्कः_Len_गन्तव्यम्_Pos_अवस्थाः << k_अङ्कः_Pos_Slot_द्वयङ्काः))
#परिभाषय Align (SpecPos + k_अङ्कः_पूर्णम्_Distances - k_अन्तः_Pos_Model_सूचकांक)
#परिभाषय LenCoder (Align + k_Align_सारणी_आकार)
#परिभाषय RepLenCoder (LenCoder + k_अङ्कः_Len_Probs)
#परिभाषय Literal (RepLenCoder + k_अङ्कः_Len_Probs)

#यदि Literal != LZMA_आधारः_आकार
StopCompilingDueBUG
#यदि_अन्तः

पूर्णाङ्क LzmaDecodeProperties(CLzmaProperties *propsRes, ध्रुव अचिह्नित वर्ण *props_दत्त, पूर्णाङ्क आकार)
{
  अचिह्नित वर्ण prop0;
  यदि (आकार < LZMA_PROPERTIES_आकार)
    प्रत्यागम LZMA_परिणाम_दत्त_ERROR;
  prop0 = props_दत्त[0];
  यदि (prop0 >= (9 * 5 * 5))
    प्रत्यागम LZMA_परिणाम_दत्त_ERROR;
  {
    प्रति (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    प्रति (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  प्रत्यागम LZMA_परिणाम_OK;
}

#परिभाषय k_Lzma_प्रवाहः_Was_परिसमाप्तम्_Id (-1)

पूर्णाङ्क LzmaDecode(CLzmaDecoderState *vs,
    ध्रुव अचिह्नित वर्ण *अन्तर्_प्रवाहः, आकार_T अन्तर्_आकार, आकार_T *अन्तर्_आकार_Processed,
    अचिह्नित वर्ण *बहिर्गमनम्_प्रवाहः, आकार_T बहिर्गमनम्_आकार, आकार_T *बहिर्गमनम्_आकार_Processed)
{
  CProb *p = vs->Probs;
  आकार_T nowPos = 0;
  Byte previousByte = 0;
  UInt32 pos_State_आवरणम् = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_आवरणम् = (1 << (vs->Properties.lp)) - 1;
  पूर्णाङ्क lc = vs->Properties.lc;


  पूर्णाङ्क state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  पूर्णाङ्क len = 0;
  ध्रुव Byte *बफर_2;
  ध्रुव Byte *बफर_Lim;
  UInt32 Range;
  UInt32 संहिता_2;

  *अन्तर्_आकार_Processed = 0;
  *बहिर्गमनम्_आकार_Processed = 0;

  {
    UInt32 i;
    UInt32 अङ्कः_Probs = Literal + ((UInt32)LZMA_LIT_आकार << (lc + vs->Properties.lp));
    प्रति (i = 0; i < अङ्कः_Probs; i++)
      p[i] = k_Bit_Model_योग >> 1;
  }
  
  RC_INIT(अन्तर्_प्रवाहः, अन्तर्_आकार);


  यावत्(nowPos < बहिर्गमनम्_आकार)
  {
    CProb *prob;
    UInt32 bound;
    पूर्णाङ्क posState = (पूर्णाङ्क)(
        (nowPos 
        )
        & pos_State_आवरणम्);

    prob = p + Is_मेलनम् + (state << k_अङ्कः_Pos_द्वयङ्काः_Max) + posState;
    IfBit0(prob)
    {
      पूर्णाङ्क चिह्नम्_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_आकार * 
        (((
        (nowPos 
        )
        & literal_Pos_आवरणम्) << lc) + (previousByte >> (8 - lc))));

      यदि (state >= k_अङ्कः_Lit_अवस्थाः)
      {
        पूर्णाङ्क मेलनम्_Byte;
        मेलनम्_Byte = बहिर्गमनम्_प्रवाहः[nowPos - rep0];
        कुरु
        {
          पूर्णाङ्क bit;
          CProb *probLit;
          मेलनम्_Byte <<= 1;
          bit = (मेलनम्_Byte & 0x100);
          probLit = prob + 0x100 + bit + चिह्नम्_2;
          RC_GET_BIT2(probLit, चिह्नम्_2, यदि (bit != 0) भङ्ग, यदि (bit == 0) भङ्ग)
        }
        यावत् (चिह्नम्_2 < 0x100);
      }
      यावत् (चिह्नम्_2 < 0x100)
      {
        CProb *probLit = prob + चिह्नम्_2;
        RC_GET_BIT(probLit, चिह्नम्_2)
      }
      previousByte = (Byte)चिह्नम्_2;

      बहिर्गमनम्_प्रवाहः[nowPos++] = previousByte;
      यदि (state < 4) state = 0;
      अन्यथा यदि (state < 10) state -= 3;
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
        state = state < k_अङ्कः_Lit_अवस्थाः ? 0 : 3;
        prob = p + LenCoder;
      }
      अन्यथा
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_अङ्कः_Pos_द्वयङ्काः_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            यदि (nowPos == 0)
              प्रत्यागम LZMA_परिणाम_दत्त_ERROR;
            
            state = state < k_अङ्कः_Lit_अवस्थाः ? 9 : 11;
            previousByte = बहिर्गमनम्_प्रवाहः[nowPos - rep0];
            बहिर्गमनम्_प्रवाहः[nowPos++] = previousByte;

            अनुवर्त;
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
        state = state < k_अङ्कः_Lit_अवस्थाः ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        पूर्णाङ्क अङ्कः_द्वयङ्काः, विस्थापनम्_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_नीचः + (posState << k_Len_अङ्कः_नीचः_द्वयङ्काः);
          विस्थापनम्_2 = 0;
          अङ्कः_द्वयङ्काः = k_Len_अङ्कः_नीचः_द्वयङ्काः;
        }
        अन्यथा
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_अङ्कः_Mid_द्वयङ्काः);
            विस्थापनम्_2 = k_Len_अङ्कः_नीचः_Symbols;
            अङ्कः_द्वयङ्काः = k_Len_अङ्कः_Mid_द्वयङ्काः;
          }
          अन्यथा
          {
            UpdateBit1(probLen);
            probLen = prob + Len_उच्चः;
            विस्थापनम्_2 = k_Len_अङ्कः_नीचः_Symbols + k_Len_अङ्कः_Mid_Symbols;
            अङ्कः_द्वयङ्काः = k_Len_अङ्कः_उच्चः_द्वयङ्काः;
          }
        }
        Range_Decoder_Bit_वृक्षः_Decode(probLen, अङ्कः_द्वयङ्काः, len);
        len += विस्थापनम्_2;
      }

      यदि (state < 4)
      {
        पूर्णाङ्क posSlot;
        state += k_अङ्कः_Lit_अवस्थाः;
        prob = p + PosSlot +
            ((len < k_अङ्कः_Len_गन्तव्यम्_Pos_अवस्थाः ? len : k_अङ्कः_Len_गन्तव्यम्_Pos_अवस्थाः - 1) << 
            k_अङ्कः_Pos_Slot_द्वयङ्काः);
        Range_Decoder_Bit_वृक्षः_Decode(prob, k_अङ्कः_Pos_Slot_द्वयङ्काः, posSlot);
        यदि (posSlot >= k_Start_Pos_Model_सूचकांक)
        {
          पूर्णाङ्क अङ्कः_प्रत्यक्षम्_द्वयङ्काः = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          यदि (posSlot < k_अन्तः_Pos_Model_सूचकांक)
          {
            rep0 <<= अङ्कः_प्रत्यक्षम्_द्वयङ्काः;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          अन्यथा
          {
            अङ्कः_प्रत्यक्षम्_द्वयङ्काः -= k_अङ्कः_Align_द्वयङ्काः;
            कुरु
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              यदि (संहिता_2 >= Range)
              {
                संहिता_2 -= Range;
                rep0 |= 1;
              }
            }
            यावत् (--अङ्कः_प्रत्यक्षम्_द्वयङ्काः != 0);
            prob = p + Align;
            rep0 <<= k_अङ्कः_Align_द्वयङ्काः;
            अङ्कः_प्रत्यक्षम्_द्वयङ्काः = k_अङ्कः_Align_द्वयङ्काः;
          }
          {
            पूर्णाङ्क i = 1;
            पूर्णाङ्क mi = 1;
            कुरु
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            यावत्(--अङ्कः_प्रत्यक्षम्_द्वयङ्काः != 0);
          }
        }
        अन्यथा
          rep0 = posSlot;
        यदि (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_प्रवाहः_Was_परिसमाप्तम्_Id;
          भङ्ग;
        }
      }

      len += k_मेलनम्_Min_Len;
      यदि (rep0 > nowPos)
        प्रत्यागम LZMA_परिणाम_दत्त_ERROR;


      कुरु
      {
        previousByte = बहिर्गमनम्_प्रवाहः[nowPos - rep0];
        len--;
        बहिर्गमनम्_प्रवाहः[nowPos++] = previousByte;
      }
      यावत्(len != 0 && nowPos < बहिर्गमनम्_आकार);
    }
  }
  RC_NORMALIZE;


  *अन्तर्_आकार_Processed = (आकार_T)(बफर_2 - अन्तर्_प्रवाहः);
  *बहिर्गमनम्_आकार_Processed = nowPos;
  प्रत्यागम LZMA_परिणाम_OK;
}
