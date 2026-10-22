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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LZMADECODE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __LZMADECODE_H

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ Byte;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ UInt16;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ UInt32;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ UInt32 ਮਾਪ_T;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CProb UInt16

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LZMA_ਨਤੀਜਾ_OK 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LZMA_ਨਤੀਜਾ_ਅੰਕੜੇ_ERROR 1


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LZMA_ਅਧਾਰ_ਮਾਪ 1846
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LZMA_LIT_ਮਾਪ 768

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LZMA_PROPERTIES_ਮਾਪ 5

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ _CLzmaProperties
{
  ਪੂਰਨ lc;
  ਪੂਰਨ lp;
  ਪੂਰਨ pb;
}CLzmaProperties;

ਪੂਰਨ LzmaDecodeProperties(CLzmaProperties *propsRes, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *props_ਅੰਕੜੇ, ਪੂਰਨ ਮਾਪ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Lzma_Get_ਅੰਕ_Probs(Properties) (LZMA_ਅਧਾਰ_ਮਾਪ + (LZMA_LIT_ਮਾਪ << ((Properties)->lc + (Properties)->lp)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ kLzmaNeedInitId (-2)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


ਪੂਰਨ LzmaDecode(CLzmaDecoderState *vs,
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ_ਧਾਰਾ, ਮਾਪ_T ਵਿੱਚ_ਮਾਪ, ਮਾਪ_T *ਵਿੱਚ_ਮਾਪ_Processed,
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਬਾਹਰਲਾ_ਧਾਰਾ, ਮਾਪ_T ਬਾਹਰਲਾ_ਮਾਪ, ਮਾਪ_T *ਬਾਹਰਲਾ_ਮਾਪ_Processed);

#ਜੇ_ਅੰਤ
