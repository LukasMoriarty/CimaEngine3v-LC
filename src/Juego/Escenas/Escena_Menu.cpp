#include "Escena_Menu.hpp"
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Primitivos/GestorEscenas.hpp>
#include <Motor/Render/Render.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <memory>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Sistemas/Sistemas.hpp>

namespace IVJ
{
	Escena_Menu::Escena_Menu()
		:CE::Escena{}
	{
	}
	void Escena_Menu::onInit()
	{
		if (!inicializar) return;
		registrarBotones(sf::Keyboard::Scancode::W,"arriba");
		registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
		registrarBotones(sf::Keyboard::Scancode::S,"abajo");
		registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
		registrarBotones(sf::Keyboard::Scancode::Enter,"aceptar");
		salto = 48.f;
		opcion = 1u;
		auto marcador = std::make_shared<Circulo>(15.f, sf::Color::Green, sf::Color::Black);
		marcador->setPosicion(470.f, 295.f);

		auto strIniciar = std::make_shared<Texto>(
			CE::GestorAssets::Get().getFont("default_font"),
			"Iniciar"
			);
		strIniciar->setPosicion(510.f, 250.f);
		strIniciar->setColor(sf::Color::Green);
		auto strSalir = std::make_shared<Texto>(
			CE::GestorAssets::Get().getFont("default_font"),
			"Salir"
			);
		strSalir->setPosicion(510.f, 300.f);

		/*
		auto cuadro = std::make_shared<Rectangulo>(
			150, 150,
			sf::Color::White,
			sf::Color::Black
		);
		cuadro->addComponente(std::make_shared<IGirar>(3.1416, 5.5f));
		cuadro->setPosicion(500, 300);

		// lab 2 y 3
		auto rect = std::make_shared<Rectangulo>(150, 150, sf::Color::Yellow, sf::Color::Black);
		rect->addComponente(std::make_shared<IArribaAbajo>(250.f, 350.f, 50.f));
		rect->setPosicion(500, 400);

		auto trian = std::make_shared<Triangulo>(150, sf::Color::Cyan, sf::Color::Black);
		trian->addComponente(std::make_shared<IWave>(5.f, 1.f, 1.f));
		trian->setPosicion(500, 500);

		auto circ = std::make_shared<Circulo>(75.f, sf::Color::Magenta, sf::Color::Black);
		circ->addComponente(std::make_shared<ICirculos>(2.f, 1.f));
		circ->setPosicion(600, 300);
		*/

		objetos.agregarPool(marcador);
		objetos.agregarPool(strIniciar);
		objetos.agregarPool(strSalir);
		//objetos.agregarPool(cuadro);
		//objetos.agregarPool(rect);
		//objetos.agregarPool(trian);
		//objetos.agregarPool(circ);

		CE::GestorAssets::Get().agregarMusica(
			"menumusic",
			ASSETS "/music/menumusic.mp3"
		);
		CE::GestorAssets::Get().getMusica("menumusic").setVolume(10.f);
		CE::GestorAssets::Get().getMusica("menumusic").play();
		CE::GestorAssets::Get().getMusica("menumusic").setLooping(true);

		inicializar=false;
	}
	void Escena_Menu::onFinal()
	{
		// detener la musica de meun
		CE::GestorAssets::Get().getMusica("menumusic").stop();
	}
	void Escena_Menu::onUpdate(float dt)
	{
		// (void)dt;
		for (auto& obj : objetos.getPool())
		{
			obj->onUpdate(dt);
			//SistemaGirar(*obj, dt);
			//SistemaRectanguloMoverUpDown(*obj, dt);
			//SistemaTrianguloWave(*obj, dt);
			//SistemaCirculoMoverEnCirculos(*obj, dt);
		}
	}
	void Escena_Menu::onInputs(const CE::Botones& accion)
	{
		// (void)accion;
		switch (accion.getTipo())
		{
			case CE::Botones::TipoAccion::OnPress:
			{
				if (accion.getNombre() == "arriba" || accion.getNombre() == "abajo")
				{
					auto select = objetos.getPool()[0]; // marcador
					auto pos = select->getTransformada()->posicion;
					pos.y = (opcion) ? pos.y + salto : pos.y - salto;
					select->setPosicion(pos.x, pos.y);
					opcion ^= 1;
				}
				if (accion.getNombre() == "aceptar")
				{
					switch (opcion)
					{
						case 0:
							std::cout << "Salir del juego exitoso\n";
							exit(EXIT_SUCCESS);
							break;
						case 1:
							CE::GestorEscenas::Get().cambiarEscena("EscenaSprite");
							break;
					}
				}
				break;
			}
			case CE::Botones::TipoAccion::OnRelease:
			{
				break;
			}
			case CE::Botones::TipoAccion::None:
			{
				break;
			}
		}
	}
	void Escena_Menu::onRender()
	{
		CE::Render::Get().OnClearColor(sf::Color(0, 0, 0));
		for (auto& obj : objetos.getPool())
		{
			CE::Render::Get().AddToDraw(*obj);
		}
	}
}