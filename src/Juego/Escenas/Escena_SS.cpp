#include "Escena_SS.hpp"

#include <algorithm>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Juego/objetos/EnteVibora.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/GUI/GLogger.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <fstream>
#include <iostream>

namespace IVJ
{
	Escena_SS::Escena_SS(std::shared_ptr<Entidad>& pref)
		: CE::Escena{}, jugador_ref{pref}, vibora{nullptr}, timer_agregar_parte{0.f}
	{
	}

	void Escena_SS::onInit()
	{
		if(!inicializar)
			return;

		// Registrar controles para la víbora
		registrarBotones(sf::Keyboard::Scancode::W, "arriba");
		registrarBotones(sf::Keyboard::Scancode::S, "abajo");
		registrarBotones(sf::Keyboard::Scancode::A, "izquierda");
		registrarBotones(sf::Keyboard::Scancode::D, "derecha");
		registrarBotones(sf::Keyboard::Scancode::Up, "arriba");
		registrarBotones(sf::Keyboard::Scancode::Down, "abajo");
		registrarBotones(sf::Keyboard::Scancode::Left, "izquierda");
		registrarBotones(sf::Keyboard::Scancode::Right, "derecha");

		// Crear la cabeza de la víbora (ahora es un Triángulo)
		vibora = std::make_shared<EnteVibora>(
			50.f,  // tam_lado (tamaño del triángulo)
			sf::Color::Green,
			sf::Color::Black
		);

		// Posición inicial en el centro de la pantalla
		vibora->setPosicion(400.f, 300.f);

		// VELOCIDAD INICIAL
		vibora->getTransformada()->velocidad = CE::Vector2D{0.f, 100.f};

		// Agregar componente de control para mover la cabeza
		vibora->addComponente(std::make_shared<CE::IControl>());

		// PASO 5: Agregar componente ICPartesCuerpo que inicie con 3 partes
		auto cuerpo = std::make_shared<ICPartesCuerpo>(50, 50);
		vibora->addComponente(cuerpo);

		// Agregar las 3 partes iniciales
		vibora->agregarParteAlCuerpo(sf::Color::Red);
		vibora->agregarParteAlCuerpo(sf::Color::Red);
		vibora->agregarParteAlCuerpo(sf::Color::Red);

		// Agregar la víbora al pool de objetos
		objetos.agregarPool(vibora);

		/*
		// ============================================
		// OTRAS FIGURAS DE PRUEBA
		// ============================================

		figura = std::make_shared<Rectangulo>(
			100.f, 50.f, sf::Color::Red, sf::Color::Yellow);
		figura->setPosicion(350.f, 400.f);
		objetos.agregarPool(figura);

		figura2 = std::make_shared<Circulo>(
			50.f, sf::Color::Blue, sf::Color::White);
		figura2->setPosicion(250.f, 200.f);
		objetos.agregarPool(figura2);

		// Cargar figuras desde archivo (opcional)
		std::ifstream miarchivo(ASSETS "/config/figuras.txt");
		std::string tipo;
		float px, py;
		float w, h;
		int fr, fg, fb, cr, cg, cb;

		while (miarchivo >> tipo)
		{
			if (tipo == "Rectangulo")
			{
				miarchivo >> px >> py >> w >> h >> fr >> fg >> fb >> cr >> cg >> cb;
				auto nuevaFigura = std::make_shared<Rectangulo>(w, h,
					sf::Color(fr, fg, fb, 255),
					sf::Color(cr, cg, cb, 255));
				nuevaFigura->setPosicion(px, py);
				nuevaFigura->addComponente(std::make_shared<IArribaAbajo>(py - 50.f, py + 50.f, 50.f));
				objetos.agregarPool(nuevaFigura);
			}
			else if (tipo == "Circulo")
			{
				miarchivo >> px >> py >> w >> fr >> fg >> fb >> cr >> cg >> cb;
				auto nuevaFigura = std::make_shared<Circulo>(w,
					sf::Color(fr, fg, fb, 255),
					sf::Color(cr, cg, cb, 255));
				nuevaFigura->setPosicion(px, py);
				nuevaFigura->addComponente(std::make_shared<ICirculos>(2.f, 1.f));
				objetos.agregarPool(nuevaFigura);
			}
			else if (tipo == "Triangulo")
			{
				miarchivo >> px >> py >> w >> fr >> fg >> fb >> cr >> cg >> cb;
				auto nuevaFigura = std::make_shared<Triangulo>(w,
					sf::Color(fr, fg, fb, 255),
					sf::Color(cr, cg, cb, 255));
				nuevaFigura->setPosicion(px, py);
				nuevaFigura->addComponente(std::make_shared<IWave>(5.f, 1.f, 1.f));
				objetos.agregarPool(nuevaFigura);
			}
		}
		*/

		inicializar = 0;
	}

	void Escena_SS::onFinal()
	{
	}

	void Escena_SS::onUpdate(float dt)
	{
		if (vibora)
		{
			timer_agregar_parte += dt;

			// Cada 3 segundos, agregar una nueva parte
			if (timer_agregar_parte >= 3.0f)
			{
				// Colores aleatorios para las nuevas partes
				static int color_index = 0;
				sf::Color colores[] = {
					sf::Color::Blue,
					sf::Color::Yellow,
					sf::Color::Magenta,
					sf::Color::Cyan,
					sf::Color::White
				};

				vibora->agregarParteAlCuerpo(colores[color_index % 5]);
				color_index++;

				// Resetear el timer
				timer_agregar_parte = 0.f;

				CE::GLogger::Get().agregarLog(
					"Nueva parte agregada a la vibora!",
					CE::GLogger::Niveles::LOG_DEBUG
				);
			}
		}

		// Actualizar todos los objetos
		for (auto& obj : objetos.getPool())
		{
			obj->onUpdate(dt);

			// Aplicar sistemas de movimiento a otras figuras
			SistemaRectanguloMoverUpDown(*obj, dt);
			SistemaTrianguloWave(*obj, dt);
			SistemaCirculoMoverEnCirculos(*obj, dt);
		}

		// Logger de debug
		static float timer{0.f};
		timer += dt;
		CE::GLogger::Get().agregarLog(
			"Timer:" + std::to_string(timer),
			CE::GLogger::Niveles::LOG_DEBUG
		);
	}

	void Escena_SS::onInputs(const CE::Botones& accion)
	{
		if (!vibora) return;

		auto trans = vibora->getTransformada();
		if (!trans) return;

		// Solo procesar al presionar tecla (cambio de dirección)
		if (accion.getTipo() != CE::Botones::TipoAccion::OnPress) return;

		float velocidad = 100.f;
		CE::Vector2D vel_actual = trans->velocidad;

		// Cambiar dirección según la tecla presionada
		if (accion.getNombre() == "arriba" && vel_actual.y == 0)
		{
			trans->velocidad = CE::Vector2D{0.f, -velocidad};
		}
		else if (accion.getNombre() == "abajo" && vel_actual.y == 0)
		{
			trans->velocidad = CE::Vector2D{0.f, velocidad};
		}
		else if (accion.getNombre() == "izquierda" && vel_actual.x == 0)
		{
			trans->velocidad = CE::Vector2D{-velocidad, 0.f};
		}
		else if (accion.getNombre() == "derecha" && vel_actual.x == 0)
		{
			trans->velocidad = CE::Vector2D{velocidad, 0.f};
		}
	}

	void Escena_SS::onRender()
	{
		for (auto& obj : objetos.getPool())
		{
			CE::Render::Get().AddToDraw(*obj);
		}
	}
}