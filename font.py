from PIL import Image, ImageDraw
import numpy as np
from scipy.ndimage import distance_transform_edt, zoom, distance_transform_cdt, distance_transform_bf
import hqx
import snowy

font_table = {
    ' ': 0x0000, '!': 0x4904, '"': 0xB400, '#': 0xFFFF,
    '$': 0xF45E, '%': 0xA54A, '&': 0x0000, '\'': 0x4800,
    '[': 0x2922, ']': 0x8928, '*': 0x1550, '+': 0x0BA0,
    ',': 0x0128, '-': 0x0380, '.': 0x0004, '/': 0x2548,
    '0': 0xF6DE, '1': 0x4924, '2': 0xE7CE, '3': 0xE59E,
    '4': 0xB792, '5': 0xF39E, '6': 0xD3DE, '7': 0xE524,
    '8': 0xF7DE, '9': 0xF792, ':': 0x0820, ';': 0x0828,
    '<': 0x2A22, '=': 0x1C70, '>': 0x88A8, '?': 0xE584,
    '@': 0xFFCE, 'A': 0x57DA, 'B': 0xD75C, 'C': 0x7246,
    'D': 0xD6DC, 'E': 0xF34E, 'F': 0xF348, 'G': 0x7256,
    'H': 0xB7DA, 'I': 0xE92E, 'J': 0x24DE, 'K': 0xB75A,
    'L': 0x924E, 'M': 0xBFDA, 'N': 0xBFFA, 'O': 0x56D4,
    'P': 0xF7C8, 'Q': 0xF7A6, 'R': 0xF7EA, 'S': 0x739C,
    'T': 0xE924, 'U': 0xB6DE, 'V': 0xB6D4, 'W': 0xB7FA,
    'X': 0xB55A, 'Y': 0xB7A4, 'Z': 0xE54E
}

def draw_char(c):
    bitmap = font_table[c]
    img = np.full((8, 7), 255)  # Adjusted size for border and doubled horizontal pixels
    n = 0
    for y in range(1, 6):  # Range for border
        for x in range(1, 7, 2):  # Range for border and step for doubled horizontal pixels
            pixel = 0 if bitmap & (1 << (15-n)) else 255
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
    imgs = [np.hstack([img, np.full((8, 1), 255)]) for img in imgs]
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

# Scale up the distance transform
scale_factor = 2.5
large_sdf = zoom(snowy_sdf, scale_factor)

large_sdf = (large_sdf * 255).astype(np.uint8)

Image.fromarray(large_sdf).save('raw_large_sdf.png')

bottomThreshold = 195
topThreshold = 210

large_sdf[large_sdf < bottomThreshold] = 0
large_sdf[large_sdf > topThreshold] = 255



Image.fromarray(large_sdf).save('large_string.png')

import os
os.system('eog large_string.png &')

