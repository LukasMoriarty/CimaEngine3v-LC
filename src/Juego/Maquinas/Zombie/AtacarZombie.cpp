#include "AtacarZombie.hpp"
#include "IdleZombie.hpp"
#include "MoverZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"
#include "../../../Motor/Primitivos/GestorAssets.hpp"
#include <string>

namespace IVJ
{
	AtacarZombie::AtacarZombie(std::map<std::string, CE::AnimClip> clips, float frame_rate)
		: FSMZombie{},
		  m_clips{std::move(clips)},
		  max_frame_time{frame_rate},
		  cur_frame_time{frame_rate}
	{
		nombre = "AtacarZombie";
		if (m_clips.count("attack"))
			m_clip = m_clips["attack"];
	}

	FSM* AtacarZombie::onInputs(Entidad& parent, CE::Vector2D& target)
	{
		auto pos = parent.getTransformada()->posicion;
		float distancia = pos.distancia(target);

		if (!ataque_completo) return nullptr;

		if (distancia > ZombieConst::RANGO_AGGRO)
			return new IdleZombie(m_clips, ZombieConst::FRAME_RATE_MOVE);
		if (distancia > ZombieConst::DISTANCIA_ATAQUE)
			return new MoverZombie(m_clips, ZombieConst::FRAME_RATE_MOVE);
		// Sigue cerca: volver a idle para que la FSM decida atacar de nuevo
		return new IdleZombie(m_clips, ZombieConst::FRAME_RATE_MOVE);
	}

	void AtacarZombie::onEntrar(const Entidad& obj)
	{
		auto iSprite = obj.getComponente<CE::ISprite>();
		if (!iSprite) return;

		sprite  = &iSprite->m_sprite;
		s_w     = iSprite->width;
		s_h     = iSprite->height;

		auto texSize = iSprite->m_sprite.getTexture().getSize();
		num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

		id_frame        = m_clip.first_frame;
		cur_frame_time  = max_frame_time;
		ataque_completo = false;

		// Reset damage-window state
		local_frame   = 0;
		past_midpoint = false;
		damage_dealt  = false;
		damage_frame  = std::max(1, m_clip.frame_count * 2 / 3);

		// Sonido aleatorio de ataque
		int num = std::rand() % 7 + 1;
		std::string key_attack = "zombie_attack_0" + std::to_string(num);
		CE::GestorAssets::Get().getSonido(key_attack).setVolume(30.f);
		CE::GestorAssets::Get().getSonido(key_attack).play();

		// Velocidad 0 al entrar en ataque
		obj.getTransformada()->velocidad = {0.f, 0.f};

		if (obj.tieneComponente<IArmaMelee>())
			obj.getComponente<IArmaMelee>()->atacando = true;
	}

	void AtacarZombie::onSalir(const Entidad& obj)
	{
		if (obj.tieneComponente<IArmaMelee>())
			obj.getComponente<IArmaMelee>()->atacando = false;
	}

	void AtacarZombie::onUpdate(const Entidad& obj, float dt)
	{
		if (!sprite) return;

		// Mantener velocidad 0 durante todo el ataque
		obj.getTransformada()->velocidad = {0.f, 0.f};

		if (ataque_completo) return;

		// Si el damage ya se aplicó, asegurarse de que la ventana esté cerrada
		if (damage_dealt)
			past_midpoint = false;

		cur_frame_time -= dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		// Renderizar frame actual
		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		// Abrir ventana de daño al pasar el midpoint
		int prospective = local_frame + 1;
		if (!damage_dealt && prospective >= damage_frame)
			past_midpoint = true;

		local_frame++;

		if (local_frame >= m_clip.frame_count)
		{
			ataque_completo = true;
			id_frame = m_clip.first_frame; // reset para la próxima vez
		}
		else
		{
			id_frame = m_clip.first_frame + local_frame;
		}
	}
} // namespace IVJ