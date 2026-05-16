#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	/**
	 * @class MoverPies
	 * @brief Estado de movimiento (walk) de los pies del jugador.
	 * Anima los frames del clip "walk" (frames 61–80 del atlas de pies).
	 * Transiciona a IdlePies cuando no hay input de movimiento.
	 */
	class MoverPies : public FSM
	{
	public:
		/**
		 * @param clip       AnimClip con {first_frame, frame_count} del JSON de pies.
		 *                   Típicamente {61, 20} para "walk".
		 * @param frame_rate Segundos por frame (por defecto 0.05).
		 */
		explicit MoverPies(CE::AnimClip clip, float frame_rate = 0.05f);
		~MoverPies() override = default;

		FSM* onInputs(const CE::IControl& control) override;
		/**
		 * @brief Procesa inputs considerando la dirección del mouse para elegir animación.
		 * @param control Estado de controles (arr, abj, izq, der, run)
		 * @param mouse_nx Componente X normalizada de la dirección al mouse (-1 a 1)
		 * @param mouse_ny Componente Y normalizada de la dirección al mouse (-1 a 1)
		 * @return Puntero a nuevo FSM si hay transición, nullptr si no
		 *
		 * Selecciona automáticamente entre:
		 * - Walk (frames 61-80) si se mueve hacia/contra la dirección del mouse
		 * - Strafe Left (frames 21-40) si se mueve perpendicular a izquierda
		 * - Strafe Right (frames 41-60) si se mueve perpendicular a derecha
		 * - Run (frames 1-20) si control.run está activado
		 */
		FSM* onInputs(const CE::IControl& control, float mouse_nx, float mouse_ny) override;
		void onEntrar(const Entidad& obj)          override;
		void onSalir(const Entidad& obj)           override;
		void onUpdate(const Entidad& obj, float dt) override;

	private:
		sf::Sprite* sprite{nullptr};
		int s_w{0};
		int s_h{0};
		int num_cols{9};

		CE::AnimClip m_clip;
		int id_frame{0};        ///< Frame absoluto actual

		float max_frame_time;
		float cur_frame_time;
	};
}