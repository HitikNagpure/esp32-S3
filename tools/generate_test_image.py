from PIL import Image, ImageDraw, ImageOps
import os

def process_image(input_image_path):
    print(f"Processing image: {input_image_path}")
    
    # Open and resize image to fit main content area (800x420)
    img = Image.open(input_image_path)
    print(f"Original image size: {img.width}x{img.height}")
    
    # Calculate resize dimensions maintaining aspect ratio
    target_width = 800
    target_height = 420
    
    # Get original aspect ratio
    width_ratio = target_width / img.width
    height_ratio = target_height / img.height
    
    # Use the smaller ratio to ensure image fits within bounds
    ratio = min(width_ratio, height_ratio)
    
    new_width = int(img.width * ratio)
    new_height = int(img.height * ratio)
    print(f"Resizing to: {new_width}x{new_height}")
    
    # Resize image
    img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
    
    # Create new white image of target size
    final_img = Image.new('RGB', (target_width, target_height), 'white')
    
    # Calculate position to center the image
    x = (target_width - new_width) // 2
    y = (target_height - new_height) // 2
    print(f"Centering image at position: ({x}, {y})")
    
    # Paste resized image onto white background
    final_img.paste(img, (x, y))
    
    # Convert to black and white
    print("Converting to black and white...")
    final_img = final_img.convert('L')  # Convert to grayscale
    final_img = final_img.point(lambda x: 0 if x < 128 else 255, '1')  # Direct threshold without inversion
    
    # Flip the image vertically for e-paper display
    final_img = ImageOps.flip(final_img)
    print("Flipped image vertically for e-paper display compatibility")
    
    # Save a preview
    preview_path = "preview.bmp"
    final_img.save(preview_path)
    print(f"Saved preview to: {preview_path}")
    
    return final_img

# Path to your image
input_image = "website.jpg"  # Using the website image

# Process image
if os.path.exists(input_image):
    print(f"Processing image: {input_image}")
    processed_img = process_image(input_image)
else:
    print(f"No image found at {input_image}, creating a sample image")
    # Create a sample image with some shapes
    img = Image.new('RGB', (800, 420), 'white')
    draw = ImageDraw.Draw(img)
    
    # Draw some interesting shapes
    # Center circle
    draw.ellipse([300, 110, 500, 310], outline='black', width=3)
    
    # Diagonal lines
    draw.line([50, 50, 750, 370], fill='black', width=3)
    draw.line([50, 370, 750, 50], fill='black', width=3)
    
    # Rectangles in corners
    draw.rectangle([50, 50, 150, 150], outline='black', width=3)
    draw.rectangle([650, 50, 750, 150], outline='black', width=3)
    draw.rectangle([50, 270, 150, 370], outline='black', width=3)
    draw.rectangle([650, 270, 750, 370], outline='black', width=3)
    
    processed_img = img.convert('1')  # Convert to 1-bit

# Save processed image
output_bmp = "test_image.bmp"
processed_img.save(output_bmp)

print(f"Saved processed image to: {output_bmp}")

# Convert to byte array for Arduino
with open(output_bmp, "rb") as f:
    # Skip BMP header (54 bytes)
    f.seek(54)
    data = f.read()

# Create header file
header_path = "../include/test_image.h"
with open(header_path, "w") as f:
    f.write("#pragma once\n\n")
    f.write("// Test image bitmap data\n")
    f.write("// Size: 800x420 pixels\n\n")
    f.write("const uint8_t TEST_IMAGE[] PROGMEM = {\n    ")
    
    # Write byte array
    bytes_per_line = 16
    for i, byte in enumerate(data):
        f.write(f"0x{byte:02X}")
        if i < len(data) - 1:
            f.write(", ")
        if (i + 1) % bytes_per_line == 0:
            f.write("\n    ")
    f.write("\n};\n")

print(f"Generated header file: {header_path}")