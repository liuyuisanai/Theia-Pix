import png
import numpy
import textwrap
import glob

from PIL import Image

filelist = [
	'screens/left.png',
	'screens/right.png',
	'screens/play.png',
	'screens/pause.png',
	'screens/play_pause.png',
	'screens/spot.png',
	'screens/home.png',
	'screens/abs.png',
	'screens/path.png',
	'screens/logo.png',
	'screens/lnd_fol.png',
	'screens/battery.png',
	'screens/battery_segment.png',	
	];

def addPath(path):
	for f in glob.glob(path):
		filelist.append(f)

def addFont(path):
	outpath = path + "/small_"
	for i in range(ord('a'), ord('z')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
		filelist.append(filename)
	
	outpath = path + "/big_"
	for i in range(ord('A'), ord('Z')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
		filelist.append(filename)
	
	outpath = path + "/"
	for i in range(ord('0'), ord('9')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
		filelist.append(filename)

	filelist.append(outpath + "percent" + ".png")
	filelist.append(outpath + "space" + ".png")
	filelist.append(outpath + "dot" + ".png")

addFont("screens/fonts/LucidaGrande_30")
addFont("screens/fonts/LucidaGrande_15")
addFont("screens/fonts/LucidaGrande_22")
addFont("screens/fonts/LucidaGrande_12")
addPath("screens/icons/*.png")

def getPngData(filename):

	col = Image.open(filename)
	gray = col.convert('L')
	bw = numpy.asarray(gray).copy()

	packed = numpy.invert(numpy.packbits(numpy.array(list(bw))))

	return gray.size[0], gray.size[1], len(packed), ", ".join(map(str, packed))

defines = "";
imageInfo = "";
totalSize = 0;
imageData = "";

i = 0
for filename in filelist:
	print 'converting',filename

	w, h, size, data = getPngData(filename)

        defines += '#define IMAGE_' + filename.replace("/", "_").split('.', 1)[0].upper() + ' ' + str(i) + '\n'

	if i > 0:
		imageData += ', '
		imageInfo += ', '

	imageInfo += '{' + str(w) + ',' + str(h) + ',' + str(totalSize) + '}'
	imageData += data 
	totalSize += size
	i += 1


imageInfo = "\n".join(textwrap.wrap(imageInfo, 80))

imageInfo = 'const struct ImageInfo imageInfo[] = {' + imageInfo + '};'

imageData = "\n".join(textwrap.wrap(imageData, 80))

imageData = 'const unsigned char imageData[] = {\n' + imageData + '\n};'

defines += '\n'
imageInfo += '\n'
imageData += '\n'


f=open('images.h', 'w');
f.write('#ifndef _IMAGES_H_\n');
f.write('#define _IMAGES_H_\n');
f.write('#ifdef __cplusplus\nextern "C" {\n#endif\n');
f.write(defines);
f.write('struct ImageInfo {\n\tunsigned int w;\n\tunsigned int h;\n\tunsigned int offset;\n};\n');
f.write('extern const struct ImageInfo imageInfo[];\n');
f.write('extern const unsigned char imageData[];\n');

f.write('#ifdef __cplusplus\n}\n#endif\n');
f.write('#endif\n');
f.close();

f=open('images.c', 'w');
f.write('#include "images.h"\n');
f.write(imageInfo);
f.write(imageData);
f.close();

#print size, data

print 'done'
