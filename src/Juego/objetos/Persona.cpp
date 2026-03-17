#include "Persona.hpp"
#include "../../Motor/Render/Render.hpp"
#include <cmath>

namespace IVJ
{
	// 1. Actualizar el constructor
	Persona::Persona(float alto, float ancho,
						   const sf::Color& relleno,
						   const sf::Color& contorno)
		: Rectangulo(alto, ancho, relleno, contorno)
	{}

	void Persona::onUpdate(float dt)
	{
		// Movimiento base de la cabeza
		transform->pos_prev = transform->posicion;
		transform->posicion.x += transform->velocidad.x * dt;
		transform->posicion.y += transform->velocidad.y * dt;

		auto windowSize = CE::Render::Get().GetVentana().getSize();
		float maxX = static_cast<float>(windowSize.x);
		float maxY = static_cast<float>(windowSize.y);

		if (transform->posicion.x < 0.f)
		{
			transform->posicion.x = 0.f;
            transform->velocidad.x *= -1.f;
        } else if (transform->posicion.x > maxX) {
            transform->posicion.x = maxX;
            transform->velocidad.x *= -1.f;
		}

		if (transform->posicion.y < 0.f)
        {
            transform->posicion.y = 0.f;
            transform->velocidad.y *= -1.f;
        } else if (transform->posicion.y > maxY) {
            transform->posicion.y = maxY;
            transform->velocidad.y *= -1.f;
        }

		// 3. Actualizar la cabeza como Triangulo
		Rectangulo::onUpdate(dt);

		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (!cuerpo || cuerpo->partes.empty()) return;

		CE::Vector2D objetivo_anterior = transform->posicion;

		for (size_t i = 0; i < cuerpo->partes.size(); ++i)
		{
			auto& parte = cuerpo->partes[i];

			parte->cola_posiciones.push(objetivo_anterior);
			objetivo_anterior = parte->pos->posicion;

			if (!parte->hacerAccion) {
				parte->timer->curr_frame++;
				if (parte->timer->curr_frame >= parte->timer->max_frame) {
					parte->hacerAccion = true;
				}
			}

			if (parte->hacerAccion && !parte->cola_posiciones.vacia()) {
				CE::Vector2D pos_destino = parte->cola_posiciones.front();
				parte->cola_posiciones.pop();
				
				parte->pos->posicion = pos_destino;
				parte->parte->setPosicion(pos_destino.x, pos_destino.y);
			}
			
			parte->parte->onUpdate(dt);
		}
	}

	void Persona::draw(sf::RenderTarget& target, sf::RenderStates state) const
	{
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (cuerpo)
		{
			for (auto& parte : cuerpo->partes)
			{
				if (parte && parte->parte)
				{
					target.draw(*parte->parte, sf::RenderStates::Default);
				}
			}
		}

		Rectangulo::draw(target, state);
	}
}