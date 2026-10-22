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

#अगर_अघोषित __LZMADECODE_H
#घोषणा __LZMADECODE_H

प्रकार अचिन्हित अक्षर Byte;
प्रकार अचिन्हित लघु UInt16;
प्रकार अचिन्हित पूर्णांक UInt32;
प्रकार UInt32 आकार_T;

#घोषणा CProb UInt16

#घोषणा LZMA_परिणाम_OK 0
#घोषणा LZMA_परिणाम_तथ्य_ERROR 1


#घोषणा LZMA_BASE_आकार 1846
#घोषणा LZMA_LIT_आकार 768

#घोषणा LZMA_PROPERTIES_आकार 5

प्रकार काष्ठा _CLzmaProperties
{
  पूर्णांक lc;
  पूर्णांक lp;
  पूर्णांक pb;
}CLzmaProperties;

पूर्णांक LzmaDecodeProperties(CLzmaProperties *propsRes, स्थिर अचिन्हित अक्षर *props_तथ्य, पूर्णांक आकार);

#घोषणा Lzma_Get_अंक_Probs(Properties) (LZMA_BASE_आकार + (LZMA_LIT_आकार << ((Properties)->lc + (Properties)->lp)))

#घोषणा kLzmaNeedInitId (-2)

प्रकार काष्ठा _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


पूर्णांक LzmaDecode(CLzmaDecoderState *vs,
    स्थिर अचिन्हित अक्षर *inStream, आकार_T in_आकार, आकार_T *in_आकार_Processed,
    अचिन्हित अक्षर *outStream, आकार_T out_आकार, आकार_T *out_आकार_Processed);

#पूर्ण_अगर
