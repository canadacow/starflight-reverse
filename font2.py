from PIL import Image, ImageDraw
import numpy as np
from scipy.ndimage import distance_transform_edt, zoom, distance_transform_cdt, distance_transform_bf
import hqx
import snowy

font_table = {
    ' ': (0x0000, 0x0000, 0x0000), '!': (0x0000, 0x0000, 0x0000),
    '"': (0x0000, 0x0000, 0x0000), '#': (0x0000, 0x0000, 0x0000),
    '$': (0x0000, 0x0000, 0x0000), '%': (0x0000, 0x0000, 0x0000),
    '&': (0x0000, 0x0000, 0x0000), '\'': (0x6F00, 0x0000, 0x0000),
    '[': (0x0000, 0x0000, 0x0000), ']': (0x0000, 0x0000, 0x0000),
    '*': (0x0000, 0x0000, 0x0000), '+': (0x0000, 0x0000, 0x0000),
    ',': (0x0006, 0xF000, 0x0000), '-': (0x0070, 0x0000, 0x0000),
    '.': (0x0200, 0x0000, 0x0000), '/': (0x0000, 0x0000, 0x0000),
    '0': (0x76F7, 0xBDED, 0xC000), '1': (0x6718, 0xC633, 0xC000),
    '2': (0x76C6, 0x6663, 0xE000), '3': (0x76C6, 0x61ED, 0xC000),
    '4': (0x35AD, 0x6F98, 0xC000), '5': (0xFE31, 0xE1ED, 0xC000),
    '6': (0x76F1, 0xEDED, 0xC000), '7': (0xFEC6, 0x6631, 0x8000),
    '8': (0x76F6, 0xEDED, 0xC000), '9': (0x76F6, 0xF1ED, 0xC000),
    ':': (0x1400, 0x0000, 0x0000), ';': (0x0000, 0x0000, 0x0000),
    '<': (0x0000, 0x0000, 0x0000), '=': (0x0000, 0x0000, 0x0000),
    '>': (0x0000, 0x0000, 0x0000), '?': (0x0000, 0x0000, 0x0000),
    '@': (0x0000, 0x0000, 0x0000), 'A': (0x3673, 0x9FE7, 0x2000),
    'B': (0xEDDE, 0xDDE7, 0x0000), 'C': (0x34CC, 0xC430, 0x0000),
    'D': (0xEDDD, 0xDDE0, 0x0000), 'E': (0xFCCE, 0xCCF0, 0x0000),
    'F': (0xFCCE, 0xCCC0, 0x0000), 'G': (0x3231, 0x8DA4, 0xC000),
    'H': (0xCE73, 0xFCE7, 0x2000), 'I': (0xF666, 0x66F0, 0x0000),
    'J': (0x3333, 0x3BF0, 0x0000), 'K': (0xDDDD, 0xEDD0, 0x0000),
    'L': (0xCCCC, 0xCCF0, 0x0000), 'M': (0xC71E, 0xF5C7, 0x1C40),
    'N': (0xCE7B, 0xBCE7, 0x3900), 'O': (0x3273, 0x9CE5, 0xC000),
    'P': (0xFDDD, 0xFCC0, 0x0000), 'Q': (0x312C, 0xB2D9, 0x2340),
    'R': (0xFDDD, 0xEDD0, 0x0000), 'S': (0x7CC6, 0x33E0, 0x0000),
    'T': (0xF666, 0x6660, 0x0000), 'U': (0xCE73, 0x9CE7, 0xE000),
    'V': (0xCE73, 0x9CE4, 0xC000), 'W': (0xC71C, 0x71D7, 0xF280),
    'X': (0xDDD2, 0xDDD0, 0x0000), 'Y': (0xDDDD, 0xF660, 0x0000),
    'Z': (0xF324, 0xCCF0, 0x0000)
}

