#pragma once
#include "../Figuras/Figuras.hpp"
#include "../Componentes/IJComponentes.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	/**
	 * @class EnteVibora
	 * @brief Representa la cabeza de la víbora con su cuerpo
	 *
	 * Esta clase gestiona la cabeza de la víbora y coordina el movimiento
	 * de todas las partes del cuerpo que la siguen.
	 */
	class EnteVibora : public Circulo
	{
	public:
		/**
		 * @brief Constructor de la víbora
		 * @param radio Radio de la cabeza
		 * @param relleno Color de relleno
		 * @param contorno Color de contorno
		 */
		explicit EnteVibora(float radio,
						   const sf::Color& relleno,
						   const sf::Color& contorno);

		~EnteVibora() override = default;

		/**
		 * @brief Actualiza la lógica de la víbora cada frame
		 * @param dt Delta time en segundos
		 *
		 * Esta función coordina el movimiento de todas las partes
		 * del cuerpo para que sigan a la cabeza.
		 */
		void onUpdate(float dt) override;

		/**
		 * @brief Renderiza la víbora completa (cabeza y cuerpo)
		 */
		void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

		/**
		 * @brief Agrega una nueva parte al cuerpo de la víbora
		 * @param color Color de la nueva parte
		 */
		void agregarParteAlCuerpo(const sf::Color& color);

	private:
		/**
		 * @brief Actualiza el movimiento de una parte específica
		 * @param parte Parte a actualizar
		 * @param objetivo Posición objetivo a seguir
		 * @param dt Delta time
		 */
		void actualizarParte(std::shared_ptr<ICParte> parte,
							const CE::Vector2D& objetivo,
							float dt);
	};
}