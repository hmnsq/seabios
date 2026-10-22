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

#അന്തര്ഭാവ "lzmadecode.h"

#define k_അക്കം_മുകൾ_Bits 24
#define k_മുകൾ_മൂല്യം ((UInt32)1 << k_അക്കം_മുകൾ_Bits)

#define k_അക്കം_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_അക്കം_Bit_Model_Total_Bits)
#define k_അക്കം_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { പൂർണ്ണസംഖ്യ i; വേണ്ടി(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { എങ്കിൽ (Buffer == BufferLim) തിരികെ LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE എങ്കിൽ (Range < k_മുകൾ_മൂല്യം) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_അക്കം_Bit_Model_Total_Bits) * *(p); എങ്കിൽ (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_അക്കം_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_അക്കം_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } അല്ലെങ്കിൽ \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_വൃക്ഷം_Decode(probs, അക്കം_Levels, res) \
  { പൂർണ്ണസംഖ്യ i = അക്കം_Levels; res = 1; \
  ചെയ്യുക { CProb *cp = probs + res; RC_GET_BIT(cp, res) } ആകുമ്പോൾ(--i != 0); \
  res -= (1 << അക്കം_Levels); }


#define k_അക്കം_Pos_Bits_Max 4
#define k_അക്കം_Pos_States_Max (1 << k_അക്കം_Pos_Bits_Max)

#define k_Len_അക്കം_താഴെയുള്ള_Bits 3
#define k_Len_അക്കം_താഴെയുള്ള_Symbols (1 << k_Len_അക്കം_താഴെയുള്ള_Bits)
#define k_Len_അക്കം_Mid_Bits 3
#define k_Len_അക്കം_Mid_Symbols (1 << k_Len_അക്കം_Mid_Bits)
#define k_Len_അക്കം_മുകളിലെ_Bits 8
#define k_Len_അക്കം_മുകളിലെ_Symbols (1 << k_Len_അക്കം_മുകളിലെ_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_താഴെയുള്ള (LenChoice2 + 1)
#define LenMid (Len_താഴെയുള്ള + (k_അക്കം_Pos_States_Max << k_Len_അക്കം_താഴെയുള്ള_Bits))
#define Len_മുകളിലെ (LenMid + (k_അക്കം_Pos_States_Max << k_Len_അക്കം_Mid_Bits))
#define k_അക്കം_Len_Probs (Len_മുകളിലെ + k_Len_അക്കം_മുകളിലെ_Symbols) 


#define k_അക്കം_States 12
#define k_അക്കം_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_അക്കം_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_അക്കം_Pos_Slot_Bits 6
#define k_അക്കം_Len_To_Pos_States 4

#define k_അക്കം_Align_Bits 4
#define kAlignTableSize (1 << k_അക്കം_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_അക്കം_States << k_അക്കം_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_അക്കം_States)
#define IsRepG1 (IsRepG0 + k_അക്കം_States)
#define IsRepG2 (IsRepG1 + k_അക്കം_States)
#define IsRep0Long (IsRepG2 + k_അക്കം_States)
#define PosSlot (IsRep0Long + (k_അക്കം_States << k_അക്കം_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_അക്കം_Len_To_Pos_States << k_അക്കം_Pos_Slot_Bits))
#define Align (SpecPos + k_അക്കം_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_അക്കം_Len_Probs)
#define Literal (RepLenCoder + k_അക്കം_Len_Probs)

#എങ്കിൽ Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

