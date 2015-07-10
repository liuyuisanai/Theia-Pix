from PIL import Image, ImageDraw, ImageFont
import sys
import numpy


def drawText(text, font, path):
	# create dummy image to get text size
	img = Image.new('1', (100, 100), (1))

	# get a drawing context
	d = ImageDraw.Draw(img)	
	size = d.textsize(text, font);
	
	# create image
	img = Image.new('1', size, (1))

	# get a drawing context
	d = ImageDraw.Draw(img)	
	
	# draw
	d.text((0,0), text, font=font, fill=(0))

	img.save(path)

def generateFont(fontPath, fontSize, imagePath):
	# make LucidaGrande 30
	fnt = ImageFont.truetype(fontPath, fontSize)
	outpath = imagePath + "/small_"
	for i in range(ord('a'), ord('z')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
		drawText(l, fnt, filename)

	outpath = imagePath + "/big_"
	for i in range(ord('A'), ord('Z')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
		drawText(l, fnt, filename)

	outpath = imagePath + "/"
	for i in range(ord('0'), ord('9')+1):
		l = str(chr(i))
		filename = outpath + l + ".png"
		drawText(l, fnt, filename)

	drawText("%", fnt, outpath + "percent" + ".png")
	drawText(" ", fnt, outpath + "space" + ".png")
	drawText(".", fnt, outpath + "dot" + ".png")

	print fontPath + " " + str(fontSize) + " is done"

generateFont("screens/fonts/LucidaGrandeBold.ttf", 30, "screens/fonts/LucidaGrande_30")
generateFont("screens/fonts/LucidaGrandeBold.ttf", 15, "screens/fonts/LucidaGrande_15")
generateFont("screens/fonts/LucidaGrandeBold.ttf", 22, "screens/fonts/LucidaGrande_22")
generateFont("screens/fonts/LucidaGrandeBold.ttf", 12, "screens/fonts/LucidaGrande_12")

