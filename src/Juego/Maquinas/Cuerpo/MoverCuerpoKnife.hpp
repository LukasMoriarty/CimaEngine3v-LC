#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 * @class MoverCuerpoKnife
	 * @brief Estado de movimiento del cuerpo con cuchillo (frames 35-54).
	 *
	 * Transiciones:
	 *  - sin movimiento → IdleCuerpoKnife
	 *  - control.acc    → AtacarCuerpoKnife
	 */
	class MoverCuerpoKnife : public FSM
	{
	public:
		explicit MoverCuerpoKnife(CE::AnimClip clip, float frame_rate = 0.05f);
		~MoverCuerpoKnife() override = default;

		FSM* onInputs(const CE::IControl& control) override;
		void onEntrar(const Entidad& obj)          override;
		void onSalir(const Entidad& obj)           override;
		void onUpdate(const Entidad& obj, float dt) override;

	private:
		sf::Sprite* sprite{nullptr};
		int s_w{0};
		int s_h{0};
		int num_cols{8};

		CE::AnimClip m_clip;
		int id_frame{0};

		float max_frame_time;
		float cur_frame_time;
	};
}