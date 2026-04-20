#include "Escena_Sprite.hpp"

#include "Escena_Camara.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <memory>
#include <cmath>

#include "Juego/Figuras/Figuras.hpp"
#include "Juego/Maquinas/Bosses/IdleBoss.hpp"
// ── Jugador FSM (nuevo) ──────────────────────────────────────────
#include "Juego/Maquinas/Jugador/IdleJugador.hpp"
// ────────────────────────────────────────────────────────────────
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

		tiles_layers[0].atlas_texture.setSmooth(true); // suavizado para que se vea mejor al hacer zoom

		// ── Textura jugador (survivor-knife) ────────────────────
		// Hoja completa: 3072×2688, frame: 384×384
		CE::GestorAssets::Get().agregarTextura(
			"jugador",
			ASSETS "/sprites/Top_Down_Survivor/knife/survivor-knife.png",
			CE::Vector2D{0.f, 0.f},
			CE::Vector2D{3072.f, 2688.f}
		);

		// ── Jugador ─────────────────────────────────────────────
		auto trans = jugador_ref->getTransformada();
		trans->velocidad = CE::Vector2D{50.f, 50.f};   // ajustar a escala del mapa
		jugador_ref->setPosicion(300.f, 300.f);

		// Sprite: frame 384×384
		auto sprite = std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("jugador"),
			384, 384,
			0.1f
		);
		jugador_ref->addComponente(sprite);
		jugador_ref->addComponente(std::make_shared<CE::IControl>());

		// ── FSM Jugador ─────────────────────────────────────────
		// 20 frames de idle, 0.05s por frame → animación fluida
		auto me = std::make_shared<IMaquinaEstado>();
		me->fsm = std::make_shared<IdleJugador>(20, 0.05f);
		jugador_ref->addComponente(me);
		jugador_ref->setFSM(me->fsm);

		inicializar = false;
	}

	void Escena_Sprite::onFinal()
	{
		CE::GestorCamaras::Get().setCamaraActiva(0);
	}

	void Escena_Sprite::onUpdate(float dt)
	{
		jugador_ref->inputFSM();
		jugador_ref->onUpdate(dt);
		SistemaMover(jugador_ref, dt);

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

		if	  (accion.getNombre() == "arriba")	 ctrl->arr  = pressed;
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
