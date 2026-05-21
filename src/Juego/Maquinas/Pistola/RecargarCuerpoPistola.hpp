#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 *	@class RecargarCuerpoPistola
	 *	@brief Estado de recarga del cuerpo con pistola
	 *
	 *	Transiciones:
	 *	- recarga_completa -> IdleCuerpoPistola
	 */
	class RecargarCuerpoPistola : public FSM
	{
		public:
			explicit RecargarCuerpoPistola(CE::AnimClip clip, float frame_rate = 0.05f);
			~RecargarCuerpoPistola() override = default;

			FSM* onInputs(const CE::IControl& control) override;
			void onEntrar(const Entidad& obj) override;
			void onSalir(const Entidad& obj) override;
			void onUpdate(const Entidad& obj, float dt) override;
		private:
			sf::Sprite* sprite{nullptr};
			int s_w{0}, s_h{0}, num_cols{8};
			CE::AnimClip m_clip;
			int id_frame{0};
			float max_frame_time, cur_frame_time;
			bool recarga_completa{false};
	};
}