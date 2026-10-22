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

#که_نه_تعریف_شوی __LZMADECODE_H
#تعریف_کړه __LZMADECODE_H

تعريفنوع بینښه حرف Byte;
تعريفنوع بینښه لنډ UInt16;
تعريفنوع بینښه صحيح UInt32;
تعريفنوع UInt32 اندازه_T;

#تعریف_کړه CProb UInt16

#تعریف_کړه LZMA_پایله_OK 0
#تعریف_کړه LZMA_پایله_معلومات_ERROR 1


#تعریف_کړه LZMA_بنسټ_اندازه 1846
#تعریف_کړه LZMA_LIT_اندازه 768

#تعریف_کړه LZMA_PROPERTIES_اندازه 5

تعريفنوع بنية _CLzmaProperties
{
  صحيح lc;
  صحيح lp;
  صحيح pb;
}CLzmaProperties;

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بینښه حرف *props_معلومات, صحيح اندازه);

#تعریف_کړه Lzma_Get_عدد_Probs(Properties) (LZMA_بنسټ_اندازه + (LZMA_LIT_اندازه << ((Properties)->lc + (Properties)->lp)))

#تعریف_کړه kLzmaNeedInitId (-2)

تعريفنوع بنية _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بینښه حرف *دننه_بهیر, اندازه_T دننه_اندازه, اندازه_T *دننه_اندازه_Processed,
    بینښه حرف *بهر_بهیر, اندازه_T بهر_اندازه, اندازه_T *بهر_اندازه_Processed);

#د_که_پای
