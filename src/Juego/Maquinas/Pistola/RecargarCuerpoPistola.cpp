#include "RecargarCuerpoPistola.hpp"
#include "IdleCuerpoPistola.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	RecargarCuerpoPistola::RecargarCuerpoPistola(CE::AnimClip clip, float frame_rate)
		: FSM{}, m_clip{clip},
		  max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
		nombre = "RecargarCuerpoPistola";
	}

	FSM *RecargarCuerpoPistola::onInputs(const CE::IControl& /*control*/) {
		if (recarga_completa)
			return new IdleCuerpoPistola(CE::AnimClip{0, 20}, 0.1f);
		return nullptr;
	}

	void RecargarCuerpoPistola::onEntrar(const Entidad &obj) {
		auto iSprite = obj.getComponente<CE::ISprite>();
		if (!iSprite) return;

		sprite = &iSprite->m_sprite;
		s_w = iSprite->width;
		s_h = iSprite->height;

		auto texSize = iSprite->m_sprite.getTexture().getSize();
		num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : num_cols;

		id_frame = m_clip.first_frame;
		cur_frame_time = max_frame_time;
		recarga_completa = false;
		local_frame = 0;

		if (obj.tieneComponente<IPistola>())
		{
			obj.getComponente<IPistola>()->recargando = true;
		}
	}

	void RecargarCuerpoPistola::onSalir(const Entidad &obj) {
		if (obj.tieneComponente<IPistola>())
		{
			obj.getComponente<IPistola>()->recargando = false;
		}
		recarga_completa = true;
	}

	void RecargarCuerpoPistola::onUpdate(const Entidad& /*obj*/, float dt) {
		if (!sprite || recarga_completa) return;

		cur_frame_time += dt;
		if (cur_frame_time > 0.f) return;
		cur_frame_time = max_frame_time;

		int col = id_frame % num_cols;
		int row = id_frame / num_cols;
		sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

		// avanzar el frame de animacion secuencialmente
		local_frame++;
		if (local_frame >= m_clip.frame_count)
		{
			recarga_completa = true;
			id_frame = m_clip.first_frame;
		}
		else
		{
			id_frame = m_clip.first_frame + local_frame;
		}
	}
} // namespace IVJ