char_width_table = {
    ' ': 4, '!': 4, '"': 4, '#': 4,
    '$': 4, '%': 4, '&': 5, '\'': 3,
    '[': 4, ']': 4, '*': 4, '+': 4,
    ',': 3, '-': 3, '.': 1, '/': 4,
    '0': 5, '1': 5, '2': 5, '3': 5,
    '4': 5, '5': 5, '6': 5, '7': 5,
    '8': 5, '9': 5, ':': 1, ';': 1,
    '<': 4, '=': 4, '>': 4, '?': 4,
    '@': 4, 'A': 5, 'B': 4, 'C': 4,
    'D': 4, 'E': 4, 'F': 4, 'G': 5,
    'H': 5, 'I': 4, 'J': 4, 'K': 4,
    'L': 4, 'M': 6, 'N': 5, 'O': 5,
    'P': 4, 'Q': 6, 'R': 4, 'S': 4,
    'T': 4, 'U': 5, 'V': 5, 'W': 6,
    'X': 4, 'Y': 4, 'Z': 4
}

font_size_actual = 6
font_size_width = font_size_actual * 2 # Pixel doubled on x axis
font_size_height = 7

font_left_padding = 1
font_right_padding = 1

font_top_padding = 1
font_bottom_padding = 1

def draw_char(c):
    bitmap = font_table[c]
    img = np.full((font_top_padding + font_size_height + font_bottom_padding, font_left_padding + font_size_width + font_right_padding), 255)  # Adjusted size for border and doubled horizontal pixels
    n = 0
    for y in range(font_top_padding, font_size_height + font_top_padding):  # Range for border
        for x in range(font_left_padding, (char_width_table[c] * 2) + font_left_padding, 2):  # Range for border and step for doubled horizontal pixels
            i = n // 16
            j = n % 16
            # Get the integer
            integer = bitmap[i]
            # Get the bit
            pixel = 0 if (bitmap[i] & (1 << (15-j))) else 255
            img[y, x] = pixel
            img[y, x+1] = pixel  # Duplicate the pixel horizontally
            n += 1
    return img

def draw_string(s):
    # Convert the string to uppercase
    s = s.upper()
    # Draw each character and concatenate
    imgs = [draw_char(c) for c in s]
    # Add a column of white pixels between each character
    imgs = [np.hstack([img, np.full((font_top_padding + font_size_height + font_bottom_padding, 1), 255)]) for img in imgs]
    return np.concatenate(imgs, axis=1)

# Draw the string
# Create a texture atlas of all the characters in the font
atlas = [draw_string(c) for c in font_table.keys()]

# Split the string into four rows of characters
rows = []
for i in range(0, len(atlas), len(atlas)//4):
    row = atlas[i:i+len(atlas)//4]
    # Add empty characters to the final row if necessary
    while len(row) < len(atlas)//4:
        row.append(draw_string(' '))
    rows.append(np.concatenate(row, axis=1))

# Concatenate all the rows vertically
img = np.concatenate(rows, axis=0)

# Save the texture atlas
Image.fromarray(img.astype(np.uint8)).save('original_string.png')

pil_img = Image.fromarray(img.astype(np.uint8))
hqx_img = hqx.hq4x(pil_img)

np_img = np.array(hqx_img.convert('L'))

# Convert the image to a binary array
binary_img = np_img # < 254

binary_img = np.expand_dims(binary_img, axis=-1).astype(bool)

snowy_sdf = snowy.unitize(snowy.generate_sdf(binary_img != 0.0))

snowy_sdf = np.squeeze(snowy_sdf, axis=-1)

raw_sdf = (snowy_sdf * 255).astype(np.uint8)

Image.fromarray(raw_sdf).save('FONT2_sdf.png')

# Scale up the distance transform
scale_factor = 2.5
large_sdf = zoom(snowy_sdf, scale_factor)

large_sdf = (large_sdf * 255).astype(np.uint8)

Image.fromarray(large_sdf).save('raw_large_sdf.png')

bottomThreshold = 195
topThreshold = 210

large_sdf[large_sdf < bottomThreshold] = 0
large_sdf[large_sdf > topThreshold] = 255

mask = (large_sdf >= bottomThreshold) & (large_sdf <= topThreshold)
large_sdf[mask] = ((large_sdf[mask] - bottomThreshold) / (topThreshold - bottomThreshold)) * 255

Image.fromarray(large_sdf).save('large_string.png')

import os
os.system('eog large_string.png &')

