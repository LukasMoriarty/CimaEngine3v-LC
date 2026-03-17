#include "Escena_Epidemia.hpp"

#include <Juego/objetos/Persona.hpp>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/GUI/GLogger.hpp>

#include <cstdlib>   // rand, RAND_MAX
#include <algorithm> // std::swap
#include <string>
#include <vector>

namespace IVJ
{
	// ── Constructor ──────────────────────────────────────────────────────────

	Escena_Epidemia::Escena_Epidemia(std::shared_ptr<Entidad>& pref)
		: CE::Escena{}, jugador_ref{pref}
	{}

	// ── onInit ───────────────────────────────────────────────────────────────

	void Escena_Epidemia::onInit()
	{
		if (!inicializar) return;

		// ── Configuración de la cuadrícula ────────────────────────────────
		const int   columnas   = 15;
		const int   filas	  = 15;
		const float tam_celda  = 32.f;   // tamaño de cada celda
		const float gap		= 2.f;	// margen visible entre celdas
		const float celda_vis  = tam_celda - gap;

		columnasGrid = columnas;

		auto windowSize = CE::Render::Get().GetVentana().getSize();
		const float ancho_total = columnas * tam_celda;
		const float alto_total  = filas	* tam_celda;

		// Centrar cuadrícula dejando espacio para el panel de stats (izquierda)
		const float offset_x = (windowSize.x - ancho_total) / 2.f;
		const float offset_y = (windowSize.y - alto_total)  / 2.f;

		// ── Crear personas en orden row-major ─────────────────────────────
		// Índice en el pool: fila * columnas + columna
		for (int fila = 0; fila < filas; ++fila)
		{
			for (int columna = 0; columna < columnas; ++columna)
			{
				auto persona = std::make_shared<Persona>(
					celda_vis, celda_vis,
					sf::Color(235, 235, 235),   // relleno inicial: sano
					sf::Color(40,  40,  40)	 // contorno oscuro
				);

				float px = offset_x + columna * tam_celda + tam_celda * 0.5f;
				float py = offset_y + fila	* tam_celda + tam_celda * 0.5f;
				persona->setPosicion(px, py);

				// Probabilidades individuales aleatorias dentro del rango
				// especificado en el enunciado
				float p_contagio  = 0.15f
					+ static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.15f;
				float p_inmunidad = 0.01f
					+ static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.09f;

				auto ep = std::make_shared<IVJ::IEstadoEpidemia>(fila, columna);
				ep->prob_contagio = p_contagio;
				ep->prob_inmunidad_permanente = p_inmunidad;

				persona->addComponente(ep);
				objetos.agregarPool(persona);
			}
		}

		// Infectar ~10 % de la población al azar
		const int total_personas = filas * columnas;
		const int n_infectados   = static_cast<int>(total_personas * 0.10f);

		// Barajar índices con Fisher-Yates para selección sin repetición
		std::vector<int> indices(total_personas);
		for (int k = 0; k < total_personas; ++k) indices[k] = k;
		for (int k = total_personas - 1; k > 0; --k)
		{
			int j = rand() % (k + 1);
			std::swap(indices[k], indices[j]);
		}

		for (int k = 0; k < n_infectados; ++k)
		{
			auto ep = objetos[indices[k]]->getComponente<IVJ::IEstadoEpidemia>();
			if (ep) ep->infectar();
		}

		finEpidemia = false;
		inicializar = 0;

		CE::GLogger::Get().agregarLog(
			"[Epidemia] Simulacion iniciada. Infectados: " + std::to_string(n_infectados),
			CE::GLogger::Niveles::LOG_DEBUG);
	}

	// ── onFinal ──────────────────────────────────────────────────────────────

	void Escena_Epidemia::onFinal()
	{}

	// ── onUpdate ─────────────────────────────────────────────────────────────

	void Escena_Epidemia::onUpdate(float dt)
	{
		// 1. Ejecutar la lógica epidémica ANTES de onUpdate de cada persona,
		//	para que el color visual refleje el estado del frame actual.
		if (!finEpidemia)
		{
			SistemaEpidemia(objetos, columnasGrid, dt);

			if (SistemaEpidemiaTerminada(objetos))
			{
				finEpidemia = true;
				CE::GLogger::Get().agregarLog(
					"[Epidemia] Simulacion terminada.",
					CE::GLogger::Niveles::LOG_DEBUG);
			}
		}

		// 2. Actualizar visualmente cada persona (Persona::onUpdate →
		//	actualizarColorEpidemia según el estado recién calculado).
		for (auto& obj : objetos.getPool())
		{
			obj->onUpdate(dt);
		}
	}

	// ── onInputs ─────────────────────────────────────────────────────────────

	void Escena_Epidemia::onInputs(const CE::Botones& accion)
	{
		(void)accion;
	}

	// ── onRender ─────────────────────────────────────────────────────────────

