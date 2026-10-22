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

#جيڪڏهن_وصف_نه_ٿيل __LZMADECODE_H
#وصف_ڪريو __LZMADECODE_H

تعريفنوع بنانشان حرف Byte;
تعريفنوع بنانشان ننڍو UInt16;
تعريفنوع بنانشان صحيح UInt32;
تعريفنوع UInt32 سائيز_T;

#وصف_ڪريو CProb UInt16

#وصف_ڪريو LZMA_نتيجو_OK 0
#وصف_ڪريو LZMA_نتيجو_معلومات_ERROR 1


#وصف_ڪريو LZMA_بنياد_سائيز 1846
#وصف_ڪريو LZMA_LIT_سائيز 768

#وصف_ڪريو LZMA_PROPERTIES_سائيز 5

تعريفنوع بنية _CLzmaProperties
{
  صحيح lc;
  صحيح lp;
  صحيح pb;
}CLzmaProperties;

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بنانشان حرف *props_معلومات, صحيح سائيز);

#وصف_ڪريو Lzma_Get_انگ_Probs(Properties) (LZMA_بنياد_سائيز + (LZMA_LIT_سائيز << ((Properties)->lc + (Properties)->lp)))

#وصف_ڪريو kLzmaNeedInitId (-2)

تعريفنوع بنية _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بنانشان حرف *اندريون_وهڪرو, سائيز_T اندريون_سائيز, سائيز_T *اندريون_سائيز_Processed,
    بنانشان حرف *ٻاهريون_وهڪرو, سائيز_T ٻاهريون_سائيز, سائيز_T *ٻاهريون_سائيز_Processed);

#جيڪڏهن_پورو
