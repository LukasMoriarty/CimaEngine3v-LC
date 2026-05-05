#include "MoverZombie.hpp"
#include "IdleZombie.hpp"
#include "AtacarZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"
#include "../../../Motor/Primitivos/Objetos.hpp"

namespace IVJ {
    MoverZombie::MoverZombie(std::map<std::string, CE::AnimClip> clips, float frame_rate)
        : FSMZombie{},
          m_clips{std::move(clips)},
          max_frame_time{frame_rate},
          cur_frame_time{frame_rate}
    {
        nombre = "MoverZombie";
        if (m_clips.count("move")) {
            m_clip = m_clips["move"];
        }
    }

    FSM *MoverZombie::onInputs(Entidad &parent, CE::Vector2D &target) {
        float distancia = parent.getTransformada()->posicion.distancia(target);

        // 1. Si está muy cerca, atacar
        if (distancia < 50.f) {
            return new AtacarZombie(m_clips, 0.08f);
        }

        // 2. Si el jugador se alejó demasiado, volver a Idle
        // Usamos 250 (un poco más que los 200 de entrada) para evitar cambios bruscos
        if (distancia > 250.f) {
            return new IdleZombie(m_clips, 0.05f);
        }

        return nullptr;
    }

    void MoverZombie::onEntrar(const Entidad &obj) {
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

    void MoverZombie::onSalir(const Entidad &obj) {
        (void) obj;
    }

    void MoverZombie::onUpdate(const Entidad &obj, float dt) {
        (void) obj;
        if (!sprite) return;

        cur_frame_time -= dt;
        if (cur_frame_time <= 0.f) {
            cur_frame_time = max_frame_time;
            // Matemáticas corregidas para animaciones cíclicas (loops)
            int local = (id_frame - m_clip.first_frame + 1) % std::max(1, m_clip.frame_count);
            id_frame = m_clip.first_frame + local;
        }

        int col = id_frame % num_cols;
        int row = id_frame / num_cols;
        sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
    }
}