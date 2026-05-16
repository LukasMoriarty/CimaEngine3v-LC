#include "IdlePies.hpp"
#include "MoverPies.hpp"
#include "Motor/Componentes/IComponentes.hpp"
#include <cmath>

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
            if (control.run)
                return new MoverPies(CE::AnimClip{1, 20}, 0.02f); // run: frames 1-20 (más rápido)
            else
                return new MoverPies(CE::AnimClip{61, 20}); // walk: frames 61-80
        }
        return nullptr;
    }

    FSM* IdlePies::onInputs(const CE::IControl& control, float mouse_nx, float mouse_ny)
    {
        // Si no hay movimiento, seguir en idle
        if (!control.arr && !control.abj && !control.der && !control.izq)
            return nullptr;

        // Calcular la dirección de movimiento del control
        float move_x = 0.f, move_y = 0.f;
        if (control.der) move_x += 1.f;
        if (control.izq) move_x -= 1.f;
        if (control.arr) move_y -= 1.f;
        if (control.abj) move_y += 1.f;

        // Normalizar dirección de movimiento
        float move_len = std::sqrt(move_x * move_x + move_y * move_y);
        if (move_len > 0.f) {
            move_x /= move_len;
            move_y /= move_len;
        }

        // Calcular producto punto y perpendicular
        float dot = mouse_nx * move_x + mouse_ny * move_y;
        float perp = mouse_nx * move_y - mouse_ny * move_x;

        CE::AnimClip next_clip{61, 20}; // walk por defecto
        float frame_rate = 0.05f;

        // Decidir qué animación usar
        if (std::abs(perp) > std::abs(dot)) {
            // Movimiento lateral: strafe
            if (perp > 0.f) {
                next_clip = CE::AnimClip{41, 20}; // Strafe derecha
            } else {
                next_clip = CE::AnimClip{21, 20}; // Strafe izquierda
            }
        } else {
            // Movimiento frontal o trasero: walk
            next_clip = CE::AnimClip{61, 20};
        }

        // Si está corriendo, usar run en su lugar
        if (control.run) {
            frame_rate = 0.02f;
        }

        return new MoverPies(next_clip, frame_rate);
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