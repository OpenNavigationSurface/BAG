#!/usr/bin/env python

import sys
import os
import PIL.Image

inname = sys.argv[1]
outname = os.path.splitext(inname)[0]+'.png'

print inname, outname

sawHeader = False
done = False

clut = []

for line in file(inname,'r').readlines():
    if not done:
        if not sawHeader:
            if line.startswith('clut1.0'):
                sawHeader = True
        else:
            if line.startswith('done'):
                done = True
            else:
                parts = line.split()
                clut.append((int(parts[1]),int(parts[2]),int(parts[3])))

img = PIL.Image.new("RGB",(len(clut),1),(0,0,0))
for i in range(len(clut)):
    img.putpixel((i,0),clut[i])
    
img.save(outname)
img.show()