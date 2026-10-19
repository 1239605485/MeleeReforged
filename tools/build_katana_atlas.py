"""Build compact trajectory strips and glints from the retained CC0 sources."""
from pathlib import Path
from PIL import Image,ImageFilter
root=Path(__file__).resolve().parents[1]
a=Image.new('RGBA',(256,64))
for cell,source in [(1,1),(2,2),(3,3)]:
    im=Image.open(root/'assets/katana'/f'source_{source}.png').convert('RGBA')
    im.thumbnail((60,60),Image.Resampling.LANCZOS)
    a.alpha_composite(im,(cell*64+(64-im.width)//2,(64-im.height)//2))
# The halo derives from the same line; a separate arc decal is no longer used.
mask=a.crop((64,0,128,64)).getchannel('A').filter(ImageFilter.GaussianBlur(2))
halo=Image.new('RGBA',(64,64),'white');halo.putalpha(mask)
a.paste(halo,(0,0));a.save(root/'assets/katana/katana_atlas.png')
b=bytearray(a.tobytes())
for i in range(0,len(b),4):
    for k in range(3):
        b[i+k]=b[i+k]*b[i+3]//255
(root/'src/generated/katana_atlas.hpp').write_text('#pragma once\nnamespace mr {\ninline constexpr int katanaAtlasWidth=256,katanaAtlasHeight=64;\ninline constexpr unsigned char katanaAtlasPixels[]={\n'+',\n'.join(','.join(map(str,b[i:i+64])) for i in range(0,len(b),64))+'\n};\n}\n')
