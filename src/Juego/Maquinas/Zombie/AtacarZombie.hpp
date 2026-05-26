#pragma once
#include "../FSMZombie.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"
#include <map>
#include <string>

namespace IVJ {
	/**
	 * @class AtacarZombie
	 * @brief Estado de ataque del zombie.
	 *
	 * El daño se aplica a través de un damage-window que se abre al pasar
	 * el midpoint de la animación, igual que AtacarCuerpoKnife.
	 * La escena llama shouldDealDamage() cada frame y, si es true,
	 * aplica el daño y llama markDamageDealt() para cerrar la ventana.
	 */
	class AtacarZombie : public FSMZombie {
	public:
		explicit AtacarZombie(std::map<std::string, CE::AnimClip> clips,
							  float frame_rate = ZombieConst::FRAME_RATE_ATTACK);
		~AtacarZombie() override = default;

		FSM* onInputs(Entidad& parent, CE::Vector2D& target) override;
		void onEntrar(const Entidad& obj) override;
		void onSalir(const Entidad& obj) override;
		void onUpdate(const Entidad& obj, float dt) override;

		// ── Damage-window API (mirrors AtacarCuerpoKnife) ────────────────────
		/** Returns true once the animation passes its midpoint AND
		 *  damage has not yet been dealt this swing. */
		[[nodiscard]] bool shouldDealDamage() const { return past_midpoint && !damage_dealt; }

		/** Call once after damage is dealt to close the window for this swing. */
		void markDamageDealt() { damage_dealt = true; }

	private:
		sf::Sprite* sprite{nullptr};
		int s_w{0};
		int s_h{0};
		int num_cols{7};

		std::map<std::string, CE::AnimClip> m_clips;
		CE::AnimClip m_clip;
		int id_frame{0};

		float max_frame_time;
		float cur_frame_time;
		bool ataque_completo{false};

		// ── Damage-window state ───────────────────────────────────────────────
		int  local_frame{0};       ///< Frames elapsed since attack started (0 = first)
		int  damage_frame{0};      ///< Local frame at which the window opens (clip midpoint)
		bool past_midpoint{false}; ///< True once local_frame has reached damage_frame
		bool damage_dealt{false};  ///< True after the scene has applied damage once
	};
}