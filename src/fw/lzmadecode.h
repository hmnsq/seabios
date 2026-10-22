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

#यदि_अपरिभाषितम् __LZMADECODE_H
#परिभाषय __LZMADECODE_H

प्रकारनिर्देश अचिह्नित वर्ण Byte;
प्रकारनिर्देश अचिह्नित ह्रस्व UInt16;
प्रकारनिर्देश अचिह्नित पूर्णाङ्क UInt32;
प्रकारनिर्देश UInt32 आकार_T;

#परिभाषय CProb UInt16

#परिभाषय LZMA_परिणाम_OK 0
#परिभाषय LZMA_परिणाम_दत्त_ERROR 1


#परिभाषय LZMA_आधारः_आकार 1846
#परिभाषय LZMA_LIT_आकार 768

#परिभाषय LZMA_PROPERTIES_आकार 5

प्रकारनिर्देश संरचना _CLzmaProperties
{
  पूर्णाङ्क lc;
  पूर्णाङ्क lp;
  पूर्णाङ्क pb;
}CLzmaProperties;

पूर्णाङ्क LzmaDecodeProperties(CLzmaProperties *propsRes, ध्रुव अचिह्नित वर्ण *props_दत्त, पूर्णाङ्क आकार);

#परिभाषय Lzma_Get_अङ्कः_Probs(Properties) (LZMA_आधारः_आकार + (LZMA_LIT_आकार << ((Properties)->lc + (Properties)->lp)))

#परिभाषय kLzmaNeedInitId (-2)

प्रकारनिर्देश संरचना _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


पूर्णाङ्क LzmaDecode(CLzmaDecoderState *vs,
    ध्रुव अचिह्नित वर्ण *अन्तर्_प्रवाहः, आकार_T अन्तर्_आकार, आकार_T *अन्तर्_आकार_Processed,
    अचिह्नित वर्ण *बहिर्गमनम्_प्रवाहः, आकार_T बहिर्गमनम्_आकार, आकार_T *बहिर्गमनम्_आकार_Processed);

#यदि_अन्तः
