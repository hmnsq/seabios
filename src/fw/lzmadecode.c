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

#அந்தர்பாவ "lzmadecode.h"

#define k_இலக்கம்_உச்சி_Bits 24
#define k_உச்சி_மதிப்பு ((UInt32)1 << k_இலக்கம்_உச்சி_Bits)

#define k_இலக்கம்_Bit_Model_Total_Bits 11
#define kBitModelTotal (1 << k_இலக்கம்_Bit_Model_Total_Bits)
#define k_இலக்கம்_Move_Bits 5

#define RC_READ_BYTE (*Buffer++)

#define RC_INIT2 Code = 0; Range = 0xFFFFFFFF; \
  { முழு i; க்கு(i = 0; i < 5; i++) { RC_TEST; Code = (Code << 8) | RC_READ_BYTE; }}


#define RC_TEST { எனில் (Buffer == BufferLim) திரும்பு LZMA_RESULT_DATA_ERROR; }

#define RC_INIT(buffer, bufferSize) Buffer = buffer; BufferLim = buffer + bufferSize; RC_INIT2
 

#define RC_NORMALIZE எனில் (Range < k_உச்சி_மதிப்பு) { RC_TEST; Range <<= 8; Code = (Code << 8) | RC_READ_BYTE; }

#define IfBit0(p) RC_NORMALIZE; bound = (Range >> k_இலக்கம்_Bit_Model_Total_Bits) * *(p); எனில் (Code < bound)
#define UpdateBit0(p) Range = bound; *(p) += (kBitModelTotal - *(p)) >> k_இலக்கம்_Move_Bits;
#define UpdateBit1(p) Range -= bound; Code -= bound; *(p) -= (*(p)) >> k_இலக்கம்_Move_Bits;

#define RC_GET_BIT2(p, mi, A0, A1) IfBit0(p) \
  { UpdateBit0(p); mi <<= 1; A0; } இல்லையெனில் \
  { UpdateBit1(p); mi = (mi + mi) + 1; A1; } 
  
#define RC_GET_BIT(p, mi) RC_GET_BIT2(p, mi, ; , ;)               

#define Range_Decoder_Bit_மரம்_Decode(probs, இலக்கம்_Levels, res) \
  { முழு i = இலக்கம்_Levels; res = 1; \
  செய் { CProb *cp = probs + res; RC_GET_BIT(cp, res) } போது(--i != 0); \
  res -= (1 << இலக்கம்_Levels); }


#define k_இலக்கம்_Pos_Bits_Max 4
#define k_இலக்கம்_Pos_States_Max (1 << k_இலக்கம்_Pos_Bits_Max)

#define k_Len_இலக்கம்_கீழ்_Bits 3
#define k_Len_இலக்கம்_கீழ்_Symbols (1 << k_Len_இலக்கம்_கீழ்_Bits)
#define k_Len_இலக்கம்_Mid_Bits 3
#define k_Len_இலக்கம்_Mid_Symbols (1 << k_Len_இலக்கம்_Mid_Bits)
#define k_Len_இலக்கம்_உயர்வு_Bits 8
#define k_Len_இலக்கம்_உயர்வு_Symbols (1 << k_Len_இலக்கம்_உயர்வு_Bits)

#define LenChoice 0
#define LenChoice2 (LenChoice + 1)
#define Len_கீழ் (LenChoice2 + 1)
#define LenMid (Len_கீழ் + (k_இலக்கம்_Pos_States_Max << k_Len_இலக்கம்_கீழ்_Bits))
#define Len_உயர்வு (LenMid + (k_இலக்கம்_Pos_States_Max << k_Len_இலக்கம்_Mid_Bits))
#define k_இலக்கம்_Len_Probs (Len_உயர்வு + k_Len_இலக்கம்_உயர்வு_Symbols) 


#define k_இலக்கம்_States 12
#define k_இலக்கம்_Lit_States 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define k_இலக்கம்_Full_Distances (1 << (kEndPosModelIndex >> 1))

#define k_இலக்கம்_Pos_Slot_Bits 6
#define k_இலக்கம்_Len_To_Pos_States 4

#define k_இலக்கம்_Align_Bits 4
#define kAlignTableSize (1 << k_இலக்கம்_Align_Bits)

#define kMatchMinLen 2

#define IsMatch 0
#define IsRep (IsMatch + (k_இலக்கம்_States << k_இலக்கம்_Pos_Bits_Max))
#define IsRepG0 (IsRep + k_இலக்கம்_States)
#define IsRepG1 (IsRepG0 + k_இலக்கம்_States)
#define IsRepG2 (IsRepG1 + k_இலக்கம்_States)
#define IsRep0Long (IsRepG2 + k_இலக்கம்_States)
#define PosSlot (IsRep0Long + (k_இலக்கம்_States << k_இலக்கம்_Pos_Bits_Max))
#define SpecPos (PosSlot + (k_இலக்கம்_Len_To_Pos_States << k_இலக்கம்_Pos_Slot_Bits))
#define Align (SpecPos + k_இலக்கம்_Full_Distances - kEndPosModelIndex)
#define LenCoder (Align + kAlignTableSize)
#define RepLenCoder (LenCoder + k_இலக்கம்_Len_Probs)
#define Literal (RepLenCoder + k_இலக்கம்_Len_Probs)

