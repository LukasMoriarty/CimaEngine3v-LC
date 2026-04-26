#include "Escena_Sprite.hpp"

#include "Escena_Camara.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <memory>
#include <cmath>

#include "Juego/Figuras/Figuras.hpp"
#include "Juego/Maquinas/Bosses/IdleBoss.hpp"
#include "Juego/Maquinas/Jugador/IdleJugador.hpp"
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Sistemas/Sistemas.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Juego/objetos/TileMap.hpp"

namespace IVJ
{
	auto cuerpo = std::make_shared<Entidad>();

	Escena_Sprite::Escena_Sprite(std::shared_ptr<Entidad>& pref)
		:CE::Escena{},jugador_ref{pref}
	{}

	void Escena_Sprite::onInit()
	{
		CE::GestorCamaras::Get().setCamaraActiva(1);
		CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador_ref);

		// ── ZOOM: reducir el tamaño de la vista para que los tiles
		//   de 32px se vean más grandes en pantalla.
		//   Vista de 480×270 → cada tile ocupa ~67px en la ventana de 1600×900.
		CE::GestorCamaras::Get().getCamaraActiva().setViewSize(240.f, 135.f);

		if (!inicializar) return;

		registrarBotones(sf::Keyboard::Scancode::W,	 "arriba");
		registrarBotones(sf::Keyboard::Scancode::Up,	"arriba");
		registrarBotones(sf::Keyboard::Scancode::S,	 "abajo");
		registrarBotones(sf::Keyboard::Scancode::Down,  "abajo");
		registrarBotones(sf::Keyboard::Scancode::A,	 "izquierda");
		registrarBotones(sf::Keyboard::Scancode::Left,  "izquierda");
		registrarBotones(sf::Keyboard::Scancode::D,	 "derecha");
		registrarBotones(sf::Keyboard::Scancode::Right, "derecha");
		registrarBotones(sf::Keyboard::Scancode::Enter, "aceptar");
		registrarBotones(sf::Keyboard::Scancode::Space, "atacar"); // <-- ataque

		// ── Mapa ────────────────────────────────────────────────
		tiles_layers.push_back(TileMap());

		tiles_layers[0].loadTileMap(ASSETS "/mapas/mapa_demo.txt");

		tiles_layers[0].atlas_texture.setSmooth(false); // suavizado para que se vea mejor al hacer zoom

		// cargar textura pies
		CE::GestorAssets::Get().agregarTextura(
			"pies",
			ASSETS "/sprites/Top_Down_Survivor/feet/survivor-feet-atlas.png",
			CE::Vector2D{0.f, 0.f},
			CE::Vector2D{3456.f, 3456.f}
		);

		CE::GestorAssets::Get().agregarTextura(
			"cuerpoCuchillo",
			ASSETS "/sprites/Top_Down_Survivor/knife/survivor-knife-atlas.png",
			CE::Vector2D{0.f, 0.f},
			CE::Vector2D{3072.f, 2688.f}
		);

		// Jugador
		auto trans = jugador_ref->getTransformada();
		trans->velocidad = CE::Vector2D{50.f, 50.f};   // ajustar a escala del mapa
		jugador_ref->setPosicion(300.f, 300.f);

		// Sprite: frame 384×384
		auto pies = std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("pies"),
			384, 384,
			0.1f
		);


		// HACER QUE EL CUERPO SIGA, EN LA MISMA COORDENADA A JUGADOR_REF
		cuerpo->getTransformada()->posicion = trans->posicion; // posición inicial igual al jugador
		cuerpo->getTransformada()->velocidad = trans->velocidad; // misma velocidad para que se mueva igual
		cuerpo->addComponente(std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("cuerpoCuchillo"),
			384, 384,
			0.1f
		));
		cuerpo->addComponente(std::make_shared<CE::IControl>()); // para que el cuerpo también reciba inputs (si es necesario)

		auto cuerpoCuchillo = std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("cuerpoCuchillo"),
			384, 384,
			0.1f
		);

		jugador_ref->addComponente(pies);
		jugador_ref->addComponente(std::make_shared<CE::IControl>());

		auto maquinaEstado = std::make_shared<IMaquinaEstado>();
		maquinaEstado->fsm = std::make_shared<IdleJugador>(1, 0.05f);
		jugador_ref->addComponente(maquinaEstado);
		cuerpo->addComponente(maquinaEstado);
		jugador_ref->setFSM(maquinaEstado->fsm);
		cuerpo->setFSM(maquinaEstado->fsm);

		inicializar = false;
	}

	void Escena_Sprite::onFinal()
	{
		CE::GestorCamaras::Get().setCamaraActiva(0);
	}

	void Escena_Sprite::onUpdate(float dt)
	{
		jugador_ref->inputFSM();
		cuerpo->inputFSM();
		jugador_ref->onUpdate(dt);
		cuerpo->onUpdate(dt);
		SistemaMover(jugador_ref, dt);
		SistemaMover(cuerpo, dt);

		for(auto& obj: objetos.getPool())
		{
			obj->inputFSM();
			obj->onUpdate(dt);
			if (obj->tieneComponente<ITarget>())
			{
				auto& tpos = *obj->getComponente<ITarget>()->pos;
				SistemaNPCLookTarget(*obj, tpos);
			}
		}
	}

	void Escena_Sprite::onInputs(const CE::Botones& accion)
	{
		auto ctrl = jugador_ref->getComponente<CE::IControl>();
		if (!ctrl) return;

		bool pressed = (accion.getTipo() == CE::Botones::TipoAccion::OnPress);

		if (accion.getNombre() == "arriba")	 ctrl->arr  = pressed;
		else if (accion.getNombre() == "abajo")	  ctrl->abj  = pressed;
		else if (accion.getNombre() == "derecha")	ctrl->der  = pressed;
		else if (accion.getNombre() == "izquierda")  ctrl->izq  = pressed;
		else if (accion.getNombre() == "atacar")	 ctrl->acc  = pressed;
	}

	void Escena_Sprite::onRender()
	{
		// pintar el fondo de la escena a negro
		CE::Render::Get().OnClearColor(sf::Color(0, 0, 0));

		// Tilemap primero
		for (auto& al: tiles_layers)
			CE::Render::Get().AddToDraw(al);

		// Objetos del pool
		for(auto& obj: objetos.getPool())
		{
			if (obj->tieneComponente<IRangoAggro>())
			{
				auto rango = obj->getComponente<IRangoAggro>();
				auto pos   = obj->getTransformada()->posicion;
				rango->marcador.setPosition({pos.x, pos.y});
				CE::Render::Get().AddToDraw(rango->marcador);
			}
			CE::Render::Get().AddToDraw(*obj);
		}

		CE::Render::Get().AddToDraw(*jugador_ref);
		CE::Render::Get().AddToDraw(*cuerpo);

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
