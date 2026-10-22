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

typedef unsigned var_na Byte;
typedef unsigned short UInt16;
typedef unsigned puur_naa_nka UInt32;
typedef UInt32 SizeT;

#define CProb UInt16

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

typedef sa_nracanaa _CLzmaProperties
{
  puur_naa_nka lc;
  puur_naa_nka lp;
  puur_naa_nka pb;
}CLzmaProperties;

puur_naa_nka LzmaDecodeProperties(CLzmaProperties *propsRes, const unsigned var_na *propsData, puur_naa_nka size);

#define LzmaGetNumProbs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

typedef sa_nracanaa _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


puur_naa_nka LzmaDecode(CLzmaDecoderState *vs,
    const unsigned var_na *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned var_na *outStream, SizeT outSize, SizeT *outSizeProcessed);

#endif
