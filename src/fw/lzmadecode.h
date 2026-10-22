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

પ્રકારવ્યાખ્યા unsigned અક્ષર Byte;
પ્રકારવ્યાખ્યા unsigned short UInt16;
પ્રકારવ્યાખ્યા unsigned પૂર્ણાંક UInt32;
પ્રકારવ્યાખ્યા UInt32 SizeT;

#define CProb UInt16

#define LZMA_RESULT_OK 0
#define LZMA_RESULT_DATA_ERROR 1


#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE 768

#define LZMA_PROPERTIES_SIZE 5

પ્રકારવ્યાખ્યા સંરચના _CLzmaProperties
{
  પૂર્ણાંક lc;
  પૂર્ણાંક lp;
  પૂર્ણાંક pb;
}CLzmaProperties;

પૂર્ણાંક LzmaDecodeProperties(CLzmaProperties *propsRes, અચલ unsigned અક્ષર *propsData, પૂર્ણાંક size);

#define Lzma_Get_અંક_Probs(Properties) (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

પ્રકારવ્યાખ્યા સંરચના _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


પૂર્ણાંક LzmaDecode(CLzmaDecoderState *vs,
    અચલ unsigned અક્ષર *inStream, SizeT inSize, SizeT *inSizeProcessed,
    unsigned અક્ષર *outStream, SizeT outSize, SizeT *outSizeProcessed);

#endif
