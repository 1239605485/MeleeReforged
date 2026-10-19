#!/usr/bin/env python3
"""Offline sprite/trajectory study. This is not Android gameplay footage."""
import argparse,json,math,subprocess
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw,ImageFont,ImageFilter
ROOT=Path(__file__).resolve().parents[1]
FONT='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
def font(n):return ImageFont.truetype(FONT,n)
def fade(im,a):
 im=im.copy();im.putalpha(im.getchannel('A').point(lambda v:int(v*max(0,min(1,a)))));return im
def paste_at(dst,im,xy):dst.alpha_composite(im,(round(xy[0]-im.width/2),round(xy[1]-im.height/2)))
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--timeline',required=True);ap.add_argument('--output',required=True);a=ap.parse_args()
 timeline=json.loads(Path(a.timeline).read_text());out=Path(a.output);out.mkdir(parents=True,exist_ok=True)
 atlas=[Image.open(ROOT/f'assets/slash-study/blue-{i}.png').convert('RGBA') for i in range(6)]
 processed=[]
 for src in atlas:
  # Canvas registration and sampling are renderer operations; source PNGs stay unchanged.
  canvas=Image.new('RGBA',(160,160));canvas.alpha_composite(src,((160-src.width)//2,(160-src.height)//2))
  ar=np.array(canvas);bright=ar[:,:,:3].max(axis=2).astype(float)/255
  rim=ar.copy();rim[:,:,:3]=[220,250,255];rim[:,:,3]=(ar[:,:,3]*(np.clip((bright-.6)/.4,0,1))*.6).astype('uint8')
  processed.append((canvas,Image.fromarray(rim)))
 base=Image.new('RGBA',(960,480),(13,20,32,255));d=ImageDraw.Draw(base)
 d.text((26,14),'SLASH TEXTURE + TRAJECTORY STUDY',font=font(21),fill='#d7f4ff')
 d.text((26,44),'Offline playback / not Android footage / silhouette is a scale guide',font=font(12),fill='#839aaf')
 for x in [0,480]:
  d.rounded_rectangle((x+18,78,x+462,430),12,fill='#111e2d',outline='#344a60',width=1)
  for gx in range(x+30,x+460,24):d.line((gx,105,gx,398),fill='#18283a')
  for gy in range(110,400,24):d.line((x+30,gy,x+450,gy),fill='#18283a')
  d.line((x+32,344,x+448,344),fill='#3b5366',width=2)
  d.text((x+36,92),'REALTIME' if x==0 else 'SLOW MOTION  x0.33',font=font(16),fill='#a0edff')
  # Deliberately plain scale guide, not a replacement character animation.
  d.ellipse((x+186,210,x+214,238),fill='#2e4052')
  d.rounded_rectangle((x+183,237,x+216,299),7,fill='#2e4052')
  d.line((x+192,298,x+183,342),fill='#2e4052',width=12)
  d.line((x+208,298,x+218,342),fill='#2e4052',width=12)
  d.line((x+211,245,x+225,251),fill='#4f667b',width=10)
  d.text((x+36,404),'Anticipation  >  Cut  >  Follow-through',font=font(12),fill='#839aaf')
 frames=[];duration=8;fps=30
 def pose(t,slow):
  local=(t*(1/3 if slow else 1))%1.25
  if local>.60:return None
  return timeline[min(119,round(local/.60*119))]
 for frame in range(duration*fps):
  t=frame/fps;im=base.copy()
  for panel in range(2):
   x=panel*480;p=pose(t,panel==1)
   if p is None:continue
   pivot=(x+225,251);k=2.2
   if p['visible']:
    src,rim=processed[p['frame']];side=round(p['radius']*2*k*80/45)
    core=src.resize((side,side),Image.Resampling.LANCZOS).rotate(-math.degrees(p['rotation']),Image.Resampling.BICUBIC,expand=True)
    edge=rim.resize((side,side),Image.Resampling.LANCZOS).rotate(-math.degrees(p['rotation']),Image.Resampling.BICUBIC,expand=True)
    paste_at(im,fade(core.filter(ImageFilter.GaussianBlur(5)),p['glow']*1.4),pivot)
    paste_at(im,fade(core.rotate(14,Image.Resampling.BICUBIC,expand=True),p['opacity']*.20),pivot)
    paste_at(im,fade(core,p['opacity']),pivot)
    paste_at(im,fade(edge,p['opacity']),pivot)
   # Wooden blade anchored to the shared C++ trajectory, with two faint afterimages.
   draw=ImageDraw.Draw(im)
   for lag,opacity in [(12,28),(6,50),(0,255)]:
    idx=max(0,round(p['u']*119)-lag);angle=timeline[idx]['angle']
    if lag and p['phase']!=3:continue
    v=np.array([math.cos(angle),math.sin(angle)]);n=np.array([-v[1],v[0]])
    h=np.array(pivot);tip=h+v*42*k
    pts=[tuple(h+n*3),tuple(tip-v*7+n*3),tuple(tip),tuple(tip-v*7-n*3),tuple(h-n*3)]
    layer=Image.new('RGBA',im.size);ld=ImageDraw.Draw(layer)
    ld.polygon(pts,fill=(173,112,48,opacity));ld.line((tuple(h),tuple(tip-v*6)),fill=(240,193,103,opacity),width=2)
    ld.line((tuple(h-n*7),tuple(h+n*7)),fill=(120,78,43,opacity),width=3)
    ld.line((tuple(h-v*9),tuple(h)),fill=(114,65,30,opacity),width=5)
    im.alpha_composite(layer)
   draw=ImageDraw.Draw(im);draw.ellipse((pivot[0]-3,pivot[1]-3,pivot[0]+3,pivot[1]+3),fill='#f3d59d')
   if .40<p['u']<.74:
    age=(p['u']-.4)/.34;alpha=int(190*(1-age))
    for j in range(7):
     theta=j*2.39996;center=np.array(pivot)+np.array(p['tip'])*k*.85
     pos=center+np.array([math.cos(theta),math.sin(theta)])*(8+age*35)*k
     layer=Image.new('RGBA',im.size);ld=ImageDraw.Draw(layer)
     end=pos+np.array([math.cos(theta),math.sin(theta)])*(5*(1-age)+1)
     ld.line((tuple(pos),tuple(end)),fill=(160,240,255,alpha),width=2);im.alpha_composite(layer)
   label='WINDUP' if p['u']<.22 else ('CUT' if p['u']<.62 else 'RECOVERY')
   ImageDraw.Draw(im).text((x+340,93),label,font=font(11),fill='#b7d9e8')
  frames.append(im.convert('RGB'))
 frames[8].save(out/'slash-study-still.png')
 # Both deliverables use these exact same frames.
 proc=subprocess.Popen(['ffmpeg','-v','error','-y','-f','rawvideo','-pixel_format','rgb24','-video_size','960x480','-framerate',str(fps),'-i','-','-c:v','libx264','-crf','19','-pix_fmt','yuv420p','-movflags','+faststart',str(out/'slash-study.mp4')],stdin=subprocess.PIPE)
 for im in frames:proc.stdin.write(im.tobytes())
 proc.stdin.close();assert proc.wait()==0
 frames[::2][0].save(out/'slash-study.gif',save_all=True,append_images=frames[2::2],duration=67,loop=0,optimize=True)
 print(out.resolve())
if __name__=='__main__':main()
