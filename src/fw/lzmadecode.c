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

#અન્તર્ભાવ "lzmadecode.h"

#define k_અંક_ટોચ_Bits 24
#define k_ટોચ_કિંમત ((UInt32)1 << k_અંક_ટોચ_Bits)

#define k_અંક_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_અંક_Bit_Model_Total_Bits)
#define k_અંક_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { પૂર્ણાંક i; માટે(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { જો (Buffer == BufferLim) પાછા LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE જો (Range < k_ટોચ_કિંમત) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_અંક_Bit_Model_Total_Bits) * *(p); જો (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_અંક_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_અંક_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } નહીંતર \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_વૃક્ષ_Decode(probs, અંક_Levels, res) \
  { પૂર્ણાંક i = અંક_Levels; res = 1; \
  કરો { CProb *cp = probs + res; RC_GET_BIT(cp, res) } જ્યારે(--i != 0); \
  res -= (1 << અંક_Levels); }


#define k_અંક_Pos_Bits_Max 4
#define k_અંક_Pos_States_Max (1 << k_અંક_Pos_Bits_Max)

#define k_Len_અંક_નીચલું_Bits 3
#define k_Len_અંક_નીચલું_Symbols (1 << k_Len_અંક_નીચલું_Bits)
#define k_Len_અંક_Mid_Bits 3
#define k_Len_અંક_Mid_Symbols (1 << k_Len_અંક_Mid_Bits)
#define k_Len_અંક_ઉપલું_Bits 8
#define k_Len_અંક_ઉપલું_Symbols (1 << k_Len_અંક_ઉપલું_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_નીચલું (LenChoice2 + 1)
#define LenMid (Len_નીચલું + (k_અંક_Pos_States_Max << k_Len_અંક_નીચલું_Bits))
#define Len_ઉપલું (LenMid + (k_અંક_Pos_States_Max << k_Len_અંક_Mid_Bits))
#define k_અંક_Len_Probs (Len_ઉપલું + k_Len_અંક_ઉપલું_Symbols) 


#define k_અંક_States 12
#define k_અંક_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_અંક_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_અંક_Pos_Slot_Bits 6
#define k_અંક_Len_To_Pos_States 4

#define k_અંક_Align_Bits 4
#define kAlignTableSize (1 << k_અંક_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_અંક_States << k_અંક_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_અંક_States)
#define IsRepG1 (IsRepG0 + k_અંક_States)
#define IsRepG2 (IsRepG1 + k_અંક_States)
#define IsRep0Long (IsRepG2 + k_અંક_States)
#define PosSlot (IsRep0Long + (k_અંક_States << k_અંક_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_અંક_Len_To_Pos_States << k_અંક_Pos_Slot_Bits))
#define Align (SpecPos + k_અંક_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_અંક_Len_Probs)
#define Literal (RepLenCoder + k_અંક_Len_Probs)

#જો Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

પૂર્ણાંક LzmaDecodeProperties(CLzmaProperties *propsRes, અચલ unsigned અક્ષર *propsData, પૂર્ણાંક size)
{
  unsigned અક્ષર prop0;
  જો (size < LZMA_PROPERTIES_SIZE)
    પાછા LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  જો (prop0 >= (9 * 5 * 5))
    પાછા LZMA_RESULT_DATA_ERROR;
  {
    માટે (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    માટે (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  પાછા LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

પૂર્ણાંક LzmaDecode(CLzmaDecoderState *vs,
    અચલ unsigned અક્ષર *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned અક્ષર *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  પૂર્ણાંક lc = vs->Properties.lc;


  પૂર્ણાંક state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  પૂર્ણાંક len = 0;
  અચલ Byte *Buffer;
  અચલ Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 અંક_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    માટે (i = 0; i < અંક_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  જ્યારે(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    પૂર્ણાંક posState = (પૂર્ણાંક)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_અંક_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      પૂર્ણાંક પ્રતીક_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      જો (state >= k_અંક_Lit_States)
      {
        પૂર્ણાંક matchByte;
        matchByte = outStream[nowPos - rep0];
        કરો
        {
          પૂર્ણાંક bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + પ્રતીક_2;
          RC_GET_BIT2(probLit, પ્રતીક_2, જો (bit != 0) તોડો, જો (bit == 0) તોડો)
        }
        જ્યારે (પ્રતીક_2 < 0x100);
      }
      જ્યારે (પ્રતીક_2 < 0x100)
      {
        CProb *probLit = prob + પ્રતીક_2;
        RC_GET_BIT(probLit, પ્રતીક_2)
      }
      previousByte = (Byte)પ્રતીક_2;

      outStream[nowPos++] = previousByte;
      જો (state < 4) state = 0;
      નહીંતર જો (state < 10) state -= 3;
      નહીંતર state -= 6;
    }
    નહીંતર             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_અંક_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      નહીંતર
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_અંક_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            જો (nowPos == 0)
              પાછા LZMA_RESULT_DATA_ERROR;
            
            state = state < k_અંક_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            ચાલુ;
          }
          નહીંતર
          {
            UpdateBit1(prob);
          }
        }
        નહીંતર
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          નહીંતર 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            નહીંતર
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
        state = state < k_અંક_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        પૂર્ણાંક અંક_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_નીચલું + (posState << k_Len_અંક_નીચલું_Bits);
          offset = 0;
          અંક_Bits = k_Len_અંક_નીચલું_Bits;
        }
        નહીંતર
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_અંક_Mid_Bits);
            offset = k_Len_અંક_નીચલું_Symbols;
            અંક_Bits = k_Len_અંક_Mid_Bits;
          }
          નહીંતર
          {
            UpdateBit1(probLen);
            probLen = prob + Len_ઉપલું;
            offset = k_Len_અંક_નીચલું_Symbols + k_Len_અંક_Mid_Symbols;
            અંક_Bits = k_Len_અંક_ઉપલું_Bits;
          }
        }
        Range_Decoder_Bit_વૃક્ષ_Decode(probLen, અંક_Bits, len);
        len += offset;
      }

      જો (state < 4)
      {
        પૂર્ણાંક posSlot;
        state += k_અંક_Lit_States;
        prob = p + PosSlot +
            ((len < k_અંક_Len_To_Pos_States ? len : k_અંક_Len_To_Pos_States - 1) << 
            k_અંક_Pos_Slot_Bits);
        Range_Decoder_Bit_વૃક્ષ_Decode(prob, k_અંક_Pos_Slot_Bits, posSlot);
        જો (posSlot >= kStartPosModelIndex)
        {
          પૂર્ણાંક અંક_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          જો (posSlot < kEndPosModelIndex)
          {
            rep0 <<= અંક_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          નહીંતર
          {
            અંક_Direct_Bits -= k_અંક_Align_Bits;
            કરો
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              જો (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            જ્યારે (--અંક_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_અંક_Align_Bits;
            અંક_Direct_Bits = k_અંક_Align_Bits;
          }
          {
            પૂર્ણાંક i = 1;
            પૂર્ણાંક mi = 1;
            કરો
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            જ્યારે(--અંક_Direct_Bits != 0);
          }
        }
        નહીંતર
          rep0 = posSlot;
        જો (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          તોડો;
        }
      }

      len += kMatchMinLen;
      જો (rep0 > nowPos)
        પાછા LZMA_RESULT_DATA_ERROR;


      કરો
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      જ્યારે(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  પાછા LZMA_RESULT_OK;
}
