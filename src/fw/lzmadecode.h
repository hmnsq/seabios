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

#اگر_تعریف_نشده __LZMADECODE_H
#تعریف_کن __LZMADECODE_H

تعريفنوع بیعلامت حرف Byte;
تعريفنوع بیعلامت کوتاه UInt16;
تعريفنوع بیعلامت صحيح UInt32;
تعريفنوع UInt32 اندازه_T;

#تعریف_کن CProb UInt16

#تعریف_کن LZMA_نتیجه_OK 0
#تعریف_کن LZMA_نتیجه_داده_ERROR 1


#تعریف_کن LZMA_پایه_اندازه 1846
#تعریف_کن LZMA_LIT_اندازه 768

#تعریف_کن LZMA_PROPERTIES_اندازه 5

تعريفنوع بنية _CLzmaProperties
{
  صحيح lc;
  صحيح lp;
  صحيح pb;
}CLzmaProperties;

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بیعلامت حرف *props_داده, صحيح اندازه);

#تعریف_کن Lzma_Get_رقم_Probs(Properties) (LZMA_پایه_اندازه + (LZMA_LIT_اندازه << ((Properties)->lc + (Properties)->lp)))

#تعریف_کن kLzmaNeedInitId (-2)

تعريفنوع بنية _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بیعلامت حرف *درون_جریان, اندازه_T درون_اندازه, اندازه_T *درون_اندازه_Processed,
    بیعلامت حرف *بیرون_جریان, اندازه_T بیرون_اندازه, اندازه_T *بیرون_اندازه_Processed);

#پایان_اگر
