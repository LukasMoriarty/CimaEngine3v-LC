#include "Escena_Sprite.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Motor/Utils/AnimLoader.hpp>

#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Juego/Componentes/IJComponentes.hpp>

#include <Juego/Maquinas/Pies/IdlePies.hpp>

#include <Juego/Maquinas/Cuerpo/IdleCuerpoKnife.hpp>

#include <Juego/Maquinas/Zombie/IdleZombie.hpp>

#include <memory>

namespace IVJ
{
	Escena_Sprite::Escena_Sprite(std::shared_ptr<Entidad>& pref)
		: CE::Escena{}, jugador_ref{pref}, cuerpo{std::make_shared<Entidad>()}
	{}

	void Escena_Sprite::onInit()
	{
		CE::GestorCamaras::Get().setCamaraActiva(1);
		CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador_ref);
		CE::GestorCamaras::Get().getCamaraActiva().setViewSize(800.f, 450.f);

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

		// Tilemap
		tiles_layers.push_back(TileMap());
		tiles_layers.push_back(TileMap());
		tiles_layers[0].loadTileMap(ASSETS "/mapas/floors.txt");
		//tiles_layers[1].loadTileMap(ASSETS "/mapas/walls.txt");
		tiles_layers[1].loadTileMap(ASSETS "/mapas/walls_colisiones.txt", objetos);
		tiles_layers[0].atlas_texture.setSmooth(true);
		tiles_layers[1].atlas_texture.setSmooth(true);

		CE::GestorAssets::Get().agregarMusica(
			"musica_fondo",
			ASSETS "/music/first_wave.mp3"
		);
		// reproducir musica de fondo (loop)
		CE::GestorAssets::Get().getMusica("musica_fondo").play();
		CE::GestorAssets::Get().getMusica("musica_fondo").setVolume(20.f);

		// Cargar texturas
		CE::GestorAssets::Get().agregarTextura(
			"pies",
			ASSETS "/sprites/Top_Down_Survivor/feet/survivor-feet-atlas.png",
			CE::Vector2D{0.f, 0.f}, CE::Vector2D{3456.f, 3456.f});

		CE::GestorAssets::Get().agregarTextura(
			"cuerpoCuchillo",
			ASSETS "/sprites/Top_Down_Survivor/knife/survivor-knife-atlas.png",
			CE::Vector2D{0.f, 0.f}, CE::Vector2D{3072.f, 2688.f});

		CE::GestorAssets::Get().agregarTextura(
			"zombie",
			ASSETS "/sprites/zombie/zombie-atlas.png",
			CE::Vector2D{0.f, 0.f}, CE::Vector2D{2688.f, 2688.f});

		// METADATA DE ANIMACION
		auto clips_pies = CE::AnimLoader::cargar(
			ASSETS "/sprites/Top_Down_Survivor/feet/indices_animacion.json");
		auto clips_knife = CE::AnimLoader::cargar(
			ASSETS "/sprites/Top_Down_Survivor/knife/indices_animacion.json");
		auto clips_zombie = CE::AnimLoader::cargar(
			ASSETS "/sprites/zombie/indices_animacion.json");

		// PIES (jugador_ref)
		auto trans = jugador_ref->getTransformada();
		trans->velocidad = CE::Vector2D{150.f, 150.f};
		jugador_ref->setPosicion(1750.f, 1750.f);

