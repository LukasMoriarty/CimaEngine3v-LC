#include "IdleCuerpoKnife.hpp"
#include "MoverCuerpoKnife.hpp"
#include "AtacarCuerpoKnife.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	IdleCuerpoKnife::IdleCuerpoKnife(CE::AnimClip clip, float frame_rate)
		: FSM{}, m_clip{clip},
		  max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
		nombre = "IdleCuerpoKnife";
	}

	FSM* IdleCuerpoKnife::onInputs(const CE::IControl& control)
	{
		if (control.acc)
			return new AtacarCuerpoKnife(CE::AnimClip{20, 15}, 0.04f);
		if (control.arr || control.abj || control.der || control.izq)
			return new MoverCuerpoKnife(CE::AnimClip{35, 20}, 0.1f);

		return nullptr;
	}

	void IdleCuerpoKnife::onEntrar(const Entidad& obj)
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
	}

	void IdleCuerpoKnife::onSalir(const Entidad& /*obj*/) {}

	void IdleCuerpoKnife::onUpdate(const Entidad& /*obj*/, float dt)
	{
		if (!sprite) return;
		cur_frame_time -= dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		int local = (id_frame - m_clip.first_frame + 1) % std::max(1, m_clip.frame_count);
		id_frame  = m_clip.first_frame + local;
	}
}