#pragma once
#include "../FSMZombie.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ {
	/**
	 * @class AtacarZombie
	 * @brief Estado de ataque del zombie.
	 * Anima los frames de "attack" cuando el jugador está muy cerca.
	 */
	class AtacarZombie : public FSMZombie {
	public:
		explicit AtacarZombie(CE::AnimClip clip, float frame_rate = 0.08f);
		~AtacarZombie() override = default;

		FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
		void onEntrar(const Entidad &obj) override;
		void onSalir(const Entidad &obj) override;
		void onUpdate(const Entidad &obj, float dt) override;

	private:
		sf::Sprite* sprite{nullptr};
		int s_w{0};
		int s_h{0};
		int num_cols{7};

		CE::AnimClip m_clip;
		int id_frame{0};

		float max_frame_time;
		float cur_frame_time;
	};
}