#include "AtacarZombie.hpp"
#include "IdleZombie.hpp"
#include "MoverZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"

namespace IVJ {
    AtacarZombie::AtacarZombie(std::map<std::string, CE::AnimClip> clips, float frame_rate)
        : FSMZombie{},
          m_clips{std::move(clips)},
          max_frame_time{frame_rate},
          cur_frame_time{frame_rate}
    {
        nombre = "AtacarZombie";
        if (m_clips.count("attack")) {
            m_clip = m_clips["attack"];
        }
    }

    FSM *AtacarZombie::onInputs(Entidad &parent, CE::Vector2D &target) {
        auto pos = parent.getTransformada()->posicion;
        float distancia = pos.distancia(target);

        // Solo transicionamos si la animación de ataque ya se completó
        if (ataque_completo) {
            if (distancia > 80.f) {
                return new MoverZombie(m_clips, 0.05f);
            }
            return new IdleZombie(m_clips, 0.1f);
        }
        return nullptr;
    }

    void AtacarZombie::onEntrar(const Entidad &obj) {
        auto iSprite = obj.getComponente<CE::ISprite>();
        if (!iSprite) return;

        sprite = &iSprite->m_sprite;
        s_w = iSprite->width;
        s_h = iSprite->height;

        auto texSize = iSprite->m_sprite.getTexture().getSize();
        num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

        id_frame = m_clip.first_frame;
        cur_frame_time = max_frame_time;
        ataque_completo = false;
    }

    void AtacarZombie::onSalir(const Entidad& obj) {
        (void) obj;
    }

    void AtacarZombie::onUpdate(const Entidad &obj, float dt) {
        (void) obj;
        if (!sprite || ataque_completo) return;

        cur_frame_time -= dt;
        if (cur_frame_time <= 0.f) {
            cur_frame_time = max_frame_time;
            id_frame++;

            // Animación lineal, no cíclica
            if (id_frame >= m_clip.first_frame + m_clip.frame_count) {
                ataque_completo = true;
                id_frame = m_clip.first_frame + m_clip.frame_count - 1;
            }
        }

        int col = id_frame % num_cols;
        int row = id_frame / num_cols;
        sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
    }
}