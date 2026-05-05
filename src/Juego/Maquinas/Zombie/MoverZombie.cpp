#include "MoverZombie.hpp"
#include "IdleZombie.hpp"
#include "AtacarZombie.hpp"
#include "../../../Motor/Componentes/IComponentes.hpp"
#include "../../../Motor/Primitivos/Objetos.hpp"

namespace IVJ {
	MoverZombie::MoverZombie(CE::AnimClip clip, float frame_rate)
		: FSMZombie{},
		  m_clip{clip},
		  max_frame_time{frame_rate},
		  cur_frame_time{frame_rate}
	{
		nombre = "MoverZombie";
	}

	FSM *MoverZombie::onInputs(Entidad &parent, CE::Vector2D &target) {
		auto pos = parent.getTransformada()->posicion;
		float distancia = pos.distancia(target);

		// Si está cerca, cambia a ataque
		if (distancia < 50.f) {
			return new AtacarZombie(CE::AnimClip{0, 9}, 0.08f);
		}

		// Si se aleja demasiado, regresa a idle
		if (distancia > 200.f) {
			return new IdleZombie(CE::AnimClip{9, 17}, 0.05f);
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

	void MoverZombie::onSalir(const Entidad& obj) {
		(void) obj;
	}

	void MoverZombie::onUpdate(const Entidad &obj, float dt) {
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