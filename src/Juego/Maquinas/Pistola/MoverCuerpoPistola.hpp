#pragma once
#include "Motor/Componentes/IComponentes.hpp"
#include "../../Componentes/IJComponentes.hpp"
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 *	@class MoverCuerpoPistola
	 *	@brief Estado de movimiento del cuerpo con pistola
	 *
	 *	Transiciones:
	 *	- sin movimiento -> IdleCuerpoPistola
	 *	- control.acc -> DispararCuerpoPistola
	 *	- control.sacc -> RecargarCuerpoPistola
	 */
	class MoverCuerpoPistola : public FSM
	{
		public:
			explicit MoverCuerpoPistola(CE::AnimClip clip, float frame_rate = 0.05f);
			~MoverCuerpoPistola() override = default;

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