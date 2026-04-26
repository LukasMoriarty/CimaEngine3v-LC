#include "IdlePies.hpp"
#include "MoverPies.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    IdlePies::IdlePies(CE::AnimClip clip, float frame_rate)
        : FSM{}, m_clip{clip},
          max_frame_time{frame_rate}, cur_frame_time{frame_rate}
    {
        nombre = "IdlePies";
    }

    FSM* IdlePies::onInputs(const CE::IControl& control)
    {
        if (control.arr || control.abj || control.der || control.izq)
        {
            // La animación walk se pasa desde Escena_Sprite al crear el FSM
            // MoverPies la recibe en su constructor; usamos un clip placeholder
            // que Escena_Sprite reemplaza al crear el estado inicial.
            // En transición, MoverPies debe construirse con el clip correcto.
            // Para eso, compartimos la referencia al mapa en Escena_Sprite.
            // Ver comentario en Escena_Sprite::onInit sobre cómo pasar clips.
            return new MoverPies(CE::AnimClip{61, 20}); // walk: frames 61-80
        }
        return nullptr;
    }

    void IdlePies::onEntrar(const Entidad& obj)
    {
        auto iSprite = obj.getComponente<CE::ISprite>();
        if (!iSprite) return;

        sprite = &iSprite->m_sprite;
        s_w    = iSprite->width;
        s_h    = iSprite->height;

        auto texSize = iSprite->m_sprite.getTexture().getSize();
        num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

        id_frame       = m_clip.first_frame;
        cur_frame_time = max_frame_time;

        // Mostrar frame idle inmediatamente
        int col = id_frame % num_cols;
        int row = id_frame / num_cols;
        sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
    }

    void IdlePies::onSalir(const Entidad& /*obj*/) {}

    void IdlePies::onUpdate(const Entidad& /*obj*/, float dt)
    {
        if (!sprite) return;
        cur_frame_time -= dt;
        if (cur_frame_time > 0.f) return;
        cur_frame_time = max_frame_time;

        // Si solo hay 1 frame, simplemente lo mostramos sin avanzar
        int local = (id_frame - m_clip.first_frame + 1) % std::max(1, m_clip.frame_count);
        id_frame  = m_clip.first_frame + local;

        int col = id_frame % num_cols;
        int row = id_frame / num_cols;
        sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
    }
}