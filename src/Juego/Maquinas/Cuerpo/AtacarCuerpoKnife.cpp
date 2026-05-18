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
		damage_frame = std::max(1, m_clip.frame_count / 2);
	}

	FSM* AtacarCuerpoKnife::onInputs(const CE::IControl& /*control*/)
	{
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

 		// Reset per-swing state.
 		local_frame    = 0;
 		past_midpoint = false;
 		damage_dealt   = false;

		// Recompute in case the clip was changed before entering.
		damage_frame = std::max(1, m_clip.frame_count / 2);
	}

	void AtacarCuerpoKnife::onSalir(const Entidad& /*obj*/) {}

	void AtacarCuerpoKnife::onUpdate(const Entidad& /*obj*/, float dt)
	{
		if (!sprite || ataque_completo) return;

		// If damage was already dealt for this swing, ensure the window is closed.
		// The flag "past_midpoint" will be set once the animation crosses the
		// midpoint and will remain true until markDamageDealt() sets damage_dealt.
		if (damage_dealt)
			past_midpoint = false;

		cur_frame_time -= dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		// Render the current frame.
		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		// If the animation crosses (or lands on) the midpoint this update,
		// set the damage pulse and keep it active until markDamageDealt() is
		// called. Use the prospective frame (local_frame + 1) to handle cases
		// where the animation may jump past the midpoint in a single tick.
		int prospective_frame = local_frame + 1;
		if (!damage_dealt && prospective_frame >= damage_frame)
			past_midpoint = true;

		local_frame++;

		if (local_frame >= m_clip.frame_count)
		{
			ataque_completo = true;
			id_frame        = m_clip.first_frame; // reset for next use
		}
		else
		{
			id_frame = m_clip.first_frame + local_frame;
		}
	}
}