#include "AtacarCuerpoKnife.hpp"
#include "IdleCuerpoKnife.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	AtacarCuerpoKnife::AtacarCuerpoKnife(CE::AnimClip clip, float frame_rate)
		: FSM{}, m_clip{clip},
		  max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
		nombre = "AtacarCuerpoKnife";
	}

	FSM* AtacarCuerpoKnife::onInputs(const CE::IControl& /*control*/)
	{
		// Solo transiciona cuando el ciclo de ataque terminó
		if (ataque_completo)
			return new IdleCuerpoKnife(CE::AnimClip{0, 20}, 0.1f);
		return nullptr;
	}

	void AtacarCuerpoKnife::onEntrar(const Entidad& obj)
	{
		auto iSprite = obj.getComponente<CE::ISprite>();
		if (!iSprite) return;

		sprite = &iSprite->m_sprite;
		s_w    = iSprite->width;
		s_h    = iSprite->height;

		auto texSize = iSprite->m_sprite.getTexture().getSize();
		num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

		id_frame        = m_clip.first_frame;
		cur_frame_time  = max_frame_time;
		ataque_completo = false;
	}

	void AtacarCuerpoKnife::onSalir(const Entidad& /*obj*/) {}

	void AtacarCuerpoKnife::onUpdate(const Entidad& /*obj*/, float dt)
	{
		if (!sprite || ataque_completo) return;

		cur_frame_time -= dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		int local = id_frame - m_clip.first_frame + 1;
		if (local >= m_clip.frame_count)
		{
			ataque_completo = true;   // onInputs detectará esto y transicionará
			id_frame = m_clip.first_frame; // reset para próxima vez
		}
		else
		{
			id_frame = m_clip.first_frame + local;
		}
	}
}