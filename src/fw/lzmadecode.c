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

#అన్తర్భావ "lzmadecode.h"

#నిర్వచించు k_అంకె_శిఖరం_బిట్లు 24
#నిర్వచించు k_శిఖరం_విలువ ((UInt32)1 << k_అంకె_శిఖరం_బిట్లు)

#నిర్వచించు k_అంకె_బిట్టు_Model_మొత్తం_బిట్లు 11
#నిర్వచించు k_బిట్టు_Model_మొత్తం (1 << k_అంకె_బిట్టు_Model_మొత్తం_బిట్లు)
#నిర్వచించు k_అంకె_Move_బిట్లు 5

#నిర్వచించు RC_READ_BYTE (*బఫరు_2++)

#నిర్వచించు RC_INIT2 సంకేతం_3 = 0; Range = 0xFFFFFFFF; \
  { పూర్ణాంకం i; కొరకు(i = 0; i < 5; i++) { RC_TEST; సంకేతం_3 = (సంకేతం_3 << 8) | RC_READ_BYTE; }}


#నిర్వచించు RC_TEST { ఒకవేళ (బఫరు_2 == బఫరు_Lim) తిరిగి LZMA_ఫలితం_దత్తాంశం_ERROR; }

#నిర్వచించు RC_INIT(బఫరు, బఫరు_కొలత) బఫరు_2 = బఫరు; బఫరు_Lim = బఫరు + బఫరు_కొలత; RC_INIT2
 

#నిర్వచించు RC_NORMALIZE ఒకవేళ (Range < k_శిఖరం_విలువ) { RC_TEST; Range <<= 8; సంకేతం_3 = (సంకేతం_3 << 8) | RC_READ_BYTE; }

#నిర్వచించు IfBit0(p) RC_NORMALIZE; bound = (Range >> k_అంకె_బిట్టు_Model_మొత్తం_బిట్లు) * *(p); ఒకవేళ (సంకేతం_3 < bound)
#నిర్వచించు UpdateBit0(p) Range = bound; *(p) += (k_బిట్టు_Model_మొత్తం - *(p)) >> k_అంకె_Move_బిట్లు;
#నిర్వచించు UpdateBit1(p) Range -= bound; సంకేతం_3 -= bound; *(p) -= (*(p)) >> k_అంకె_Move_బిట్లు;

#నిర్వచించు RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } లేకపోతే \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#నిర్వచించు RC_GET_బిట్టు(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#నిర్వచించు Range_Decoder_బిట్టు_వృక్షం_Decode(probs, అంకె_స్థాయిలు, res) \
  { పూర్ణాంకం i = అంకె_స్థాయిలు; res = 1; \
  చేయి { CProb *cp = probs + res; RC_GET_బిట్టు(cp, res) } అయితే(--i != 0); \
  res -= (1 << అంకె_స్థాయిలు); }


#నిర్వచించు k_అంకె_Pos_బిట్లు_Max 4
#నిర్వచించు k_అంకె_Pos_స్థితులు_Max (1 << k_అంకె_Pos_బిట్లు_Max)

#నిర్వచించు k_Len_అంకె_దిగువ_బిట్లు 3
#నిర్వచించు k_Len_అంకె_దిగువ_Symbols (1 << k_Len_అంకె_దిగువ_బిట్లు)
#నిర్వచించు k_Len_అంకె_Mid_బిట్లు 3
#నిర్వచించు k_Len_అంకె_Mid_Symbols (1 << k_Len_అంకె_Mid_బిట్లు)
#నిర్వచించు k_Len_అంకె_ఎగువ_బిట్లు 8
#నిర్వచించు k_Len_అంకె_ఎగువ_Symbols (1 << k_Len_అంకె_ఎగువ_బిట్లు)

#నిర్వచించు LenChoice 0
#నిర్వచించు LenChoice2 (LenChoice + 1)
#నిర్వచించు Len_దిగువ (LenChoice2 + 1)
#నిర్వచించు LenMid (Len_దిగువ + (k_అంకె_Pos_స్థితులు_Max << k_Len_అంకె_దిగువ_బిట్లు))
#నిర్వచించు Len_ఎగువ (LenMid + (k_అంకె_Pos_స్థితులు_Max << k_Len_అంకె_Mid_బిట్లు))
#నిర్వచించు k_అంకె_Len_Probs (Len_ఎగువ + k_Len_అంకె_ఎగువ_Symbols) 


