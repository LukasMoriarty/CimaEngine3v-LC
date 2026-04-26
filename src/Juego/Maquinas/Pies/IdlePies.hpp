#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 * @class IdlePies
	 * @brief Estado idle de los pies del jugador.
	 * Solo muestra el frame 0 del atlas de pies (survivor-idle_00).
	 * Transiciona a MoverPies cuando hay input de movimiento.
	 */
	class IdlePies : public FSM
	{
	public:
		/**
		 * @param clip  AnimClip con {first_frame, frame_count} del JSON de pies.
		 *              Típicamente {0, 1} para "idle".
		 * @param frame_rate Segundos por frame (por defecto 0.1).
		 */
		explicit IdlePies(CE::AnimClip clip, float frame_rate = 0.1f);
		~IdlePies() override = default;

		FSM* onInputs(const CE::IControl& control) override;
		void onEntrar(const Entidad& obj)        override;
		void onSalir(const Entidad& obj)         override;
		void onUpdate(const Entidad& obj, float dt) override;

	private:
		sf::Sprite* sprite{nullptr};
		int s_w{0};
		int s_h{0};
		int num_cols{9};        ///< Columnas del atlas (3456/384 = 9)

		CE::AnimClip m_clip;    ///< Rango de frames de esta animación
		int id_frame{0};        ///< Frame absoluto actual (dentro del clip)

		float max_frame_time;
		float cur_frame_time;
	};
}