		jugador_ref->addComponente(std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("pies"),
			384, 384, .5f));

		jugador_ref->addComponente(std::make_shared<CE::IControl>());

		// caja de colisiones para el jugador
		jugador_ref->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{64.f, 64.f}));

		auto fsm_pies = std::make_shared<IdlePies>(
			CE::AnimLoader::getClip(clips_pies, "idle"));

		auto mq_pies = std::make_shared<IMaquinaEstado>();
		mq_pies->fsm = fsm_pies;
		jugador_ref->addComponente(mq_pies);
		jugador_ref->setFSM(fsm_pies);

		// CUERPO
		// El cuerpo siempre se posiciona igual que los pies (en onUpdate)
		cuerpo->getTransformada()->posicion = trans->posicion;
		cuerpo->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};

		cuerpo->addComponente(std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("cuerpoCuchillo"),
			384, 384, .5f));

		// El cuerpo necesita IControl para que inputFSM funcione.
		// Se sincroniza en onUpdate con el control de jugador_ref.
		cuerpo->addComponente(std::make_shared<CE::IControl>());

		// FSM de cuerpo
		auto fsm_cuerpo = std::make_shared<IdleCuerpoKnife>(
			CE::AnimLoader::getClip(clips_knife, "idle_knife"));

		auto mq_cuerpo = std::make_shared<IMaquinaEstado>();
		mq_cuerpo->fsm = fsm_cuerpo;
		cuerpo->addComponente(mq_cuerpo);
		cuerpo->setFSM(fsm_cuerpo);

		// ZOMBIE
		auto zombie = std::make_shared<Entidad>();
		zombie->getTransformada()->posicion = CE::Vector2D{2200.f, 2200.f};
		zombie->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};
		zombie->addComponente(std::make_shared<CE::ISprite>(
			CE::GestorAssets::Get().getTextura("zombie"),
			384, 384, .5f));

		auto fsm_zombie = std::make_shared<IdleZombie>(
			CE::AnimLoader::getClip(clips_zombie, "skeleton-idle"));

		auto mq_zombie = std::make_shared<IMaquinaEstado>();
		mq_zombie->fsm = fsm_zombie;
		zombie->addComponente(mq_zombie);
		zombie->setFSM(fsm_zombie);

		zombie->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f, 32.f}));

		zombie->addComponente(std::make_shared<IRangoAggro>(100.f));

		zombie->addComponente(std::make_shared<ITarget>(jugador_ref.get()));
		//agregar a pool
		objetos.agregarPool(zombie);

		if (sf::Joystick::isConnected(0))
			std::cout << "Gamepad conectado: " << sf::Joystick::getIdentification(0).name.toAnsiString() << std::endl;

		inicializar = false;
	}

	void Escena_Sprite::onFinal()
	{
		CE::GestorCamaras::Get().setCamaraActiva(0);
	}

	void Escena_Sprite::onUpdate(float dt)
	{
		// 1. Sincronizar control del cuerpo con el de los pies
		//	(ambas entidades reaccionan a los mismos inputs)
		auto ctrl_pies   = jugador_ref->getComponente<CE::IControl>();
		auto ctrl_cuerpo = cuerpo->getComponente<CE::IControl>();

		// =======================================================
		// 0. SOPORTE PARA GAMEPAD (Sobrescribe inputs en tiempo real)
		// =======================================================
		if (ctrl_pies && sf::Joystick::isConnected(0)) {
			float x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
			float y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
			float deadzone = 25.f; // Evita que Dieter se mueva solo si el joystick está flojo

			// Usamos || (OR) lógico para que funcione YA SEA con el Gamepad o el Teclado.
			ctrl_pies->der = (x > deadzone) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
			ctrl_pies->izq = (x < -deadzone) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
			ctrl_pies->arr = (y < -deadzone) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
			ctrl_pies->abj = (y > deadzone) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

			// El Botón 1 suele ser la 'B' (o botón derecho del pad de acciones).
			// El Botón 0 suele ser la 'A'. Si ataca con A, cambia el 1 por un 0.
			ctrl_pies->acc = sf::Joystick::isButtonPressed(0, 10) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
		}

		if (ctrl_pies && ctrl_cuerpo)
			*ctrl_cuerpo = *ctrl_pies;

		// 2. Procesar transiciones de FSM
		jugador_ref->inputFSM();
		cuerpo->inputFSM();

		// 3. Actualizar lógica interna
		jugador_ref->onUpdate(dt);
		cuerpo->onUpdate(dt);

		// 4. Mover pies según input; el cuerpo se arrastra a la misma posición
		SistemaMover(jugador_ref, dt);
		cuerpo->getTransformada()->posicion = jugador_ref->getTransformada()->posicion;

		// 5. Sincronizar ángulo (rotación según dirección del movimiento)
		cuerpo->getTransformada()->angulo = jugador_ref->getTransformada()->angulo;
		if (cuerpo->getComponente<CE::ISprite>())
		{
			cuerpo->getComponente<CE::ISprite>()->m_sprite.setRotation(
				sf::radians(cuerpo->getTransformada()->angulo));
		}

		// 6. Actualizar objetos del pool (NPCs, etc.)
		for (auto& obj : objetos.getPool())
		{
			obj->inputFSM();
			obj->onUpdate(dt);
			if (obj->tieneComponente<ITarget>())
			{
				auto& tpos = *obj->getComponente<ITarget>()->pos;
				SistemaNPCLookTarget(*obj, tpos);
			}
			// se agrega todas las cajas de colisiiones generadas del tilemap al sistema de colisiones
			SistemaColAABB(*jugador_ref, *obj, true);
		}
	}

	void Escena_Sprite::onInputs(const CE::Botones& accion)
	{
		auto ctrl = jugador_ref->getComponente<CE::IControl>();
		if (!ctrl) return;

		bool pressed = (accion.getTipo() == CE::Botones::TipoAccion::OnPress);

		if (accion.getNombre() == "arriba")	ctrl->arr = pressed;
		else if (accion.getNombre() == "abajo") ctrl->abj = pressed;
		else if (accion.getNombre() == "derecha") ctrl->der = pressed;
		else if (accion.getNombre() == "izquierda") ctrl->izq = pressed;
		else if (accion.getNombre() == "atacar") ctrl->acc = pressed;
		// Nota: el control del cuerpo se sincroniza al inicio de onUpdate
	}

	void Escena_Sprite::onRender()
	{
		CE::Render::Get().OnClearColor(sf::Color(0, 0, 0));

		// Tilemap al fondo
		for (auto& al : tiles_layers)
			CE::Render::Get().AddToDraw(al);

		// Objetos del pool
		for (auto& obj: objetos.getPool())
		{
			if (obj->tieneComponente<IRangoAggro>())
			{
				auto rango = obj->getComponente<IRangoAggro>();
				rango->marcador.setPosition(
					{obj->getTransformada()->posicion.x,
					 obj->getTransformada()->posicion.y});
				CE::Render::Get().AddToDraw(rango->marcador);
			}
			CE::Render::Get().AddToDraw(*obj);
		}

		// Pies primero (debajo), luego cuerpo (encima)
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
