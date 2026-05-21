#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 *	@class DispararCuerpoPistola
	 *	@brief Estado para disparar la pistola del jugador
	 *
	 *	Transiciones
	 *	- animacion_terminada -> IdleCuerpoPistola
	 */
	class DispararCuerpoPistola : public FSM
	{
		public:
			explicit DispararCuerpoPistola(CE::AnimClip clip, float frame_rate = 0.1f);
			~DispararCuerpoPistola() override = default;
			FSM* onInputs(const CE::IControl &control) override;
			void onEntrar(const Entidad &obj) override;
			void onSalir(const Entidad &obj) override;
			void onUpdate(const Entidad &obj, float dt) override;
		private:
			sf::Sprite* sprite{nullptr};
			int s_w{0}, s_h{0}, num_cols{0};
			CE::AnimClip m_clip;
			int id_frame{0};
			float max_frame_time, cur_frame_time;
			bool disparo_completo{false};
			int local_frame{0};
	};
}