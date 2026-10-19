#!/usr/bin/env python3
"""Offline rig guide from exported C++ poses; never represents device footage."""
import argparse,json,math,subprocess
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont,ImageFilter
ROOT=Path(__file__).resolve().parents[1]
F='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
def font(n):return ImageFont.truetype(F,n)
def vec(a,r):return (math.cos(a)*r,math.sin(a)*r)
def add(a,b):return (a[0]+b[0],a[1]+b[1])
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--timeline',required=True);ap.add_argument('--output',required=True);a=ap.parse_args()
 out=Path(a.output);out.mkdir(parents=True,exist_ok=True)
 poses=json.loads(Path(a.timeline).read_text());source=[]
 for i in range(6):
  im=Image.open(ROOT/f'assets/slash-study/blue-{i}.png').convert('RGBA');tile=Image.new('RGBA',(160,160));tile.alpha_composite(im,((160-im.width)//2,(160-im.height)//2));source.append(tile)
 base=Image.new('RGB',(960,540),'#0d1522');d=ImageDraw.Draw(base)
 d.text((25,17),'A / C   MOTION PROTOTYPE   0.3.0',font=font(23),fill='#deedf7')
 d.text((25,51),'OFFLINE C++ POSE PLAYBACK | BODY IS A RIG GUIDE | NOT ANDROID FOOTAGE',font=font(12),fill='#9caebf')
 labels=[['STEP CUT','REVERSE RISE','THRUST'],['HEAVY CLEAVE','LOW SWEEP','FINISHING RISE']]
 def at(h,m,u):return poses[h*183+m*61+min(60,max(0,round(u*60)))]
 frames=[];fps=30;duration=7
 for f in range(fps*duration):
  im=base.copy().convert('RGBA');d=ImageDraw.Draw(im);time=f/fps
  for h in range(2):
   x=h*480;cycle=.6 if h==0 else .8;local=time%3.5
   index=min(2,int(local/cycle));u=min(1,(local-index*cycle)/cycle)
   p=at(h,index,u);color=(255,211,100) if h else (103,225,255)
   d.rounded_rectangle((x+18,90,x+462,514),12,fill='#142234',outline='#35485e')
   d.text((x+35,108),'C / CHARGED' if h else 'A / NORMAL',font=font(20),fill=color)
   d.text((x+35,139),labels[h][index],font=font(14),fill='#d0dfeb')
   d.line((x+35,424,x+445,424),fill='#4a6078',width=2)
   for grid in range(x+40,x+450,24):d.line((grid,185,grid,420),fill='#1b2d42')
   k=3.4;hip=(x+205,352);shoulder=(hip[0]+p['shift']*k,hip[1]-18*k+p['drop']*k)
   def world(v):return (shoulder[0]+v[0]*k,shoulder[1]+v[1]*k)
   grip=world(p['grip']);tip=world(p['tip']);draw=ImageDraw.Draw(im)
   # Separate native sprite parts are represented schematically, at game scale.
   for side in [-1,1]:
    foot=(hip[0]+side*7*k+p['legShift']*k,424)
    knee=((hip[0]+foot[0])/2+math.sin(p['leg'])*10*k,391)
    draw.line((hip,knee,foot),fill='#718ba0' if side<0 else '#9db3c2',width=14)
   torso=(shoulder[0]-math.sin(p['lean'])*9*k,shoulder[1])
   draw.line((hip,torso),fill='#6d879f',width=32)
   head=(shoulder[0],shoulder[1]-11*k)
   draw.ellipse((head[0]-14,head[1]-15,head[0]+14,head[1]+15),fill='#c6d5db')
   back=add(shoulder,vec(p['back'],10*k));draw.line((shoulder,back),fill='#7796a7',width=9)
   elbow=((shoulder[0]+grip[0])/2-3,(shoulder[1]+grip[1])/2+7)
   draw.line((shoulder,elbow,grip),fill='#d0e0e5',width=10)
   # Native core/halo frames use the same phase and registration parameters.
   if p['fx'] and not(h==0 and index==2):
    tile=source[p['frame']]
    if p['mirrored']:tile=tile.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    alpha=tile.getchannel('A');lum=tile.convert('RGB').convert('L')
    tint=Image.new('RGBA',tile.size,color+(255,));tint.putalpha(Image.eval(alpha,lambda a:int(a*p['opacity']*.85)))
    # Restore texture modulation without changing its alpha registration.
    from PIL import ImageChops
    tint_rgb=ImageChops.multiply(tint.convert('RGB'),Image.merge('RGB',(lum,lum,lum)))
    tint=Image.merge('RGBA',(*tint_rgb.split(),tint.getchannel('A')))
    side=round(160*p['radius']/45*k);tint=tint.resize((side,side)).rotate(-math.degrees(p['rotation']),Image.Resampling.BICUBIC,expand=True)
    glow=tint.filter(ImageFilter.GaussianBlur(5));glow.putalpha(glow.getchannel('A').point(lambda a:int(a*.28)))
    for layer in [glow,tint]:im.alpha_composite(layer,(round(grip[0]-layer.width/2),round(grip[1]-layer.height/2)))
   draw=ImageDraw.Draw(im)
   if h==0 and index==2 and p['phase']==3:
    draw.line((grip,tip),fill=color+(100,),width=15)
   # Blade uses the same fixed length as the Android adapter (31 game pixels).
   draw.line((grip,tip),fill='#9d6335',width=8)
   draw.line((grip,tip),fill='#e2b875',width=3)
   normal=vec(p['blade']+math.pi/2,5*k)
   draw.line(((grip[0]-normal[0],grip[1]-normal[1]),add(grip,normal)),fill='#bb9862',width=4)
   draw.ellipse((grip[0]-4,grip[1]-4,grip[0]+4,grip[1]+4),fill='#e0bf95')
   for move in range(3):
    box=(x+35+move*137,462,x+162+move*137,490)
    draw.rounded_rectangle(box,5,fill='#34475c' if move==index else '#1b2b3c')
    draw.text((box[0]+10,box[1]+7),str(move+1)+' / '+['CUT','RISE','THRUST'][move] if h==0 else str(move+1)+' / '+['CLEAVE','SWEEP','RISE'][move],font=font(10),fill=color)
  frames.append(im.convert('RGB'))
 frames[11].save(out/'motion-preview.png')
 proc=subprocess.Popen(['ffmpeg','-v','error','-y','-f','rawvideo','-pixel_format','rgb24','-video_size','960x540','-framerate',str(fps),'-i','-','-c:v','libx264','-crf','19','-pix_fmt','yuv420p','-movflags','+faststart',str(out/'motion-preview.mp4')],stdin=subprocess.PIPE)
 for frame in frames:proc.stdin.write(frame.tobytes())
 proc.stdin.close();assert proc.wait()==0
 frames[0].save(out/'motion-preview.gif',save_all=True,append_images=frames[2::2],duration=67,loop=0,optimize=True)
 print(out)
if __name__=='__main__':main()
