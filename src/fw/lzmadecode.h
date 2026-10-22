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

प्रकारव्याख्या अचिन्हित अक्षर Byte;
प्रकारव्याख्या अचिन्हित लघु UInt16;
प्रकारव्याख्या अचिन्हित पूर्णांक UInt32;
प्रकारव्याख्या UInt32 SizeT;

#define CProb UInt16

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

प्रकारव्याख्या संरचना _CLzmaProperties
{
  पूर्णांक lc;
  पूर्णांक lp;
  पूर्णांक pb;
}CLzmaProperties;

पूर्णांक LzmaDecodeProperties(CLzmaProperties *propsRes, स्थिरांक अचिन्हित अक्षर *propsData, पूर्णांक size);

#define Lzma_Get_अंक_Probs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

प्रकारव्याख्या संरचना _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


पूर्णांक LzmaDecode(CLzmaDecoderState *vs,
    स्थिरांक अचिन्हित अक्षर *inStream, SizeT inSize, SizeT *inSizeProcessed,
    अचिन्हित अक्षर *outStream, SizeT outSize, SizeT *outSizeProcessed);

#endif
