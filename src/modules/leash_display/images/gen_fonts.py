from PIL import Image, ImageDraw, ImageFont
import sys
import numpy


def drawText(text, font, fontSize, path):
	# create dummy image to get text size
        img = Image.new('1', (100, 100), (1))

	# get a drawing context
	d = ImageDraw.Draw(img)	
	size = d.textsize(text, font);
	
	# create image
        img = Image.new('1', size, (1))

	# get a drawing context
	d = ImageDraw.Draw(img)	
	


        x = 0;
        y = 0;

        # for some reasons some sybmols drawing is incorrect
        # add some correction manually

        if fontSize <= 13 and text == '0':
            y = 1

	# draw
        d.text((x,y), text, font=font, fill=(0))

	img.save(path)

def generateFont(fontPath, fontSize, imagePath):
	# make LucidaGrande 30
	fnt = ImageFont.truetype(fontPath, fontSize)
	outpath = imagePath + "/small_"
	for i in range(ord('a'), ord('z')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
                drawText(l, fnt, fontSize, filename)

	outpath = imagePath + "/big_"
	for i in range(ord('A'), ord('Z')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
                drawText(l, fnt, fontSize, filename)

	outpath = imagePath + "/"
	for i in range(ord('0'), ord('9')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
                drawText(l, fnt, fontSize, filename)

        drawText("%", fnt, fontSize, outpath + "percent" + ".png")
        drawText(" ", fnt, fontSize, outpath + "space" + ".png")
        drawText(".", fnt, fontSize, outpath + "dot" + ".png")

	print fontPath + " " + str(fontSize) + " is done"

generateFont("screens/fonts/LucidaGrandeBold.ttf", 30, "screens/fonts/LucidaGrande_30")
generateFont("screens/fonts/LucidaGrandeBold.ttf", 15, "screens/fonts/LucidaGrande_15")
generateFont("screens/fonts/LucidaGrandeBold.ttf", 22, "screens/fonts/LucidaGrande_22")
generateFont("screens/fonts/LucidaGrandeBold.ttf", 12, "screens/fonts/LucidaGrande_12")

