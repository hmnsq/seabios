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

തരനിർവചനം unsigned അക്ഷരം Byte;
തരനിർവചനം unsigned short UInt16;
തരനിർവചനം unsigned പൂർണ്ണസംഖ്യ UInt32;
തരനിർവചനം UInt32 SizeT;

#define CProb UInt16

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

തരനിർവചനം ഘടന _CLzmaProperties
{
  പൂർണ്ണസംഖ്യ lc;
  പൂർണ്ണസംഖ്യ lp;
  പൂർണ്ണസംഖ്യ pb;
}CLzmaProperties;

പൂർണ്ണസംഖ്യ LzmaDecodeProperties(CLzmaProperties *propsRes, സ്ഥിരം unsigned അക്ഷരം *propsData, പൂർണ്ണസംഖ്യ size);

#define Lzma_Get_അക്കം_Probs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

തരനിർവചനം ഘടന _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


പൂർണ്ണസംഖ്യ LzmaDecode(CLzmaDecoderState *vs,
    സ്ഥിരം unsigned അക്ഷരം *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned അക്ഷരം *outStream, SizeT outSize, SizeT *outSizeProcessed);

#endif
