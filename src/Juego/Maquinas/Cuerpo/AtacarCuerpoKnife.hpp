#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 * @class AtacarCuerpoKnife
	 * @brief Ataque cuerpo a cuerpo con cuchillo (frames 20-34, 15 frames).
	 *
	 * The attack plays once then transitions back to IdleCuerpoKnife.
	 *
	 * Damage window: the scene may deal damage only AFTER the animation passes
	 * its midpoint AND before markDamageDealt() is called.  This prevents hits
	 * at the very start or very end of the swing, and the window is large enough
	 * (roughly half the animation) that framerate jitter cannot miss it.
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

		// ── Damage-window API ─────────────────────────────────────────────────

		/**
		 * @brief Returns true once the animation has reached its midpoint AND
		 *        damage has not yet been dealt this swing.
		 *
		 * The window stays open until markDamageDealt() is called, so the scene
		 * can check this every frame without worrying about missing a single tick.
		 */
		bool shouldDealDamage() const { return past_midpoint && !damage_dealt; }

		/**
		 * @brief Call once after damage is dealt to close the window for this swing.
		 */
		void markDamageDealt() { damage_dealt = true; }

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

		// ── Damage-window state ───────────────────────────────────────────────
		/// Local animation frame counter (0 = first frame of clip).
		int local_frame{0};
		/// The local frame index at which the damage window opens (clip midpoint).
		int damage_frame{0};
		/// True once local_frame has passed damage_frame; stays true until reset.
		bool past_midpoint{false};
		/// True after the scene has dealt damage; prevents double-hitting.
		bool damage_dealt{false};
	};
}