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

প্রকারলেখ unsigned অক্ষর Byte;
প্রকারলেখ unsigned short UInt16;
প্রকারলেখ unsigned পূর্ণসংখ্যা UInt32;
প্রকারলেখ UInt32 আকার_T;

#define CProb UInt16

#define LZMA_ফলাফল_OK 0
#define LZMA_ফলাফল_তথ্য_ERROR 1


#define LZMA_BASE_আকার 1846
#define LZMA_LIT_আকার 768

#define LZMA_PROPERTIES_আকার 5

প্রকারলেখ কাঠামো _CLzmaProperties
{
  পূর্ণসংখ্যা lc;
  পূর্ণসংখ্যা lp;
  পূর্ণসংখ্যা pb;
}CLzmaProperties;

পূর্ণসংখ্যা LzmaDecodeProperties(CLzmaProperties *propsRes, ধ্রুবক unsigned অক্ষর *props_তথ্য, পূর্ণসংখ্যা আকার_2);

#define Lzma_Get_অঙ্ক_Probs(Properties) (LZMA_BASE_আকার + (LZMA_LIT_আকার << ((Properties)->lc + (Properties)->lp)))

#define kLzmaNeedInitId (-2)

প্রকারলেখ কাঠামো _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


পূর্ণসংখ্যা LzmaDecode(CLzmaDecoderState *vs,
    ধ্রুবক unsigned অক্ষর *inStream, আকার_T in_আকার, আকার_T *in_আকার_Processed,
    unsigned অক্ষর *outStream, আকার_T out_আকার, আকার_T *out_আকার_Processed);

#endif
