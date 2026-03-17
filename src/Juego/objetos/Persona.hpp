#pragma once
#include "../Figuras/Figuras.hpp"
#include "../Componentes/IJComponentes.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	/**
	 * @class Persona
	 * @brief Representa la persona, que tiene estados de salud y de color en funcion a su salud
	 */
	class Persona : public Rectangulo
	{
	public:
		/**
		 * @brief Constructor de la persona
		 * @param alto Alto del rectángulo que representa a la persona
		 * @param ancho Ancho del rectángulo que representa a la persona
		 * @param relleno Color de relleno
		 * @param contorno Color de contorno
		 */
		explicit Persona(float alto, float ancho,
						   const sf::Color& relleno,
						   const sf::Color& contorno);

		~Persona() override = default;

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
	};
}