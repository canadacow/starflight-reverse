from PIL import Image, ImageDraw
import numpy as np
from scipy.ndimage import distance_transform_edt, zoom

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
    img = np.full((7, 5), 255)  # Increased size for border and initialized to 255
    n = 0
    for y in range(1, 6):  # Adjusted range for border
        for x in range(1, 4):  # Adjusted range for border
            img[y, x] = 0 if bitmap & (1 << (15-n)) else 255
            n += 1
    return img

def draw_string(s):
    # Convert the string to uppercase
    s = s.upper()
    # Draw each character and concatenate
    imgs = [draw_char(c) for c in s]
    return np.concatenate(imgs, axis=1)

# Draw the string
img = draw_string('Hello World')

# Convert the image to a binary array
binary_img = img < 128

# Compute the distance transform
dist_transform = distance_transform_edt(binary_img)

# Normalize the distance transform to the range [0, 255]
dist_transform = (dist_transform / dist_transform.max()) * 255

# Scale up the distance transform
scale_factor = 20
large_sdf = zoom(dist_transform, scale_factor)

# Threshold the SDF to create a binary image
large_img = large_sdf > 128

# Convert the binary image back to an 8-bit grayscale image
large_img = Image.fromarray(large_img.astype(np.uint8) * 255)

# Save the original character image, the SDF, and the final output
Image.fromarray(img.astype(np.uint8)).save('original_string.png')
Image.fromarray(dist_transform.astype(np.uint8)).save('sdf_string.png')
large_img.save('large_string.png')