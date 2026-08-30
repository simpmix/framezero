from rembg import remove
from PIL import Image

input_path = 'D:/Anti/framezero/README_LOGO.png'
output_path = 'D:/Anti/framezero/README_LOGO_TRANS.png'

print("Opening image...")
input_img = Image.open(input_path)

print("Removing background...")
output_img = remove(input_img)

print("Saving transparent image...")
output_img.save(output_path)
print("Done!")
