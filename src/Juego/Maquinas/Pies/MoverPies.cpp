#include "MoverPies.hpp"
#include "IdlePies.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	MoverPies::MoverPies(CE::AnimClip clip, float frame_rate)
		: FSM{}, m_clip{clip},
		  max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
		nombre = "MoverPies";
	}

	FSM* MoverPies::onInputs(const CE::IControl& control)
	{
		if (!control.arr && !control.abj && !control.der && !control.izq)
			return new IdlePies(CE::AnimClip{0, 1}); // idle: frame 0

		// Cambio dinámico entre correr y caminar
		if (control.run && m_clip.first_frame != 1)
			return new MoverPies(CE::AnimClip{1, 20}, 0.02f); // run (más rápido)

		if (!control.run && m_clip.first_frame != 61)
			return new MoverPies(CE::AnimClip{61, 20}); // walk (normal)

		return nullptr;
	}

	void MoverPies::onEntrar(const Entidad& obj)
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

	void MoverPies::onSalir(const Entidad& /*obj*/) {}

	void MoverPies::onUpdate(const Entidad& /*obj*/, float dt)
	{
		if (!sprite) return;
		cur_frame_time -= dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		int local = (id_frame - m_clip.first_frame + 1) % m_clip.frame_count;
		id_frame  = m_clip.first_frame + local;
	}
}