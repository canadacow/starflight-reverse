from PIL import Image, ImageDraw

# Create an image with RGB mode
width, height = 256, 256
background_color = (int(0.66 * 255), int(0.66 * 255), int(0.66 * 255))  # RGB values for 0.66 are approximately 168
circle_color = (84, 84, 84)  # RGB values for 0.33 are approximately 84
image = Image.new("RGB", (width, height), background_color)

# Draw a grey circle
draw = ImageDraw.Draw(image)
circle_radius = int(0.66 * (width / 2))  # 66% of the half of the image width
circle_center = (width // 2, height // 2)
draw.ellipse((circle_center[0] - circle_radius, circle_center[1] - circle_radius,
              circle_center[0] + circle_radius, circle_center[1] + circle_radius), fill=circle_color)

# Save the image
image.save("base.png")