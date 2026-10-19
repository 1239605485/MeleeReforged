"""Regenerate the native premultiplied atlas from the four retained CC0 sources."""
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
a=Image.new('RGBA',(256,64))
for i in range(4):
    im=Image.open(root/'assets/katana'/f'source_{i}.png').convert('RGBA')
    im.thumbnail((60,60),Image.Resampling.LANCZOS)
    a.alpha_composite(im,(i*64+(64-im.width)//2,(64-im.height)//2))
a.save(root/'assets/katana/katana_atlas.png')
b=bytearray(a.tobytes())
for i in range(0,len(b),4):
    for k in range(3):
        b[i+k]=b[i+k]*b[i+3]//255
(root/'src/generated/katana_atlas.hpp').write_text('#pragma once\nnamespace mr {\ninline constexpr int katanaAtlasWidth=256,katanaAtlasHeight=64;\ninline constexpr unsigned char katanaAtlasPixels[]={\n'+',\n'.join(','.join(map(str,b[i:i+64])) for i in range(0,len(b),64))+'\n};\n}\n')
