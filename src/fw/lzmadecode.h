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

#اگر_غیر_تعریف_شدہ __LZMADECODE_H
#تعریف_کرو __LZMADECODE_H

قسمتعریف غیرعلامتی حرف Byte;
قسمتعریف غیرعلامتی مختصر UInt16;
قسمتعریف غیرعلامتی صحیح UInt32;
قسمتعریف UInt32 سائز_T;

#تعریف_کرو CProb UInt16

#تعریف_کرو LZMA_نتیجہ_OK 0
#تعریف_کرو LZMA_نتیجہ_ڈیٹا_ERROR 1


#تعریف_کرو LZMA_بنیاد_سائز 1846
#تعریف_کرو LZMA_LIT_سائز 768

#تعریف_کرو LZMA_PROPERTIES_سائز 5

قسمتعریف ساخت _CLzmaProperties
{
  صحیح lc;
  صحیح lp;
  صحیح pb;
}CLzmaProperties;

صحیح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت غیرعلامتی حرف *props_ڈیٹا, صحیح سائز);

#تعریف_کرو Lzma_Get_عدد_Probs(Properties) (LZMA_بنیاد_سائز + (LZMA_LIT_سائز << ((Properties)->lc + (Properties)->lp)))

#تعریف_کرو kLzmaNeedInitId (-2)

قسمتعریف ساخت _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


صحیح LzmaDecode(CLzmaDecoderState *vs,
    ثابت غیرعلامتی حرف *اندر_دھارا, سائز_T اندر_سائز, سائز_T *اندر_سائز_Processed,
    غیرعلامتی حرف *برآمد_دھارا, سائز_T برآمد_سائز, سائز_T *برآمد_سائز_Processed);

#اگر_ختم
