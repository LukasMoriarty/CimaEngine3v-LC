from PIL import Image, ImageDraw, ImageFont
from sys import argv

if __name__ == "__main__":
    if len(argv)< 3:
        print(f"Modo de usarse (solo tiles cuadrados)\ntile_over.py imagen.png tile_dim")
        exit(-1)
    # Load image
    img = Image.open(argv[1])
    w, h = img.size

    print(f"Imagen dim: {w}x{h}")

    tile = int(argv[2])
    # Create overlay
    draw = ImageDraw.Draw(img)

    # Try a simple font
    try:
        font = ImageFont.truetype("DejaVuSans-Bold.ttf", 24)
    except:
        font = ImageFont.load_default()

    count = 0
    for y in range(0, h, tile):
        for x in range(0, w, tile):
            # rectangle
            draw.rectangle([x, y, x+tile, y+tile], outline="red", width=1)
            # number
            draw.text((x+5, y+5), str(count), fill="yellow", font=font)
            count += 1

    str_split = argv[1].split(".")
    out_path = str_split[0]+'_tiled.'+str_split[1]
    img.save(out_path)
    out_path
