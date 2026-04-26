import glob
import json
import struct
import math
import sys
import os

def obtener_resolucion_png(ruta_archivo):
    """Lee el encabezado binario del PNG para obtener su ancho y alto sin usar librerías externas."""
    with open(ruta_archivo, 'rb') as f:
        f.read(16) # Saltamos la firma del PNG hasta llegar a las dimensiones
        ancho, alto = struct.unpack('>LL', f.read(8))
        return ancho, alto

# 1. Obtener y ordenar los archivos
archivos = sorted(glob.glob("*.png"))

if not archivos:
    print("❌ Error: No se encontraron archivos .png en este directorio.")
    sys.exit(1)

# 2. Validar que todos tengan el mismo tamaño
ancho_base, alto_base = obtener_resolucion_png(archivos[0])
print(f"🔍 Validando {len(archivos)} sprites (Tamaño base: {ancho_base}x{alto_base}px)...")

for archivo in archivos[1:]:
    ancho_actual, alto_actual = obtener_resolucion_png(archivo)
    if (ancho_actual, alto_actual) != (ancho_base, alto_base):
        print(f"❌ Error crítico: El archivo '{archivo}' mide {ancho_actual}x{alto_actual}px y rompe la cuadrícula.")
        sys.exit(1)

# 3. Calcular sugerencias para el Spritesheet (Buscando una cuadrícula cuadrada)
total_sprites = len(archivos)
columnas = math.ceil(math.sqrt(total_sprites))
filas = math.ceil(total_sprites / columnas)

atlas_ancho = columnas * ancho_base
atlas_alto = filas * alto_base

# 4. Construir el JSON
datos_json = {
    "metadata": {
        "sprite_width": ancho_base,
        "sprite_height": alto_base,
        "total_sprites": total_sprites,
        "suggested_grid": f"{columnas}x{filas}",
        "suggested_atlas_size": f"{atlas_ancho}x{atlas_alto}px",
        "montage_command": f"montage *.png -tile {columnas}x -geometry +0+0 -background transparent survivor-atlas.png"
    },
    "frames": {archivo: indice for indice, archivo in enumerate(archivos)}
}

# 5. Guardar el archivo
with open("indices_animacion.json", "w") as f_out:
    json.dump(datos_json, f_out, indent=4)

print("✅ ¡Éxito! Todos los tamaños coinciden.")
print(f"📄 Se ha generado 'indices_animacion.json'.")
print(f"💡 Sugerencia para montage: {datos_json['metadata']['montage_command']}")
