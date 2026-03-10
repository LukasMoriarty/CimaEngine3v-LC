#include "EnteVibora.hpp"
#include "../../Motor/Render/Render.hpp"
#include <cmath>

namespace IVJ
{
	EnteVibora::EnteVibora(float ancho, float largo,
						   const sf::Color& relleno,
						   const sf::Color& contorno)
		: Rectangulo(ancho, largo, relleno, contorno)
	{}

	void EnteVibora::onUpdate(float dt)
	{
		// 1. Movimiento base de la cabeza
		transform->pos_prev = transform->posicion;
		transform->posicion.x += transform->velocidad.x * dt;
		transform->posicion.y += transform->velocidad.y * dt;
		Rectangulo::onUpdate(dt);

		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (!cuerpo || cuerpo->partes.empty()) return;

		// El primer objetivo a perseguir es la posición actual de la cabeza
		CE::Vector2D objetivo_anterior = transform->posicion;

		for (size_t i = 0; i < cuerpo->partes.size(); ++i)
		{
			auto& parte = cuerpo->partes[i];

			// 2. Insertamos la posición a perseguir
			parte->cola_posiciones.push(objetivo_anterior);

			// 3. IMPORTANTE: Guardamos dónde está ESTA parte ANTES de moverla,
			// para que la SIGUIENTE parte la persiga a ella.
			objetivo_anterior = parte->pos->posicion;

			// 4. Lógica del timer para iniciar el movimiento
			if (!parte->hacerAccion) {
				parte->timer->curr_frame++;
				if (parte->timer->curr_frame >= parte->timer->max_frame) {
					parte->hacerAccion = true;
				}
			}
	
			// 5. Aplicar el movimiento si el timer ya terminó
			if (parte->hacerAccion && !parte->cola_posiciones.vacia()) {
				CE::Vector2D pos_destino = parte->cola_posiciones.front();
				parte->cola_posiciones.pop();

				parte->pos->posicion = pos_destino;
				parte->parte->setPosicion(pos_destino.x, pos_destino.y);
			}

			// Actualizar la figura por si tiene lógica interna
			parte->parte->onUpdate(dt);
		}
	}

	void EnteVibora::actualizarParte(std::shared_ptr<ICParte> parte,
						  const CE::Vector2D& objetivo,
						  float dt)
	{
		if (!parte || !parte->parte) return;

		// Calcular dirección hacia el objetivo (Usamos ->posicion)
		CE::Vector2D direccion = objetivo - parte->pos->posicion;
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

		// Actualizar posición (Usamos ->posicion)
		parte->pos->posicion = parte->pos->posicion + movimiento;

		// Actualizar la posición de la figura visual (Usamos ->posicion.x y ->posicion.y)
		if (parte->parte)
		{
		   parte->parte->setPosicion(parte->pos->posicion.x, parte->pos->posicion.y);
		}
	}

	void EnteVibora::draw(sf::RenderTarget& target, sf::RenderStates state) const
	{
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (cuerpo)
		{
			for (auto& parte : cuerpo->partes)
			{
				if (parte && parte->parte)
				{
					// IMPORTANTE: Usamos Default para dibujar en coordenadas globales reales
					target.draw(*parte->parte, sf::RenderStates::Default);
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

		auto nueva_figura = std::make_shared<Circulo>(cuerpo->width / 2.0f, color, sf::Color::Black);

		CE::Vector2D pos_inicial;
		if (cuerpo->partes.empty()) {
			pos_inicial = transform->posicion;
		} else {
			auto& ultima_parte = cuerpo->partes.back();
			pos_inicial = ultima_parte->pos->posicion; // Extraído correctamente
		}

		// Le damos 15 frames de retraso inicial para separarla de la parte anterior
		auto nueva_parte = std::make_shared<ICParte>(nueva_figura, 15);
		nueva_parte->pos->posicion = pos_inicial;
		nueva_parte->parte->setPosicion(pos_inicial.x, pos_inicial.y);

		cuerpo->partes.push_back(nueva_parte);
	}
}