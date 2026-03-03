from PIL import Image, ImageDraw, ImageFont
import os

def create_icon():
    sizes = [256, 128, 64, 48, 32, 16]
    images = []
    
    for size in sizes:
        # Создаём изображение с градиентным фоном
        img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        
        # Радиус скругления
        radius = int(size * 0.2)
        
        # Рисуем фон с градиентом (синий)
        for y in range(size):
            r = int(74 + (42 - 74) * y / size)
            g = int(144 + (95 - 144) * y / size)
            b = int(217 + (143 - 217) * y / size)
            draw.rectangle([(0, y), (size, y+1)], fill=(r, g, b, 255))
        
        # Рисуем скруглённые края (прозрачность по краям)
        mask = Image.new('L', (size, size), 0)
        mask_draw = ImageDraw.Draw(mask)
        mask_draw.rounded_rectangle([(0, 0), (size-1, size-1)], radius=radius, fill=255)
        img.putalpha(mask)
        
        # Рисуем букву "P" белую
        p_width = int(size * 0.12)
        p_height = int(size * 0.5)
        p_x = int(size * 0.28)
        p_y = int(size * 0.25)
        
        # Вертикальная часть P
        draw.rounded_rectangle(
            [(p_x, p_y), (p_x + p_width, p_y + p_height * 2)],
            radius=int(p_width/2),
            fill=(255, 255, 255, 255)
        )
        
        # Верхняя дуга P
        arc_width = int(size * 0.22)
        draw.rounded_rectangle(
            [(p_x + p_width, p_y), (p_x + p_width + arc_width, p_y + p_height)],
            radius=int(p_height/2),
            fill=(255, 255, 255, 255)
        )
        
        # Внутренняя часть дуги (синяя, как фон)
        inner_x1 = p_x + p_width + int(p_height * 0.15)
        inner_y1 = p_y + int(p_height * 0.15)
        inner_x2 = p_x + p_width + arc_width - int(p_height * 0.15)
        inner_y2 = p_y + p_height - int(p_height * 0.15)
        if inner_x1 < inner_x2 and inner_y1 < inner_y2:
            draw.ellipse(
                [(inner_x1, inner_y1), (inner_x2, inner_y2)],
                fill=(74, 144, 217, 255)
            )
        
        # Текст "MD" внизу
        try:
            font = ImageFont.truetype("arialbd.ttf", int(size * 0.18))
        except:
            font = ImageFont.load_default()
        
        text = "MD"
        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
        text_x = (size - text_width) // 2
        text_y = int(size * 0.75)
        draw.text((text_x, text_y), text, fill=(255, 255, 255, 255), font=font)
        
        images.append(img)
    
    # Сохраняем как ICO
    icon_path = os.path.join(os.path.dirname(__file__), 'appicon.ico')
    images[0].save(
        icon_path,
        format='ICO',
        sizes=[(s, s) for s in sizes],
        append_images=images[1:]
    )
    print(f"Icon created: {icon_path}")

if __name__ == '__main__':
    create_icon()
