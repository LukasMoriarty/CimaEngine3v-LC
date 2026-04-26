#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 * @class AtacarCuerpoKnife
	 * @brief Ataque cuerpo a cuerpo con cuchillo (frames 20-34, 15 frames).
	 *
	 * El ataque se reproduce una vez completo y luego transiciona automáticamente
	 * a IdleCuerpoKnife (en onInputs cuando ataque_completo == true).
	 */
	class AtacarCuerpoKnife : public FSM
	{
	public:
		explicit AtacarCuerpoKnife(CE::AnimClip clip, float frame_rate = 0.025f);
		~AtacarCuerpoKnife() override = default;

		FSM* onInputs(const CE::IControl& control) override;
		void onEntrar(const Entidad& obj)           override;
		void onSalir(const Entidad& obj)            override;
		void onUpdate(const Entidad& obj, float dt)  override;

	private:
		sf::Sprite* sprite{nullptr};
		int s_w{0};
		int s_h{0};
		int num_cols{8};

		CE::AnimClip m_clip;
		int id_frame{0};

		float max_frame_time;
		float cur_frame_time;
		bool ataque_completo{false};
	};
}