from PIL import Image
import os

# Открываем ICO и сохраняем как PNG (256x256)
ico_path = os.path.join(os.path.dirname(__file__), 'appicon.ico')
png_path = os.path.join(os.path.dirname(__file__), 'appicon.png')

# Открываем первое изображение из ICO (самое большое)
icon = Image.open(ico_path)
# Сохраняем как PNG
icon.save(png_path, 'PNG')
print(f"PNG created: {png_path}")