#எனில் Literal != LZMA_BASE_SIZE
StopCompilingDueBUG
#endif

முழு LzmaDecodeProperties(CLzmaProperties *propsRes, மாறிலி unsigned எழுத்து *propsData, முழு size)
{
  unsigned எழுத்து prop0;
  எனில் (size < LZMA_PROPERTIES_SIZE)
    திரும்பு LZMA_RESULT_DATA_ERROR;
  prop0 = propsData[0];
  எனில் (prop0 >= (9 * 5 * 5))
    திரும்பு LZMA_RESULT_DATA_ERROR;
  {
    க்கு (propsRes->pb = 0; prop0 >= (9 * 5); propsRes->pb++, prop0 -= (9 * 5));
    க்கு (propsRes->lp = 0; prop0 >= 9; propsRes->lp++, prop0 -= 9);
    propsRes->lc = prop0;
    /*
    unsigned char remainder = (unsigned char)(prop0 / 9);
    propsRes->lc = prop0 % 9;
    propsRes->pb = remainder / 5;
    propsRes->lp = remainder % 5;
    */
  }

  திரும்பு LZMA_RESULT_OK;
}

#define kLzmaStreamWasFinishedId (-1)

முழு LzmaDecode(CLzmaDecoderState *vs,
    மாறிலி unsigned எழுத்து *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned எழுத்து *outStream, SizeT outSize, SizeT *outSizeProcessed)
{
  CProb *p = vs->Probs;
  SizeT nowPos = 0;
  Byte previousByte = 0;
  UInt32 posStateMask = (1 << (vs->Properties.pb)) - 1;
  UInt32 literalPosMask = (1 << (vs->Properties.lp)) - 1;
  முழு lc = vs->Properties.lc;


  முழு state = 0;
  UInt32 rep0 = 1, rep1 = 1, rep2 = 1, rep3 = 1;
  முழு len = 0;
  மாறிலி Byte *Buffer;
  மாறிலி Byte *BufferLim;
  UInt32 Range;
  UInt32 Code;

  *inSizeProcessed = 0;
  *outSizeProcessed = 0;

  {
    UInt32 i;
    UInt32 இலக்கம்_Probs = Literal + ((UInt32)LZMA_LIT_SIZE << (lc + vs->Properties.lp));
    க்கு (i = 0; i < இலக்கம்_Probs; i++)
      p[i] = kBitModelTotal >> 1;
  }
  
  RC_INIT(inStream, inSize);


  போது(nowPos < outSize)
  {
    CProb *prob;
    UInt32 bound;
    முழு posState = (முழு)(
        (nowPos 
        )
        & posStateMask);

    prob = p + IsMatch + (state << k_இலக்கம்_Pos_Bits_Max) + posState;
    IfBit0(prob)
    {
      முழு குறியீடு_2 = 1;
      UpdateBit0(prob)
      prob = p + Literal + (LZMA_LIT_SIZE * 
        (((
        (nowPos 
        )
        & literalPosMask) << lc) + (previousByte >> (8 - lc))));

      எனில் (state >= k_இலக்கம்_Lit_States)
      {
        முழு matchByte;
        matchByte = outStream[nowPos - rep0];
        செய்
        {
          முழு bit;
          CProb *probLit;
          matchByte <<= 1;
          bit = (matchByte & 0x100);
          probLit = prob + 0x100 + bit + குறியீடு_2;
          RC_GET_BIT2(probLit, குறியீடு_2, எனில் (bit != 0) நிறுத்து, எனில் (bit == 0) நிறுத்து)
        }
        போது (குறியீடு_2 < 0x100);
      }
      போது (குறியீடு_2 < 0x100)
      {
        CProb *probLit = prob + குறியீடு_2;
        RC_GET_BIT(probLit, குறியீடு_2)
      }
      previousByte = (Byte)குறியீடு_2;

      outStream[nowPos++] = previousByte;
      எனில் (state < 4) state = 0;
      இல்லையெனில் எனில் (state < 10) state -= 3;
      இல்லையெனில் state -= 6;
    }
    இல்லையெனில்             
    {
      UpdateBit1(prob);
      prob = p + IsRep + state;
      IfBit0(prob)
      {
        UpdateBit0(prob);
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        state = state < k_இலக்கம்_Lit_States ? 0 : 3;
        prob = p + LenCoder;
      }
      இல்லையெனில்
      {
        UpdateBit1(prob);
        prob = p + IsRepG0 + state;
        IfBit0(prob)
        {
          UpdateBit0(prob);
          prob = p + IsRep0Long + (state << k_இலக்கம்_Pos_Bits_Max) + posState;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            
            எனில் (nowPos == 0)
              திரும்பு LZMA_RESULT_DATA_ERROR;
            
            state = state < k_இலக்கம்_Lit_States ? 9 : 11;
            previousByte = outStream[nowPos - rep0];
            outStream[nowPos++] = previousByte;

            தொடர்;
          }
          இல்லையெனில்
          {
            UpdateBit1(prob);
          }
        }
        இல்லையெனில்
        {
          UInt32 distance;
          UpdateBit1(prob);
          prob = p + IsRepG1 + state;
          IfBit0(prob)
          {
            UpdateBit0(prob);
            distance = rep1;
          }
          இல்லையெனில் 
          {
            UpdateBit1(prob);
            prob = p + IsRepG2 + state;
            IfBit0(prob)
            {
              UpdateBit0(prob);
              distance = rep2;
            }
            இல்லையெனில்
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
        state = state < k_இலக்கம்_Lit_States ? 8 : 11;
        prob = p + RepLenCoder;
      }
      {
        முழு இலக்கம்_Bits, offset;
        CProb *probLen = prob + LenChoice;
        IfBit0(probLen)
        {
          UpdateBit0(probLen);
          probLen = prob + Len_கீழ் + (posState << k_Len_இலக்கம்_கீழ்_Bits);
          offset = 0;
          இலக்கம்_Bits = k_Len_இலக்கம்_கீழ்_Bits;
        }
        இல்லையெனில்
        {
          UpdateBit1(probLen);
          probLen = prob + LenChoice2;
          IfBit0(probLen)
          {
            UpdateBit0(probLen);
            probLen = prob + LenMid + (posState << k_Len_இலக்கம்_Mid_Bits);
            offset = k_Len_இலக்கம்_கீழ்_Symbols;
            இலக்கம்_Bits = k_Len_இலக்கம்_Mid_Bits;
          }
          இல்லையெனில்
          {
            UpdateBit1(probLen);
            probLen = prob + Len_உயர்வு;
            offset = k_Len_இலக்கம்_கீழ்_Symbols + k_Len_இலக்கம்_Mid_Symbols;
            இலக்கம்_Bits = k_Len_இலக்கம்_உயர்வு_Bits;
          }
        }
        Range_Decoder_Bit_மரம்_Decode(probLen, இலக்கம்_Bits, len);
        len += offset;
      }

      எனில் (state < 4)
      {
        முழு posSlot;
        state += k_இலக்கம்_Lit_States;
        prob = p + PosSlot +
            ((len < k_இலக்கம்_Len_To_Pos_States ? len : k_இலக்கம்_Len_To_Pos_States - 1) << 
            k_இலக்கம்_Pos_Slot_Bits);
        Range_Decoder_Bit_மரம்_Decode(prob, k_இலக்கம்_Pos_Slot_Bits, posSlot);
        எனில் (posSlot >= kStartPosModelIndex)
        {
          முழு இலக்கம்_Direct_Bits = ((posSlot >> 1) - 1);
          rep0 = (2 | ((UInt32)posSlot & 1));
          எனில் (posSlot < kEndPosModelIndex)
          {
            rep0 <<= இலக்கம்_Direct_Bits;
            prob = p + SpecPos + rep0 - posSlot - 1;
          }
          இல்லையெனில்
          {
            இலக்கம்_Direct_Bits -= k_இலக்கம்_Align_Bits;
            செய்
            {
              RC_NORMALIZE
              Range >>= 1;
              rep0 <<= 1;
              எனில் (Code >= Range)
              {
                Code -= Range;
                rep0 |= 1;
              }
            }
            போது (--இலக்கம்_Direct_Bits != 0);
            prob = p + Align;
            rep0 <<= k_இலக்கம்_Align_Bits;
            இலக்கம்_Direct_Bits = k_இலக்கம்_Align_Bits;
          }
          {
            முழு i = 1;
            முழு mi = 1;
            செய்
            {
              CProb *prob3 = prob + mi;
              RC_GET_BIT2(prob3, mi, ; , rep0 |= i);
              i <<= 1;
            }
            போது(--இலக்கம்_Direct_Bits != 0);
          }
        }
        இல்லையெனில்
          rep0 = posSlot;
        எனில் (++rep0 == (UInt32)(0))
        {
          /* it's for stream version */
          len = kLzmaStreamWasFinishedId;
          நிறுத்து;
        }
      }

      len += kMatchMinLen;
      எனில் (rep0 > nowPos)
        திரும்பு LZMA_RESULT_DATA_ERROR;


      செய்
      {
        previousByte = outStream[nowPos - rep0];
        len--;
        outStream[nowPos++] = previousByte;
      }
      போது(len != 0 && nowPos < outSize);
    }
  }
  RC_NORMALIZE;


  *inSizeProcessed = (SizeT)(Buffer - inStream);
  *outSizeProcessed = nowPos;
  திரும்பு LZMA_RESULT_OK;
}