	void Escena_Epidemia::onRender()
	{
		CE::Render::Get().OnClearColor(sf::Color(245, 245, 245));

		// Dibujar cuadrícula de personas
		for (auto& obj : objetos.getPool())
		{
			CE::Render::Get().AddToDraw(*obj);
		}

		// Dibujar panel de estadísticas y título
		renderizarStats();
	}

	// renderizarEstadisticas

	void Escena_Epidemia::renderizarStats()
	{
		// Contar estados
		int n_sanos	   = 0;
		int n_parcial	 = 0;
		int n_enfermos	= 0;
		int n_recuperando = 0;
		int n_inmunes	 = 0;
		int n_muertos	 = 0;

		using Estado = IVJ::IEstadoEpidemia::Estado;

		for (auto& obj : objetos.getPool())
		{
			auto ep = obj->getComponente<IVJ::IEstadoEpidemia>();
			if (!ep) continue;
			switch (ep->estado_actual)
			{
				case Estado::Normal:
					if (ep->tiene_inmunidad_parcial)
						n_parcial++;
					else
						n_sanos++;
					break;
				case Estado::Enfermo:	  n_enfermos++;	break;
				case Estado::Recuperacion: n_recuperando++; break;
				case Estado::Inmune:	   n_inmunes++;	 break;
				case Estado::Muerto:	   n_muertos++;	 break;
			}
		}

		auto& font = CE::GestorAssets::Get().getFont("default_font");
		auto  dim  = CE::Render::Get().GetVentana().getSize();

		// ── Título centrado arriba ────────────────────────────────────────
		{
			sf::Text titulo(font, "Simulacion de Epidemia  15x15", 18);
			titulo.setFillColor(sf::Color(30, 30, 80));
			titulo.setStyle(sf::Text::Bold);
			auto bounds = titulo.getLocalBounds();
			titulo.setPosition({
				(float)dim.x * 0.5f - bounds.size.x * 0.5f,
				6.f
			});
			CE::Render::Get().AddToDraw(titulo);
		}

		// ── Panel de estadísticas (esquina superior izquierda) ────────────
		struct Stat { const char* label; int count; sf::Color color; };
		const Stat stats[] = {
			{ "Sanos",		   n_sanos,	   sf::Color(245,  245,  245)  },
			{ "Inm. parcial",	n_parcial,	 sf::Color(0,   110, 110) },
			{ "Enfermos",		n_enfermos,	sf::Color(210, 40,  0)   },
			{ "Recuperando",	 n_recuperando, sf::Color(60,  90,  200) },
			{ "Inmunes",		 n_inmunes,	 sf::Color(0,   150, 0)   },
			{ "Muertos",		 n_muertos,	 sf::Color(90,  90,  90)  },
		};

		const float x0	 = 10.f;
		const float y0	 = 30.f;
		const float line_h = 22.f;
		const float sq_sz  = 14.f;   // tamaño del cuadrito de color

		for (int i = 0; i < 6; ++i)
		{
			float y = y0 + i * line_h;

			// Cuadrito de color de referencia
			sf::RectangleShape sq({sq_sz, sq_sz});
			sq.setFillColor(stats[i].color);
			sq.setOutlineColor(sf::Color(20, 20, 20));
			sq.setOutlineThickness(1.f);
			sq.setPosition({x0, y + 2.f});
			CE::Render::Get().AddToDraw(sq);

			// Texto
			sf::Text t(font,
				std::string(stats[i].label) + ": " + std::to_string(stats[i].count),
				15);
			// si son sanos (indice 0), figura blanca pero texto negro
			if (i == 0)
				t.setFillColor((sf::Color(0,0,0)));
			else
				t.setFillColor(stats[i].color);
			t.setPosition({x0 + sq_sz + 5.f, y});
			CE::Render::Get().AddToDraw(t);
		}

		// mensaje de fin de epidemia
		if (finEpidemia)
		{
			sf::RectangleShape fondo_fin({380.f, 40.f});
			fondo_fin.setFillColor(sf::Color(20, 20, 120, 200));
			fondo_fin.setOutlineColor(sf::Color::White);
			fondo_fin.setOutlineThickness(2.f);
			fondo_fin.setPosition({(float)dim.x * 0.5f - 190.f, (float)dim.y - 52.f});
			CE::Render::Get().AddToDraw(fondo_fin);

			sf::Text t_fin(font, "  EPIDEMIA TERMINADA  ", 22);
			t_fin.setFillColor(sf::Color::White);
			t_fin.setStyle(sf::Text::Bold);
			auto b = t_fin.getLocalBounds();
			t_fin.setPosition({
				(float)dim.x * 0.5f - b.size.x * 0.5f,
				(float)dim.y - 48.f
			});
			CE::Render::Get().AddToDraw(t_fin);
		}
	}

} // namespace IVJ
