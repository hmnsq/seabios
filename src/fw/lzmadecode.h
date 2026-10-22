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

#నిర్వచితం_కాకపోతే __LZMADECODE_H
#నిర్వచించు __LZMADECODE_H

రకనిర్వచనం గుర్తులేని అక్షరం Byte;
రకనిర్వచనం గుర్తులేని పొట్టి UInt16;
రకనిర్వచనం గుర్తులేని పూర్ణాంకం UInt32;
రకనిర్వచనం UInt32 కొలత_T;

#నిర్వచించు CProb UInt16

#నిర్వచించు LZMA_ఫలితం_OK 0
#నిర్వచించు LZMA_ఫలితం_దత్తాంశం_ERROR 1


#నిర్వచించు LZMA_పునాది_కొలత 1846
#నిర్వచించు LZMA_LIT_కొలత 768

#నిర్వచించు LZMA_PROPERTIES_కొలత 5

రకనిర్వచనం నిర్మాణం _CLzmaProperties
{
  పూర్ణాంకం lc;
  పూర్ణాంకం lp;
  పూర్ణాంకం pb;
}CLzmaProperties;

పూర్ణాంకం LzmaDecodeProperties(CLzmaProperties *propsRes, స్థిరరాశి గుర్తులేని అక్షరం *props_దత్తాంశం, పూర్ణాంకం కొలత);

#నిర్వచించు Lzma_Get_అంకె_Probs(Properties) (LZMA_పునాది_కొలత + (LZMA_LIT_కొలత << ((Properties)->lc + (Properties)->lp)))

#నిర్వచించు kLzmaNeedInitId (-2)

రకనిర్వచనం నిర్మాణం _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


పూర్ణాంకం LzmaDecode(CLzmaDecoderState *vs,
    స్థిరరాశి గుర్తులేని అక్షరం *లోపల_ప్రవాహం, కొలత_T లోపల_కొలత, కొలత_T *లోపల_కొలత_Processed,
    గుర్తులేని అక్షరం *బహిర్గతం_ప్రవాహం, కొలత_T బహిర్గతం_కొలత, కొలత_T *బహిర్గతం_కొలత_Processed);

#అయితే_ముగింపు