#నిర్వచించు k_అంకె_స్థితులు 12
#నిర్వచించు k_అంకె_Lit_స్థితులు 7

#నిర్వచించు k_ప్రారంభం_Pos_Model_సూచీ 4
#నిర్వచించు k_చివర_Pos_Model_సూచీ 14
#నిర్వచించు k_అంకె_నిండిన_Distances (1 << (k_చివర_Pos_Model_సూచీ >> 1))

#నిర్వచించు k_అంకె_Pos_స్థానం_బిట్లు 6
#నిర్వచించు k_అంకె_Len_గమ్యం_Pos_స్థితులు 4

#నిర్వచించు k_అంకె_Align_బిట్లు 4
#నిర్వచించు k_Align_పట్టిక_కొలత (1 << k_అంకె_Align_బిట్లు)

#నిర్వచించు k_సరిపోలిక_Min_Len 2

#నిర్వచించు Is_సరిపోలిక 0
#నిర్వచించు IsRep (Is_సరిపోలిక + (k_అంకె_స్థితులు << k_అంకె_Pos_బిట్లు_Max))
#నిర్వచించు IsRepG0 (IsRep + k_అంకె_స్థితులు)
#నిర్వచించు IsRepG1 (IsRepG0 + k_అంకె_స్థితులు)
#నిర్వచించు IsRepG2 (IsRepG1 + k_అంకె_స్థితులు)
#నిర్వచించు IsRep0Long (IsRepG2 + k_అంకె_స్థితులు)
#నిర్వచించు Pos_స్థానం (IsRep0Long + (k_అంకె_స్థితులు << k_అంకె_Pos_బిట్లు_Max))
#నిర్వచించు SpecPos (Pos_స్థానం + (k_అంకె_Len_గమ్యం_Pos_స్థితులు << k_అంకె_Pos_స్థానం_బిట్లు))
#నిర్వచించు Align (SpecPos + k_అంకె_నిండిన_Distances - k_చివర_Pos_Model_సూచీ)
#నిర్వచించు LenCoder (Align + k_Align_పట్టిక_కొలత)
#నిర్వచించు RepLenCoder (LenCoder + k_అంకె_Len_Probs)
#నిర్వచించు Literal (RepLenCoder + k_అంకె_Len_Probs)

#ఒకవేళ Literal != LZMA_పునాది_కొలత
StopCompilingDueBUG
#అయితే_ముగింపు

