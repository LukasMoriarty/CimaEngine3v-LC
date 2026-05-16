#include "MoverPies.hpp"
#include "IdlePies.hpp"
#include "Motor/Componentes/IComponentes.hpp"
#include <cmath>

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
		// Versión sin información del mouse: elegir "walk" por defecto.
		if (!control.arr && !control.abj && !control.der && !control.izq)
			return new IdlePies(CE::AnimClip{0, 1}); // idle: frame 0

		CE::AnimClip desired{61, 20};
		float desired_rate = 0.05f;

		// Si el jugador está corriendo, mantener la misma animación pero más rápida.
		if (control.run) {
			desired_rate = 0.02f;
		}

		// Si la animación actual no coincide con la deseada (por ejemplo, estamos en run frames), cambiar.
		if (m_clip.first_frame != desired.first_frame || std::abs(max_frame_time - desired_rate) > 1e-6f)
			return new MoverPies(desired, desired_rate);

		return nullptr;
	}

	FSM* MoverPies::onInputs(const CE::IControl& control, float mouse_nx, float mouse_ny)
	{
		// Si no hay movimiento, ir a idle
		if (!control.arr && !control.abj && !control.der && !control.izq)
			return new IdlePies(CE::AnimClip{0, 1}); // idle: frame 0

		// Calcular la dirección de movimiento del control
		float move_x = 0.f, move_y = 0.f;
		if (control.der) move_x += 1.f;
		if (control.izq) move_x -= 1.f;
		if (control.arr) move_y -= 1.f;
		if (control.abj) move_y += 1.f;

		// Normalizar dirección de movimiento
		if (float move_len = std::sqrt(move_x * move_x + move_y * move_y); move_len > 0.f) {
			move_x /= move_len;
			move_y /= move_len;
		}

		// Calcular producto punto entre dirección del mouse y dirección de movimiento
		float dot = mouse_nx * move_x + mouse_ny * move_y;

		// Calcular componente perpendicular (para detectar strafe)
		float perp = mouse_nx * move_y - mouse_ny * move_x;

		// Decidir animación basándose en el ángulo relativo
		// Si se mueve principalmente hacia una dirección perpendicular al mouse, es strafe
		// Si se mueve principalmente hacia el mouse o opuesto, es walk

		CE::AnimClip next_clip{61, 20}; // walk por defecto
		float frame_rate = 0.05f; // framerate normal para walk

		// Aplicar corrección si se está moviendo lateralmente respecto al mouse
		if (std::abs(perp) > std::abs(dot)) {
			// Movimiento lateral: strafe
			if (perp > 0.f) {
				// Strafe a la derecha
				next_clip = CE::AnimClip{41, 20};
			} else {
				// Strafe a la izquierda
				next_clip = CE::AnimClip{21, 20};
			}
		} else {
			// Movimiento frontal o trasero: walk
			next_clip = CE::AnimClip{61, 20};
		}

		// Si control.run está activo, no hacer fallback a la animación "run".
		// En su lugar, usar la animación calculada (walk/strafe) pero con framerate más rapido.
		if (control.run) {
			frame_rate = 0.02f; // framerate más rápido
		}

		// Transicionar si el clip o el framerate deseado difieren del actual
		if (m_clip.first_frame != next_clip.first_frame || std::abs(max_frame_time - frame_rate) > 1e-6f)
			return new MoverPies(next_clip, frame_rate);

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