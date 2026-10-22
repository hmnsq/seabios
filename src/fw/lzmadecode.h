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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LZMADECODE_H
#ವ್ಯಾಖ್ಯಾನಿಸು __LZMADECODE_H

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ Byte;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ UInt16;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ UInt32;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ UInt32 ಗಾತ್ರ_T;

#ವ್ಯಾಖ್ಯಾನಿಸು CProb UInt16

#ವ್ಯಾಖ್ಯಾನಿಸು LZMA_ಫಲಿತಾಂಶ_OK 0
#ವ್ಯಾಖ್ಯಾನಿಸು LZMA_ಫಲಿತಾಂಶ_ದತ್ತಾಂಶ_ERROR 1


#ವ್ಯಾಖ್ಯಾನಿಸು LZMA_ತಳ_ಗಾತ್ರ 1846
#ವ್ಯಾಖ್ಯಾನಿಸು LZMA_LIT_ಗಾತ್ರ 768

#ವ್ಯಾಖ್ಯಾನಿಸು LZMA_PROPERTIES_ಗಾತ್ರ 5

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ _CLzmaProperties
{
  ಪೂರ್ಣಾಂಕ lc;
  ಪೂರ್ಣಾಂಕ lp;
  ಪೂರ್ಣಾಂಕ pb;
}CLzmaProperties;

ಪೂರ್ಣಾಂಕ LzmaDecodeProperties(CLzmaProperties *propsRes, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *props_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2);

#ವ್ಯಾಖ್ಯಾನಿಸು Lzma_Get_ಅಂಕೆ_Probs(Properties) (LZMA_ತಳ_ಗಾತ್ರ + (LZMA_LIT_ಗಾತ್ರ << ((Properties)->lc + (Properties)->lp)))

#ವ್ಯಾಖ್ಯಾನಿಸು kLzmaNeedInitId (-2)

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ _CLzmaDecoderState
{
  CLzmaProperties Properties;
  CProb *Probs;


} CLzmaDecoderState;


ಪೂರ್ಣಾಂಕ LzmaDecode(CLzmaDecoderState *vs,
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ_ಹರಿವು, ಗಾತ್ರ_T ಒಳ_ಗಾತ್ರ, ಗಾತ್ರ_T *ಒಳ_ಗಾತ್ರ_Processed,
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಹೊರಹೋಗು_ಹರಿವು, ಗಾತ್ರ_T ಹೊರಹೋಗು_ಗಾತ್ರ, ಗಾತ್ರ_T *ಹೊರಹೋಗು_ಗಾತ್ರ_Processed);

#ಆದರೆ_ಅಂತ್ಯ
