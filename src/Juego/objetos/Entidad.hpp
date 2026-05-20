#pragma once
#include "../../Motor/Primitivos/Objetos.hpp" 
#include "../Maquinas/FSM.hpp"

#include <memory>

namespace IVJ
{
    class FSM; //referencia circular
    class FSMBoss;
	class Entidad: public CE::Objeto
	{
		public:
			Entidad() = default;
			Entidad(const Entidad &cpy);
			void onUpdate(float dt) override;
			void draw(sf::RenderTarget &target, sf::RenderStates state) const override;
			//fsm
			void setFSM(const std::shared_ptr<FSM>& mq);
			void inputFSM() override;   // patch

			// versiones const
			const std::shared_ptr<CE::INombre> getNombre() const
			{
				return nombre;
			}
			const std::shared_ptr<CE::IStats> getStats() const
			{
				return stats;
			}
			const std::vector<std::shared_ptr<CE::IComponentes>> &getListaComponentes() const
			{
				return componentes;
			}
			/**
			 * @brief Establece la dirección normalizada hacia el mouse.
			 * @param nx Componente X normalizada (-1 a 1)
			 * @param ny Componente Y normalizada (-1 a 1)
			 *
			 * Utilisado por sistemas que necesitan pasar información del mouse
			 * al FSM de la entidad (ej: sistema de animación de movimiento).
			 */
			void setMouseDirection(float nx, float ny)
			{
				mouse_dir_x = nx;
				mouse_dir_y = ny;
			}

	private:
		/** @brief Componente X normalizada de la dirección del mouse (almacenada) */
		float mouse_dir_x{0.f};
		/** @brief Componente Y normalizada de la dirección del mouse (almacenada) */
		float mouse_dir_y{0.f};
	};
}
