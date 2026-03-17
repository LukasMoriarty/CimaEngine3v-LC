#pragma once
#include "../Figuras/Figuras.hpp"
#include "../Componentes/IJComponentes.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
	/**
	 * @class Persona
	 * @brief Representa a una persona en la simulación.
	 *
	 * En la escena de epidemia, la Persona es un elemento estático de la
	 * cuadrícula. Su apariencia visual cambia automáticamente cada frame
	 * según el componente IEstadoEpidemia que tenga adjunto:
	 *
	 *  - Normal (sin inmunidad) → relleno gris claro, contorno oscuro
	 *  - Normal (inmunidad parcial) → relleno cian claro
	 *  - Enfermo                → relleno rojo-naranja
	 *  - Recuperación           → relleno azul-morado
	 *  - Inmune (permanente)    → relleno verde
	 *  - Muerto                 → relleno gris oscuro
	 */
	class Persona : public Rectangulo
	{
	public:
		/**
		 * @brief Constructor.
		 * @param alto     Alto del rectángulo
		 * @param ancho    Ancho del rectángulo
		 * @param relleno  Color inicial de relleno
		 * @param contorno Color inicial de contorno
		 */
		explicit Persona(float alto, float ancho,
						 const sf::Color& relleno,
						 const sf::Color& contorno);

		~Persona() override = default;

		/**
		 * @brief Actualiza la lógica de la persona cada frame.
		 *
		 * - Si tiene velocidad (uso como entidad móvil): aplica movimiento
		 *   con rebote en los bordes de ventana.
		 * - Si tiene IEstadoEpidemia: actualiza color según estado.
		 * - Si tiene ICPartesCuerpo (uso como víbora): actualiza las partes.
		 */
		void onUpdate(float dt) override;

		/** @brief Renderiza la persona (cuerpo + rectángulo base). */
		void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

	private:
		/**
		 * @brief Actualiza el color del rectángulo según el estado epidémico.
		 * @param ep Referencia constante al componente de estado.
		 */
		void actualizarColorEpidemia(const IVJ::IEstadoEpidemia& ep);
	};
}