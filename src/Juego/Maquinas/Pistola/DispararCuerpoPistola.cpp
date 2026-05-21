#include "DispararCuerpoPistola.hpp"
#include "IdleCuerpoPistola.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	DispararCuerpoPistola::DispararCuerpoPistola(CE::AnimClip clip, float frame_rate)
		: FSM{}, m_clip{clip},
		  max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
		nombre = "DispararCuerpoPistola";
	}

	FSM* DispararCuerpoPistola::onInputs(const CE::IControl& /*control*/)
	{
		if (disparo_completo)
			return new IdleCuerpoPistola(CE::AnimClip{-1, -1}, 0.1f);
		return nullptr;
	}

	void DispararCuerpoPistola::onEntrar(const Entidad &obj) {
		auto iSprite = obj.getComponente<CE::ISprite>();
		if (!iSprite) return;

		sprite = &iSprite->m_sprite;
		s_w = iSprite->width;
		s_h = iSprite->height;

		auto texSize = iSprite->m_sprite.getTexture().getSize();
		num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

		id_frame = m_clip.first_frame;
		cur_frame_time = max_frame_time;
		disparo_completo = false;

		if (obj.tieneComponente<IPistola>())
		{
			obj.getComponente<IPistola>()->disparando = true;
		}
	}

	void DispararCuerpoPistola::onSalir(const Entidad &obj) {
		if (obj.tieneComponente<IPistola>())
		{
			obj.getComponente<IPistola>()->disparando = false;
		}
		disparo_completo = true;
	}

	void DispararCuerpoPistola::onUpdate(const Entidad& /*obj*/, float dt) {
		if (!sprite || disparo_completo) return;

		cur_frame_time += dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		id_frame = m_clip.first_frame;
	}
} // namespace IVJ
