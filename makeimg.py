from PIL import Image

# Create an image with RGB mode
width, height = 512, 256
image = Image.new("RGB", (width, height), (168, 168, 168))  # RGB values for 0.33 are approximately 84

# Draw a black border
border_width = 4
for x in range(width):
    for y in range(height):
        if x < border_width or x >= width - border_width or y < border_width or y >= height - border_width:
            image.putpixel((x, y), (0, 0, 0))

# Save the image
image.save("pane3.png")