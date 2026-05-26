#include "AtacarZombie.hpp"
#include "IdleZombie.hpp"
#include "MoverZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"
#include "../../../Motor/Primitivos/GestorAssets.hpp"
#include <cstdlib>
#include <string>
#include <cmath>
#include "Motor/Render/Render.hpp"

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
            if (distancia > ZombieConst::RANGO_AGGRO) {
                return new IdleZombie(m_clips, ZombieConst::FRAME_RATE_MOVE);
            }
            if (distancia > ZombieConst::DISTANCIA_ATAQUE) {
                return new MoverZombie(m_clips, ZombieConst::FRAME_RATE_MOVE);
            }
            return new IdleZombie(m_clips, ZombieConst::FRAME_RATE_MOVE);
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

        // Reproducir sonido aleatorio de ataque
        int num = std::rand() % 7 + 1;
        std::string key_attack = "zombie_attack_0" + std::to_string(num);
        CE::GestorAssets::Get().getSonido(key_attack).setVolume(30.f);
        CE::GestorAssets::Get().getSonido(key_attack).play();

        // Estrictamente velocidad 0 al atacar
        if (obj.tieneComponente<CE::ITransform>()) {
            obj.getTransformada()->velocidad = {0.f, 0.f};
        }

        if (obj.getComponente<IArmaMelee>()) {
            obj.getComponente<IArmaMelee>()->atacando = true;
        }
    }

    void AtacarZombie::onSalir(const Entidad& obj) {
        if (obj.tieneComponente<IArmaMelee>()) {
            obj.getComponente<IArmaMelee>()->atacando = false;
        }
    }

    void AtacarZombie::onUpdate(const Entidad &obj, float dt) {
        (void) obj;
        if (!sprite) return;

        // Forzar velocidad 0 cada frame de ataque por si acaso
        if (obj.tieneComponente<CE::ITransform>()) {
            obj.getTransformada()->velocidad = {0.f, 0.f};
        }

        if (ataque_completo) return;

        cur_frame_time -= dt;
        if (cur_frame_time <= 0.f) {
            cur_frame_time = max_frame_time;
            id_frame++;

            // Animación lineal, no cíclica
            if (id_frame >= m_clip.first_frame + m_clip.frame_count) {
                ataque_completo = true;
                id_frame = m_clip.first_frame + m_clip.frame_count - 1;

                // Aplicar daño
                if (auto itarget = obj.getComponente<ITarget>()) {
                    if (itarget->target_obj) {
                        auto target_stats = itarget->target_obj->getStats();
                        if (target_stats->hp > 0) {
                            target_stats->hp = (target_stats->hp > 25) ? target_stats->hp - 25 : 0;
                            target_stats->timer_damage_shader = 1.0f; // 1 segundo de feedback visual

							// Update HP text if present
							if (itarget->target_obj->tieneComponente<CE::ITexto>()) {
								itarget->target_obj->getComponente<CE::ITexto>()->m_texto.setString(
									"HP: " + std::to_string((int)target_stats->hp));
							}
                        }
                    }
                }
            }
        }

        int col = id_frame % num_cols;
        int row = id_frame / num_cols;
        sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
    }
}