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

ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର Byte;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned short UInt16;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା UInt32;
ପ୍ରକାରବ୍ୟାଖ୍ୟା UInt32 SizeT;

#define CProb UInt16

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା _CLzmaProperties
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lp;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pb;
}CLzmaProperties;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା LzmaDecodeProperties(CLzmaProperties *propsRes, ଅଚଳ unsigned ଅକ୍ଷର *propsData, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size);

#define Lzma_Get_ଅଙ୍କ_Probs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


ପୂର୍ଣ୍ଣସଂଖ୍ୟା LzmaDecode(CLzmaDecoderState *vs,
    ଅଚଳ unsigned ଅକ୍ଷର *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned ଅକ୍ଷର *outStream, SizeT outSize, SizeT *outSizeProcessed);

#endif