పూర్ణాంకం LzmaDecodeProperties(CLzmaProperties *propsRes, స్థిరరాశి గుర్తులేని అక్షరం *props_దత్తాంశం, పూర్ణాంకం కొలత)
{
  గుర్తులేని అక్షరం prop0;
  ఒకవేళ (కొలత < LZMA_PROPERTIES_కొలత)
    తిరిగి LZMA_ఫలితం_దత్తాంశం_ERROR;
  prop0 = props_దత్తాంశం[0];
  ఒకవేళ (prop0 >= (9 * 5 * 5))
    తిరిగి LZMA_ఫలితం_దత్తాంశం_ERROR;
  {
    కొరకు (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    కొరకు (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  తిరిగి LZMA_ఫలితం_OK;
}

#నిర్వచించు k_Lzma_ప్రవాహం_Was_ముగిసిన_Id (-1)

పూర్ణాంకం LzmaDecode(CLzmaDecoderState *vs,
    స్థిరరాశి గుర్తులేని అక్షరం *లోపల_ప్రవాహం, కొలత_T లోపల_కొలత, కొలత_T *లోపల_కొలత_Processed,
    గుర్తులేని అక్షరం *బహిర్గతం_ప్రవాహం, కొలత_T బహిర్గతం_కొలత, కొలత_T *బహిర్గతం_కొలత_Processed)
{
  CProb *p = vs->Probs;
  కొలత_T nowPos = 0;
  Byte మునుపటి_Byte = 0;
  UInt32 pos_State_ముసుగు = (1 << (vs->Properties.pb)) - 1;
  UInt32 literal_Pos_ముసుగు = (1 << (vs->Properties.lp)) - 1;
  పూర్ణాంకం lc = vs->Properties.lc;


  పూర్ణాంకం state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  పూర్ణాంకం len = 0;
  స్థిరరాశి Byte *బఫరు_2;
  స్థిరరాశి Byte *బఫరు_Lim;
  UInt32 Range;
  UInt32 సంకేతం_3;

  *లోపల_కొలత_Processed = 0;
  *బహిర్గతం_కొలత_Processed = 0;

  {
    UInt32 i;
    UInt32 అంకె_Probs = Literal + ((UInt32)LZMA_LIT_కొలత << (lc + vs->Properties.lp));
    కొరకు (i = 0; i < అంకె_Probs; i++)
      p[i] = k_బిట్టు_Model_మొత్తం >> 1;
  }
  
  RC_INIT(లోపల_ప్రవాహం, లోపల_కొలత);


  అయితే(nowPos < బహిర్గతం_కొలత)
  {
    CProb *prob;
    UInt32 bound;
    పూర్ణాంకం posState = (పూర్ణాంకం)(
        (nowPos 
        )
        & pos_State_ముసుగు);

    prob = p + Is_సరిపోలిక + (state << k_అంకె_Pos_బిట్లు_Max) + posState;
    IfBit0(prob)
    {
      పూర్ణాంకం చిహ్నం_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_కొలత * 
        (((
        (nowPos 
        )
        & literal_Pos_ముసుగు) << lc) + (మునుపటి_Byte >> (8 - lc))));

      ఒకవేళ (state >= k_అంకె_Lit_స్థితులు)
      {
        పూర్ణాంకం సరిపోలిక_Byte;
        సరిపోలిక_Byte = బహిర్గతం_ప్రవాహం[nowPos - rep0];
        చేయి
        {
          పూర్ణాంకం బిట్టు;
          CProb *probLit;
          సరిపోలిక_Byte <<= 1;
          బిట్టు = (సరిపోలిక_Byte & 0x100);
          probLit = prob + 0x100 + బిట్టు + చిహ్నం_2;
          RC_GET_BIT2(probLit, చిహ్నం_2, ఒకవేళ (బిట్టు != 0) ఆపు, ఒకవేళ (బిట్టు == 0) ఆపు)
        }
        అయితే (చిహ్నం_2 < 0x100);
      }
      అయితే (చిహ్నం_2 < 0x100)
      {
        CProb *probLit = prob + చిహ్నం_2;
        RC_GET_బిట్టు(probLit, చిహ్నం_2)
      }
      మునుపటి_Byte = (Byte)చిహ్నం_2;

      బహిర్గతం_ప్రవాహం[nowPos++] = మునుపటి_Byte;
      ఒకవేళ (state < 4) state = 0;
      లేకపోతే ఒకవేళ (state < 10) state -= 3;
      లేకపోతే state -= 6;
    }
    లేకపోతే             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_అంకె_Lit_స్థితులు ? 0 : 3;
        prob = p + LenCoder;
      }
      లేకపోతే
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_అంకె_Pos_బిట్లు_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            ఒకవేళ (nowPos == 0)
              తిరిగి LZMA_ఫలితం_దత్తాంశం_ERROR;
            
            state = state < k_అంకె_Lit_స్థితులు ? 9 : 11;
            మునుపటి_Byte = బహిర్గతం_ప్రవాహం[nowPos - rep0];
            బహిర్గతం_ప్రవాహం[nowPos++] = మునుపటి_Byte;

            కొనసాగు;
          }
          లేకపోతే
          {
            UpdateBit1(prob);
          }
        }
        లేకపోతే
        {
          UInt32 దూరం;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            దూరం = rep1;
          }
          లేకపోతే 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              దూరం = rep2;
            }
            లేకపోతే
            {
              UpdateBit1(prob);
              దూరం = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = దూరం;
        }
        state = state < k_అంకె_Lit_స్థితులు ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        పూర్ణాంకం అంకె_బిట్లు, స్థానభ్రంశం_2;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_దిగువ + (posState << k_Len_అంకె_దిగువ_బిట్లు);
          స్థానభ్రంశం_2 = 0;
          అంకె_బిట్లు = k_Len_అంకె_దిగువ_బిట్లు;
        }
        లేకపోతే
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_అంకె_Mid_బిట్లు);
            స్థానభ్రంశం_2 = k_Len_అంకె_దిగువ_Symbols;
            అంకె_బిట్లు = k_Len_అంకె_Mid_బిట్లు;
          }
          లేకపోతే
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ఎగువ;
            స్థానభ్రంశం_2 = k_Len_అంకె_దిగువ_Symbols + k_Len_అంకె_Mid_Symbols;
            అంకె_బిట్లు = k_Len_అంకె_ఎగువ_బిట్లు;
          }
        }
        Range_Decoder_బిట్టు_వృక్షం_Decode(probLen, అంకె_బిట్లు, len);
        len += స్థానభ్రంశం_2;
      }

      ఒకవేళ (state < 4)
      {
        పూర్ణాంకం pos_స్థానం;
        state += k_అంకె_Lit_స్థితులు;
        prob = p + Pos_స్థానం +
            ((len < k_అంకె_Len_గమ్యం_Pos_స్థితులు ? len : k_అంకె_Len_గమ్యం_Pos_స్థితులు - 1) << 
            k_అంకె_Pos_స్థానం_బిట్లు);
        Range_Decoder_బిట్టు_వృక్షం_Decode(prob, k_అంకె_Pos_స్థానం_బిట్లు, pos_స్థానం);
        ఒకవేళ (pos_స్థానం >= k_ప్రారంభం_Pos_Model_సూచీ)
        {
          పూర్ణాంకం అంకె_ప్రత్యక్ష_బిట్లు = ((pos_స్థానం >> 1) - 1);
          rep0 = (2 | ((UInt32)pos_స్థానం & 1));
          ఒకవేళ (pos_స్థానం < k_చివర_Pos_Model_సూచీ)
          {
            rep0 <<= అంకె_ప్రత్యక్ష_బిట్లు;
            prob = p + SpecPos + rep0 - pos_స్థానం - 1;
          }
          లేకపోతే
          {
            అంకె_ప్రత్యక్ష_బిట్లు -= k_అంకె_Align_బిట్లు;
            చేయి
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              ఒకవేళ (సంకేతం_3 >= Range)
              {
                సంకేతం_3 -= Range;
                rep0 |= 1;
              }
            }
            అయితే (--అంకె_ప్రత్యక్ష_బిట్లు != 0);
            prob = p + Align;
            rep0 <<= k_అంకె_Align_బిట్లు;
            అంకె_ప్రత్యక్ష_బిట్లు = k_అంకె_Align_బిట్లు;
          }
          {
            పూర్ణాంకం i = 1;
            పూర్ణాంకం mi = 1;
            చేయి
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            అయితే(--అంకె_ప్రత్యక్ష_బిట్లు != 0);
          }
        }
        లేకపోతే
          rep0 = pos_స్థానం;
        ఒకవేళ (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = k_Lzma_ప్రవాహం_Was_ముగిసిన_Id;
          ఆపు;
        }
      }

      len += k_సరిపోలిక_Min_Len;
      ఒకవేళ (rep0 > nowPos)
        తిరిగి LZMA_ఫలితం_దత్తాంశం_ERROR;


      చేయి
      {
        మునుపటి_Byte = బహిర్గతం_ప్రవాహం[nowPos - rep0];
        len--;
        బహిర్గతం_ప్రవాహం[nowPos++] = మునుపటి_Byte;
      }
      అయితే(len != 0 && nowPos < బహిర్గతం_కొలత);
    }
  }
  RC_NORMALIZE;


  *లోపల_కొలత_Processed = (కొలత_T)(బఫరు_2 - లోపల_ప్రవాహం);
  *బహిర్గతం_కొలత_Processed = nowPos;
  తిరిగి LZMA_ఫలితం_OK;
}
