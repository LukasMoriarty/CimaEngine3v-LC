#include "Persona.hpp"
#include "../../Motor/Render/Render.hpp"
#include <cmath>

namespace IVJ
{
	// ── Constructor ──────────────────────────────────────────────────────────

	Persona::Persona(float alto, float ancho,
					 const sf::Color& relleno,
					 const sf::Color& contorno)
		: Rectangulo(alto, ancho, relleno, contorno)
	{}

	// ── onUpdate ─────────────────────────────────────────────────────────────

	void Persona::onUpdate(float dt)
	{
		// 1. Movimiento (si la persona tiene velocidad distinta de cero).
		//    Para uso estático en la cuadrícula, velocidad = {0,0} → sin efecto.
		transform->pos_prev = transform->posicion;
		transform->posicion.x += transform->velocidad.x * dt;
		transform->posicion.y += transform->velocidad.y * dt;

		// Rebote en los límites de la ventana (solo si se mueve)
		if (transform->velocidad.x != 0.f || transform->velocidad.y != 0.f)
		{
			auto windowSize = CE::Render::Get().GetVentana().getSize();
			float maxX = static_cast<float>(windowSize.x);
			float maxY = static_cast<float>(windowSize.y);

			if (transform->posicion.x < 0.f) {
				transform->posicion.x  = 0.f;
				transform->velocidad.x *= -1.f;
			} else if (transform->posicion.x > maxX) {
				transform->posicion.x  = maxX;
				transform->velocidad.x *= -1.f;
			}
			if (transform->posicion.y < 0.f) {
				transform->posicion.y  = 0.f;
				transform->velocidad.y *= -1.f;
			} else if (transform->posicion.y > maxY) {
				transform->posicion.y  = maxY;
				transform->velocidad.y *= -1.f;
			}
		}

		// 2. Sincronizar posición con rect_img (llamada al padre).
		Rectangulo::onUpdate(dt);

		// 3. Actualizar color visual según estado epidémico (si aplica).
		//    El SistemaEpidemia ya actualizó el estado ANTES de llamar a onUpdate,
		//    por lo que aquí reflejamos el estado más reciente.
		auto ep = getComponente<IVJ::IEstadoEpidemia>();
		if (ep)
		{
			actualizarColorEpidemia(*ep);
		}

		// 4. Actualizar partes del cuerpo (uso como víbora, opcional).
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (!cuerpo || cuerpo->partes.empty()) return;

		CE::Vector2D objetivo_anterior = transform->posicion;

		for (size_t i = 0; i < cuerpo->partes.size(); ++i)
		{
			auto& parte = cuerpo->partes[i];

			parte->cola_posiciones.push(objetivo_anterior);
			objetivo_anterior = parte->pos->posicion;

			if (!parte->hacerAccion)
			{
				parte->timer->curr_frame++;
				if (parte->timer->curr_frame >= parte->timer->max_frame)
					parte->hacerAccion = true;
			}

			if (parte->hacerAccion && !parte->cola_posiciones.vacia())
			{
				CE::Vector2D pos_destino = parte->cola_posiciones.front();
				parte->cola_posiciones.pop();
				parte->pos->posicion = pos_destino;
				parte->parte->setPosicion(pos_destino.x, pos_destino.y);
			}

			parte->parte->onUpdate(dt);
		}
	}

	// ── draw ─────────────────────────────────────────────────────────────────

	void Persona::draw(sf::RenderTarget& target, sf::RenderStates state) const
	{
		// Dibujar partes del cuerpo primero (si existen)
		auto cuerpo = getComponente<ICPartesCuerpo>();
		if (cuerpo)
		{
			for (auto& parte : cuerpo->partes)
			{
				if (parte && parte->parte)
					target.draw(*parte->parte, sf::RenderStates::Default);
			}
		}

		// Dibujar el rectángulo principal
		Rectangulo::draw(target, state);
	}

	// actualizarColorEpidemia

	void Persona::actualizarColorEpidemia(const IVJ::IEstadoEpidemia& ep)
	{
		using Estado = IVJ::IEstadoEpidemia::Estado;

		switch (ep.estado_actual)
		{
			case Estado::Normal:
				if (ep.tiene_inmunidad_parcial)
				{
					// Inmunidad parcial activa: cian claro (protegido temporalmente)
					setColores(sf::Color(160, 220, 220), sf::Color(0, 100, 100));
				}
				else
				{
					// Sano sin protección: gris muy claro, contorno oscuro
					setColores(sf::Color(235, 235, 235), sf::Color(40, 40, 40));
				}
				break;

			case Estado::Enfermo:
				// Gama roja-naranja: peligro activo
				setColores(sf::Color(220, 50, 10), sf::Color(140, 0, 0));
				break;

			case Estado::Recuperacion:
				// Gama azul-morado: convalecencia, no infeccioso
				setColores(sf::Color(80, 100, 210), sf::Color(30, 0, 140));
				break;

			case Estado::Inmune:
				// Gama verde: protección permanente
				setColores(sf::Color(50, 195, 80), sf::Color(0, 120, 0));
				break;

			case Estado::Muerto:
				// Gris oscuro / negro
				setColores(sf::Color(70, 70, 70), sf::Color(20, 20, 20));
				break;
		}
	}
} // namespace IVJ