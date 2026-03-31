#include "Escena_Sprite.hpp"

#include "Escena_Camara.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <memory>
#include <cmath>

#include "Juego/Figuras/Figuras.hpp"
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Sistemas/Sistemas.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Juego/objetos/TileMap.hpp"

namespace IVJ
{
	Escena_Sprite::Escena_Sprite(std::shared_ptr<Entidad>& pref)
		:CE::Escena{},jugador_ref{pref}
	{}

	void Escena_Sprite::onInit()
	{
		CE::GestorCamaras::Get().setCamaraActiva(2);
		CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador_ref);
		if (!inicializar) return;
		registrarBotones(sf::Keyboard::Scancode::W,"arriba");
		registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
		registrarBotones(sf::Keyboard::Scancode::S,"abajo");
		registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
		registrarBotones(sf::Keyboard::Scancode::A,"izquierda");
		registrarBotones(sf::Keyboard::Scancode::Left,"izquierda");
		registrarBotones(sf::Keyboard::Scancode::D,"derecha");
		registrarBotones(sf::Keyboard::Scancode::Right,"derecha");
		registrarBotones(sf::Keyboard::Scancode::Enter,"aceptar");

		std::cout << "Cargando mapa..." << std::endl;
		// cargar mapa 3 layers
		tiles_layers.push_back(TileMap());	// solo mar
		tiles_layers.push_back(TileMap());	// las islas
		tiles_layers.push_back(TileMap());	// los objetos
		std::cout << "Mapas cargados..." << std::endl;

		if (!tiles_layers[0].loadTileMap(ASSETS "/mapas/playa_layer1.txt"))
			exit(EXIT_FAILURE);
		if (!tiles_layers[1].loadTileMap(ASSETS "/mapas/playa_layer2.txt"))
			exit(EXIT_FAILURE);
		if (!tiles_layers[2].loadTileMap(ASSETS "/mapas/playa_layer3.txt"))
			exit(EXIT_FAILURE);


		// CARGAR EL SPRITE
		CE::GestorAssets::Get().agregarTextura(
			"naveb",
			ASSETS "/sprites/jugador/player_b.png",
			CE::Vector2D{0.f, 0.f},
			CE::Vector2D{64.f,64.f});

		auto trans = jugador_ref->getTransformada();
		trans->velocidad = CE::Vector2D{500.f, 500.f};
		jugador_ref->setPosicion(300.f, 300.f);

		auto sprite = std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("naveb"),
			64.f, 64.f,
			1.f);

		jugador_ref->addComponente(sprite);
		jugador_ref->addComponente(std::make_shared<CE::IControl>());

		/*
		// OBJETOS PARA QUE SE MUESTRE EL MOVIMIENTO
		int montes_count = 100;
		float dstd = 20.f;
		for (int i = 0; i < montes_count; i++)
		{
			// GAUSS
			double por = std::exp(-0.5 * (((i - montes_count / 2.f) * (i - montes_count / 2.f)) / (dstd * dstd)));
			int inc = 200;
			auto monte = std::make_shared<Rectangulo>(
				200.f, 200.f + (inc * por),
				sf::Color{184, 134, 11}, sf::Color::Black);
			monte->setPosicion(100 + (i * 200), 100 - (inc * por / 2.f));
			objetos.agregarPool(monte);
		}
		*/
		inicializar = false;
	}
	void Escena_Sprite::onFinal()
	{
		// resetear la camara a la estatica al salir/cambiar de escena
		CE::GestorCamaras::Get().setCamaraActiva(0);
	}
	void Escena_Sprite::onUpdate(float dt)
	{
		jugador_ref->onUpdate(dt);
		SistemaMover(jugador_ref, dt);
		for(auto& obj: objetos.getPool())
		{
			obj->onUpdate(dt);
		}
	}
	void Escena_Sprite::onInputs(const CE::Botones& accion)
	{
		switch(accion.getTipo())
		{
			case CE::Botones::TipoAccion::OnPress:
			{
				if (accion.getNombre() == "arriba")
					jugador_ref->getComponente<CE::IControl>()->arr = true;
				if (accion.getNombre() == "abajo")
					jugador_ref->getComponente<CE::IControl>()->abj = true;
				if (accion.getNombre() == "derecha")
					jugador_ref->getComponente<CE::IControl>()->der = true;
				if (accion.getNombre() == "izquierda")
					jugador_ref->getComponente<CE::IControl>()->izq = true;
				break;
			}
			case CE::Botones::TipoAccion::OnRelease:
			{
				if (accion.getNombre() == "arriba")
					jugador_ref->getComponente<CE::IControl>()->arr = false;
				if (accion.getNombre() == "abajo")
					jugador_ref->getComponente<CE::IControl>()->abj = false;
				if (accion.getNombre() == "derecha")
					jugador_ref->getComponente<CE::IControl>()->der = false;
				if (accion.getNombre() == "izquierda")
					jugador_ref->getComponente<CE::IControl>()->izq = false;
				break;
			}
			case CE::Botones::TipoAccion::None:
			{
				break;
			}
		}
	}
	void Escena_Sprite::onRender()
	{
		// renderizar los layers primero para pintar el fondo
		for (auto& al: tiles_layers)
			CE::Render::Get().AddToDraw(al);

		for(auto& obj: objetos.getPool())
			CE::Render::Get().AddToDraw(*obj);

		CE::Render::Get().AddToDraw(*jugador_ref);

#if DEBUG
		auto cam = &CE::GestorCamaras::Get().getCamaraActiva();
		auto csv = (CE::CamaraSnapVentana*) cam;
		auto csvpos = csv->getTransformada().posicion;
		sf::RectangleShape debugcam{{csv->m_vdim.x, csv->m_vdim.y}};
		debugcam.setOrigin({csv->m_vdim.x/2.f, csv->m_vdim.y/2.f});
		debugcam.setPosition({csvpos.x, csvpos.y});
		debugcam.setOutlineThickness(3.f);
		debugcam.setOutlineColor(sf::Color::Yellow);
		debugcam.setFillColor(sf::Color::Transparent);
		CE::Render::Get().AddToDraw(debugcam);
#endif
	}
}
