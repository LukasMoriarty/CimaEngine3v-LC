#include "MoverCuerpoPistola.hpp"
#include "RecargarCuerpoPistola.hpp"
#include "DispararCuerpoPistola.hpp"
#include "IdleCuerpoPistola.hpp"

namespace IVJ
{
	MoverCuerpoPistola::MoverCuerpoPistola(CE::AnimClip clip, float frame_rate)
		: FSM{}, m_clip{clip},
		  max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
		nombre = "MoverCuerpoPistola";
	}

	FSM *MoverCuerpoPistola::onInputs(const CE::IControl &control)
	{
		if (control.acc)
			return new DispararCuerpoPistola(CE::AnimClip{70, 3}, 0.04f);
		if (!control.arr && !control.abj && !control.der && !control.izq)
			return new IdleCuerpoPistola(CE::AnimClip{0, 20}, 0.1f);
		if (control.sacc)	// recargar
			return new RecargarCuerpoPistola(CE::AnimClip{55, 15}, 0.1f);
		return nullptr;
	}

	void MoverCuerpoPistola::onEntrar(const Entidad &obj) {
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

	void MoverCuerpoPistola::onSalir(const Entidad &/*obj*/) {}

	void MoverCuerpoPistola::onUpdate(const Entidad& /*obj*/, float dt) {
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
} // namespace IVJ
