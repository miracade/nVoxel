
from itertools import product
from PIL import Image, ImageEnhance


BLOCK_SIZE = BS = 16

def tile_image(img: Image.Image):
	"""Takes a 16x16 image and tiles it into a 64x64 image"""
	ret = Image.new("RGB", (BS*4, BS*4))
	for xy in product(range(0, BS*4, BS), repeat=2):
		ret.paste(img, xy)
	
	return ret

def alter_brightness(img: Image.Image, factor: float):
	"""Alters the brightness of an image by a factor"""
	enhancer = ImageEnhance.Brightness(img)
	return enhancer.enhance(factor)

src_img = Image.open('./assets/blocks.png')
block_count = src_img.width // BS

dst_img = Image.new('RGB', (BS * 4 * 3, BS * 4 * block_count))

for i in range(0, src_img.width, BS):
	tex = src_img.crop((i, 0, i + BS, BS))
	tex = tile_image(tex)

	dst_img.paste(alter_brightness(tex, 1.0), (0, i * 4))
	dst_img.paste(alter_brightness(tex, 0.8), (BS * 4, i * 4))
	dst_img.paste(alter_brightness(tex, 0.6), (BS * 4 * 2, i * 4))

dst_img.show()
dst_img.save('./assets/spritesheet.png')