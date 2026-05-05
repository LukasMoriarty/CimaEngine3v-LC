#include "AtacarZombie.hpp"
#include "MoverZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"

namespace IVJ {
	AtacarZombie::AtacarZombie(CE::AnimClip clip, float frame_rate)
		: FSMZombie{},
		  m_clip{clip},
		  max_frame_time{frame_rate},
		  cur_frame_time{frame_rate}
	{
		nombre = "AtacarZombie";
	}

	FSM *AtacarZombie::onInputs(Entidad &parent, CE::Vector2D &target) {
		auto pos = parent.getTransformada()->posicion;
		float distancia = pos.distancia(target);

		// Si el target se aleja, vuelve a moverse
		if (distancia > 80.f) {
			return new MoverZombie(CE::AnimClip{26, 17}, 0.05f);
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
	}

	void AtacarZombie::onSalir(const Entidad& obj) {
		(void) obj;
	}

	void AtacarZombie::onUpdate(const Entidad &obj, float dt) {
		if (!sprite) return;

		// Animar
		cur_frame_time -= dt;
		if (cur_frame_time <= 0.f) {
			cur_frame_time = max_frame_time;

			int local = (id_frame - m_clip.first_frame + 1) % std::max(1, m_clip.frame_count);
			id_frame = m_clip.first_frame + local;
		}

		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});
	}
}