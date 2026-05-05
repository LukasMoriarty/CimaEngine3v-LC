#include "IdleZombie.hpp"
#include "MoverZombie.hpp"
#include "AtacarZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"

namespace IVJ {
    IdleZombie::IdleZombie(std::map<std::string, CE::AnimClip> clips, float frame_rate)
        : FSMZombie{},
          m_clips{std::move(clips)},
          max_frame_time{frame_rate},
          cur_frame_time{frame_rate}
    {
        nombre = "IdleZombie";
        if (m_clips.count("idle")) {
            m_clip = m_clips["idle"];
        }
    }

    FSM *IdleZombie::onInputs(Entidad &parent, CE::Vector2D &target) {
        float distancia = parent.getTransformada()->posicion.distancia(target);

        // Si el jugador entra en el rango de detección (200)
        if (distancia < 200.f) {
            return new MoverZombie(m_clips, 0.05f);
        }
        return nullptr;
    }

    void IdleZombie::onEntrar(const Entidad &obj) {
        auto iSprite = obj.getComponente<CE::ISprite>();
        if (!iSprite) return;

        sprite = &iSprite->m_sprite;
        s_w = iSprite->width;
        s_h = iSprite->height;

        auto texSize = iSprite->m_sprite.getTexture().getSize();
        num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

        id_frame = m_clip.first_frame;
        cur_frame_time = max_frame_time;
    }

    void IdleZombie::onSalir(const Entidad& obj) {
        (void) obj;
    }

    void IdleZombie::onUpdate(const Entidad &obj, float dt) {
        (void) obj;
        if (!sprite) return;

        cur_frame_time -= dt;
        if (cur_frame_time <= 0.f) {
            cur_frame_time = max_frame_time;
            // Animación cíclica
            int local = (id_frame - m_clip.first_frame + 1) % std::max(1, m_clip.frame_count);
            id_frame = m_clip.first_frame + local;
        }

        int col = id_frame % num_cols;
        int row = id_frame / num_cols;
        sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
    }
}