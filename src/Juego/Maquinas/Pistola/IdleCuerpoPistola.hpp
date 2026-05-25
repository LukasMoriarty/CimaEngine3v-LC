#pragma once
#include "Motor/Componentes/IComponentes.hpp"
#include "../../Componentes/IJComponentes.hpp"
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ {
	/**
	 *	@class IdleCuerpoPistola
	 *	@brief Estado idle del cuerpo con pistola
	 *
	 *	Transiciones:
	 *	- control.acc	->	DispararCuerpoPistola
	 *	- control.sacc	->	RecargarCuerpoPistola
	 *	- movimiento (arr/abj/...)	->	MoverCuerpoPistola
	 */
	class IdleCuerpoPistola : public FSM
	{
		public:
			explicit IdleCuerpoPistola(CE::AnimClip clip, float frame_rate = 0.1f);
			~IdleCuerpoPistola() override = default;

			FSM* onInputs(const CE::IControl& control) override;
			void onEntrar(const Entidad& obj) override;
			void onSalir(const Entidad& obj) override;
			void onUpdate(const Entidad& obj, float dt) override;
		private:
			sf::Sprite* sprite{nullptr};
			int s_w{0}, s_h{0}, num_cols{8};
			IVJ::IPistola* mi_arma{nullptr};
			CE::AnimClip m_clip;
			int id_frame{0};
			float max_frame_time, cur_frame_time;
	};
}