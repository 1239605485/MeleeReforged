"""Original polar sweep frames plus retained CC0 charge/hit materials."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter
import math
root=Path(__file__).resolve().parents[1]
a=Image.new('RGBA',(1024,320))
for heavy in [False,True]:
    for frame in range(8):
        u=(frame+1)/8
        end=2.35+((-.30 if heavy else -.22)-2.35)*(1-(1-u)**3)
        begin=2.35+((-.30 if heavy else -.22)-2.35)*(1-(1-max(0,u-.65))**3)
        span=max(.06,begin-end)
        im=Image.new('RGBA',(128,128));pix=im.load()
        for y in range(128):
            for x in range(128):
                dx=x-64;dy=y-64;r=math.hypot(dx,dy)/54
                theta=math.atan2(dy,dx)
                if theta<0:theta+=2*math.pi
                if not (.22<r<1.05 and theta<span):continue
                tail=(1-theta/span)**1.4
                envelope=max(0,min(1,(r-.22)/.5))*max(0,min(1,(1.05-r)/.10))
                grain=.75+.25*math.sin(theta*73+r*51+frame*.8)**2
                streak=.5+.5*math.sin(r*145+theta*9)**8
                outer=.94
                rim=math.exp(-((r-outer)/.012)**2)+.3*math.exp(-((r-.81)/.009)**2)
                core=math.exp(-(theta/.025)**2)
                alpha=int(min(245,(22*tail*grain*streak+165*rim*tail+135*core)*envelope))
                pix[x,y]=(int(80+155*r),int(150+100*r),255,alpha)
        # A restrained soft halo surrounds the textured sweep.
        halo=Image.new('RGBA',(128,128),(110,170,255,0));halo.putalpha(im.getchannel('A').filter(ImageFilter.GaussianBlur(2)).point(lambda v:int(v*.32)))
        halo.alpha_composite(im);a.alpha_composite(halo,(frame*128,128 if heavy else 0))
for cell,source in [(2,2),(3,3)]:
    im=Image.open(root/'assets/katana'/f'source_{source}.png').convert('RGBA');im.thumbnail((60,60),Image.Resampling.LANCZOS)
    a.alpha_composite(im,(cell*64+(64-im.width)//2,256+(64-im.height)//2))
# Original scabbard: dark indigo lacquer, steel edge and warm brass fittings.
sheath=Image.new('RGBA',(64,64));d=ImageDraw.Draw(sheath)
centers=[(x,32+3*math.sin(math.pi*(x-2)/59)) for x in range(2,62)]
polygon=[(x,y-4) for x,y in centers]+[(x,y+4) for x,y in reversed(centers)]
d.polygon(polygon,fill=(17,25,43,255),outline=(81,107,138,255))
d.line([(x,y-3) for x,y in centers],fill=(126,151,174,220),width=1)
for x in [3,12,55]:
    y=32+3*math.sin(math.pi*(x-2)/59);d.line((x,y-4,x,y+4),fill=(190,151,82,255),width=2)
a.alpha_composite(sheath,(256,256));a.save(root/'assets/katana/katana_atlas.png')
b=bytearray(a.tobytes())
for i in range(0,len(b),4):
    for k in range(3):b[i+k]=b[i+k]*b[i+3]//255
(root/'src/generated/katana_atlas.hpp').write_text('#pragma once\nnamespace mr {\ninline constexpr int katanaAtlasWidth=1024,katanaAtlasHeight=320;\ninline constexpr unsigned char katanaAtlasPixels[]={\n'+',\n'.join(','.join(map(str,b[i:i+64])) for i in range(0,len(b),64))+'\n};\n}\n')
