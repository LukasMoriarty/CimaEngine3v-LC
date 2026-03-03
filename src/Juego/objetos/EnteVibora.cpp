#include "EnteVibora.hpp"
#include "../../Motor/Render/Render.hpp"
#include <cmath>

namespace IVJ
{
	EnteVibora::EnteVibora(float ancho, float largo,
	                       const sf::Color& relleno,
	                       const sf::Color& contorno)
		: Rectangulo(ancho, largo, relleno, contorno)
	{
		// La cabeza se inicializa con su componente de partes del cuerpo
		// Se hará en la escena
	}

	void EnteVibora::onUpdate(float dt)
	{
		// Actualizar la posición de la cabeza (comportamiento base de Rectangulo)
		Rectangulo::onUpdate(dt);

		// Obtener el componente de partes del cuerpo si existe
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (!cuerpo) return;

		// Si no hay partes, no hacer nada
		if (cuerpo->partes.empty()) return;

		// La primera parte sigue a la cabeza
		CE::Vector2D objetivo_anterior = transform->posicion;

		// Actualizar cada parte del cuerpo
		for (size_t i = 0; i < cuerpo->partes.size(); ++i)
		{
			auto& parte = cuerpo->partes[i];

			// Incrementar el timer de la parte
			parte->timer->curr_frame++;

			// Si el timer alcanzó su máximo, activar la acción
			if (parte->timer->curr_frame >= parte->timer->max_frame)
			{
				parte->hacerAccion = true;
			}

			// Si la parte debe hacer acción (perseguir)
			if (parte->hacerAccion)
			{
				// Agregar la posición objetivo a la cola
				parte->cola_posiciones.push(objetivo_anterior);

				// Mantener un tamaño máximo de cola (para evitar memoria infinita)
				while (parte->cola_posiciones.size() > 50)
				{
					parte->cola_posiciones.pop();
				}

				// Actualizar la parte hacia el objetivo
				actualizarParte(parte, objetivo_anterior, dt);
			}

			// La siguiente parte seguirá a esta parte
			objetivo_anterior = parte->pos;
		}
	}

	void EnteVibora::actualizarParte(std::shared_ptr<ICParte> parte,
	                                 const CE::Vector2D& objetivo,
	                                 float dt)
	{
		if (!parte || !parte->parte) return;

		// Calcular dirección hacia el objetivo
		CE::Vector2D direccion = objetivo - parte->pos;
		float distancia = direccion.longitud();

		// Si está muy cerca, no moverse
		if (distancia < 1.0f) return;

		// Normalizar la dirección
		direccion.normalizacion();

		// Velocidad de seguimiento (ajustable)
		float velocidad = 150.0f;

		// Calcular nuevo movimiento
		CE::Vector2D movimiento = direccion;
		movimiento.escala(velocidad * dt);

		// Actualizar posición
		parte->pos = parte->pos + movimiento;

		// Actualizar la posición de la figura visual
		if (parte->parte)
		{
			parte->parte->setPosicion(parte->pos.x, parte->pos.y);
		}
	}

	void EnteVibora::draw(sf::RenderTarget& target, sf::RenderStates state) const
	{
		// Primero dibujar todas las partes del cuerpo (de atrás hacia adelante)
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (cuerpo)
		{
			for (auto& parte : cuerpo->partes)
			{
				if (parte && parte->parte)
				{
					target.draw(*parte->parte, state);
				}
			}
		}

		// Luego dibujar la cabeza encima
		Rectangulo::draw(target, state);
	}

	void EnteVibora::agregarParteAlCuerpo(const sf::Color& color)
	{
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (!cuerpo) return;

		// Crear nueva figura para la parte (círculo para diferenciarlo de la cabeza)
		auto nueva_figura = std::make_shared<Circulo>(
			cuerpo->width / 2.0f,  // radio basado en width
			color,
			sf::Color::Black
		);

		// Determinar posición inicial de la nueva parte
		CE::Vector2D pos_inicial;
		if (cuerpo->partes.empty())
		{
			// Si es la primera parte, colocarla detrás de la cabeza
			pos_inicial = transform->posicion;
			pos_inicial.x -= cuerpo->width;
		}
		else
		{
			// Colocarla detrás de la última parte
			auto& ultima_parte = cuerpo->partes.back();
			pos_inicial = ultima_parte->pos;
			pos_inicial.x -= cuerpo->width;
		}

		nueva_figura->setPosicion(pos_inicial.x, pos_inicial.y);

		// Crear el componente ICParte
		auto nueva_parte = std::make_shared<ICParte>(
			nueva_figura,
			10 * (cuerpo->partes.size() + 1)  // Timer más largo para partes posteriores
		);

		nueva_parte->pos = pos_inicial;

		// Agregar la parte al cuerpo
		cuerpo->agregarParte(nueva_parte);
	}
}