/* 
  LzmaDecode.h
  LZMA Decoder interface

  LZMA SDK 4.40 Copyright (c) 1999-2006 Igor Pavlov (2006-05-01)
  http://www.7-zip.org/

  LZMA SDK is licensed under two licenses:
  1) GNU Lesser General Public License (GNU LGPL)
  2) Common Public License (CPL)
  It means that you can select one of these two licenses and 
  follow rules of that license.

  SPECIAL EXCEPTION:
  Igor Pavlov, as the author of this code, expressly permits you to 
  statically or dynamically link your code (or bind by name) to the 
  interfaces of this file without subjecting your linked code to the 
  terms of the CPL or GNU LGPL. Any modifications or additions 
  to this file, however, are subject to the LGPL or CPL terms.
*/

#ifndef __LZMADECODE_H
#define __LZMADECODE_H

ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ವರ್ಣ Byte;
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned short UInt16;
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ಪೂರ್ಣಾಙ್ಕ UInt32;
ಪ್ರಕಾರನಿರ್ದೇಶ UInt32 SizeT;

#define CProb UInt16

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ _CLzmaProperties
{
  ಪೂರ್ಣಾಙ್ಕ lc;
  ಪೂರ್ಣಾಙ್ಕ lp;
  ಪೂರ್ಣಾಙ್ಕ pb;
}CLzmaProperties;

ಪೂರ್ಣಾಙ್ಕ LzmaDecodeProperties(CLzmaProperties *propsRes, ಧ್ರುವ unsigned ವರ್ಣ *propsData, ಪೂರ್ಣಾಙ್ಕ size);

#define Lzma_Get_ಅಂಕೆ_Probs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


ಪೂರ್ಣಾಙ್ಕ LzmaDecode(CLzmaDecoderState *vs,
    ಧ್ರುವ unsigned ವರ್ಣ *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned ವರ್ಣ *outStream, SizeT outSize, SizeT *outSizeProcessed);

#endif
