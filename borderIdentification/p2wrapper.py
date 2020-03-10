import subprocess
from PIL import Image
import sys

if len(sys.argv) < 3:
    'Call should be python p2wrapper.py filename.jpg threshold'
    exit()

fname = sys.argv[1]
threshold = sys.argv[2]

img = Image.open(fname)
rgb_img= img.convert('RGB')
bmap = [[0 for w in range(img.width)] for h in range(img.height)]
maxval = 0
for h in range(img.height):
    for w in range(img.width):
        r, g, b = rgb_img.getpixel((w, h))
        val = r * (256)**2 + g*256 + b
        bmap[h][w] = val
        if val > maxval:
            maxval = val
            
outfname = fname.split('.')[0]+'_map.ppm'

outf = open(outfname,'w')
outf.write("P6\n"+str(img.width)+" "+ str(img.height)+"\n16777215\n")
for h in range(img.height):
    for w in range(img.width):
        bmap[h][w]*3
        outf.write(str(int(bmap[h][w])))
        if w < img.width -1:
            outf.write(' ')
    outf.write("\n")

result = subprocess.run(['./bi', outfname,threshold], stdout=subprocess.PIPE)
print(result.stdout.decode('utf-8'))