പൂർണ്ണസംഖ്യ LzmaDecodeProperties(CLzmaProperties *propsRes, സ്ഥിരം unsigned അക്ഷരം *propsData, പൂർണ്ണസംഖ്യ size)
{
  unsigned അക്ഷരം prop0;
  എങ്കിൽ (size < LZMA_PROPERTIES_SIZE)
    തിരികെ LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  എങ്കിൽ (prop0 >= (9 * 5 * 5))
    തിരികെ LZMA_RESULT_DATA_ERROR;
  {
    വേണ്ടി (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    വേണ്ടി (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  തിരികെ LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

പൂർണ്ണസംഖ്യ LzmaDecode(CLzmaDecoderState *vs,
    സ്ഥിരം unsigned അക്ഷരം *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned അക്ഷരം *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  പൂർണ്ണസംഖ്യ lc = vs->Properties.lc;


  പൂർണ്ണസംഖ്യ state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  പൂർണ്ണസംഖ്യ len = 0;
  സ്ഥിരം Byte *Buffer;
  സ്ഥിരം Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 അക്കം_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    വേണ്ടി (i = 0; i < അക്കം_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  ആകുമ്പോൾ(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    പൂർണ്ണസംഖ്യ posState = (പൂർണ്ണസംഖ്യ)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_അക്കം_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      പൂർണ്ണസംഖ്യ ചിഹ്നം_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      എങ്കിൽ (state >= k_അക്കം_Lit_States)
      {
        പൂർണ്ണസംഖ്യ matchByte;
        matchByte = outStream[nowPos - rep0];
        ചെയ്യുക
        {
          പൂർണ്ണസംഖ്യ bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + ചിഹ്നം_2;
          RC_GET_BIT2(probLit, ചിഹ്നം_2, എങ്കിൽ (bit != 0) നിർത്തുക, എങ്കിൽ (bit == 0) നിർത്തുക)
        }
        ആകുമ്പോൾ (ചിഹ്നം_2 < 0x100);
      }
      ആകുമ്പോൾ (ചിഹ്നം_2 < 0x100)
      {
        CProb *probLit = prob + ചിഹ്നം_2;
        RC_GET_BIT(probLit, ചിഹ്നം_2)
      }
      previousByte = (Byte)ചിഹ്നം_2;

      outStream[nowPos++] = previousByte;
      എങ്കിൽ (state < 4) state = 0;
      അല്ലെങ്കിൽ എങ്കിൽ (state < 10) state -= 3;
      അല്ലെങ്കിൽ state -= 6;
    }
    അല്ലെങ്കിൽ             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_അക്കം_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      അല്ലെങ്കിൽ
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_അക്കം_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            എങ്കിൽ (nowPos == 0)
              തിരികെ LZMA_RESULT_DATA_ERROR;
            
            state = state < k_അക്കം_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            തുടരുക;
          }
          അല്ലെങ്കിൽ
          {
            UpdateBit1(prob);
          }
        }
        അല്ലെങ്കിൽ
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          അല്ലെങ്കിൽ 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            അല്ലെങ്കിൽ
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
        state = state < k_അക്കം_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        പൂർണ്ണസംഖ്യ അക്കം_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_താഴെയുള്ള + (posState << k_Len_അക്കം_താഴെയുള്ള_Bits);
          offset = 0;
          അക്കം_Bits = k_Len_അക്കം_താഴെയുള്ള_Bits;
        }
        അല്ലെങ്കിൽ
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_അക്കം_Mid_Bits);
            offset = k_Len_അക്കം_താഴെയുള്ള_Symbols;
            അക്കം_Bits = k_Len_അക്കം_Mid_Bits;
          }
          അല്ലെങ്കിൽ
          {
            UpdateBit1(probLen);
            probLen = prob + Len_മുകളിലെ;
            offset = k_Len_അക്കം_താഴെയുള്ള_Symbols + k_Len_അക്കം_Mid_Symbols;
            അക്കം_Bits = k_Len_അക്കം_മുകളിലെ_Bits;
          }
        }
        Range_Decoder_Bit_വൃക്ഷം_Decode(probLen, അക്കം_Bits, len);
        len += offset;
      }

      എങ്കിൽ (state < 4)
      {
        പൂർണ്ണസംഖ്യ posSlot;
        state += k_അക്കം_Lit_States;
        prob = p + PosSlot +
            ((len < k_അക്കം_Len_To_Pos_States ? len : k_അക്കം_Len_To_Pos_States - 1) << 
            k_അക്കം_Pos_Slot_Bits);
        Range_Decoder_Bit_വൃക്ഷം_Decode(prob, k_അക്കം_Pos_Slot_Bits, posSlot);
        എങ്കിൽ (posSlot >= kStartPosModelIndex)
        {
          പൂർണ്ണസംഖ്യ അക്കം_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          എങ്കിൽ (posSlot < kEndPosModelIndex)
          {
            rep0 <<= അക്കം_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          അല്ലെങ്കിൽ
          {
            അക്കം_Direct_Bits -= k_അക്കം_Align_Bits;
            ചെയ്യുക
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              എങ്കിൽ (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            ആകുമ്പോൾ (--അക്കം_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_അക്കം_Align_Bits;
            അക്കം_Direct_Bits = k_അക്കം_Align_Bits;
          }
          {
            പൂർണ്ണസംഖ്യ i = 1;
            പൂർണ്ണസംഖ്യ mi = 1;
            ചെയ്യുക
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            ആകുമ്പോൾ(--അക്കം_Direct_Bits != 0);
          }
        }
        അല്ലെങ്കിൽ
          rep0 = posSlot;
        എങ്കിൽ (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          നിർത്തുക;
        }
      }

      len += kMatchMinLen;
      എങ്കിൽ (rep0 > nowPos)
        തിരികെ LZMA_RESULT_DATA_ERROR;


      ചെയ്യുക
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      ആകുമ്പോൾ(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  തിരികെ LZMA_RESULT_OK;
}
