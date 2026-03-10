#include "EnteVibora.hpp"
#include "../../Motor/Render/Render.hpp"
#include <cmath>

namespace IVJ
{
	// 1. Actualizar el constructor
	EnteVibora::EnteVibora(float radio,
						   const sf::Color& relleno,
						   const sf::Color& contorno)
		: Circulo(radio, relleno, contorno)
	{}

	void EnteVibora::onUpdate(float dt)
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
		Circulo::onUpdate(dt);

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

	void EnteVibora::draw(sf::RenderTarget& target, sf::RenderStates state) const
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

		Circulo::draw(target, state);
	}

	void EnteVibora::agregarParteAlCuerpo(const sf::Color& color)
	{
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (!cuerpo) return;

		auto nueva_figura = std::make_shared<Rectangulo>(
			cuerpo->width,		// Ancho de la parte
			cuerpo->height,		// Alto de la parte
			color,				// Color de relleno
			sf::Color::Black	// Color de contorno
		);

		CE::Vector2D pos_inicial;
		if (cuerpo->partes.empty()) {
			pos_inicial = transform->posicion;
		} else {
			auto& ultima_parte = cuerpo->partes.back();
			pos_inicial = ultima_parte->pos->posicion;
		}

		auto nueva_parte = std::make_shared<ICParte>(nueva_figura, 35);
		nueva_parte->pos->posicion = pos_inicial;
		nueva_parte->parte->setPosicion(pos_inicial.x, pos_inicial.y);

		cuerpo->partes.push_back(nueva_parte);
	}
}