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

#جے_بنا_تعریف __LZMADECODE_H
#تعریف_کرو __LZMADECODE_H

تعريفنوع بنانشانی حرف Byte;
تعريفنوع بنانشانی چھوٹا UInt16;
تعريفنوع بنانشانی صحيح UInt32;
تعريفنوع UInt32 ناپ_T;

#تعریف_کرو CProb UInt16

#تعریف_کرو LZMA_نتیجہ_OK 0
#تعریف_کرو LZMA_نتیجہ_انکڑے_ERROR 1


#تعریف_کرو LZMA_ادھار_ناپ 1846
#تعریف_کرو LZMA_LIT_ناپ 768

#تعریف_کرو LZMA_PROPERTIES_ناپ 5

تعريفنوع بنية _CLzmaProperties
{
  صحيح lc;
  صحيح lp;
  صحيح pb;
}CLzmaProperties;

صحيح LzmaDecodeProperties(CLzmaProperties *propsRes, ثابت بنانشانی حرف *props_انکڑے, صحيح ناپ);

#تعریف_کرو Lzma_Get_عدد_Probs(Properties) (LZMA_ادھار_ناپ + (LZMA_LIT_ناپ << ((Properties)->lc + (Properties)->lp)))

#تعریف_کرو kLzmaNeedInitId (-2)

تعريفنوع بنية _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


صحيح LzmaDecode(CLzmaDecoderState *vs,
    ثابت بنانشانی حرف *وچ_دھارا, ناپ_T وچ_ناپ, ناپ_T *وچ_ناپ_Processed,
    بنانشانی حرف *باہرلا_دھارا, ناپ_T باہرلا_ناپ, ناپ_T *باہرلا_ناپ_Processed);

#جے_انت
