#include "Escena_Sprite.hpp"
#include <Motor/Camaras/CamarasGestor.hpp>
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Utils/AnimLoader.hpp>

#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/Sistemas/Sistemas.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Juego/Maquinas/Pies/IdlePies.hpp>
#include <Juego/Maquinas/Cuerpo/IdleCuerpoKnife.hpp>
#include <Juego/Maquinas/Pistola/IdleCuerpoPistola.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

#include "Juego/Maquinas/Balas/BalasBoom.hpp"

namespace IVJ {
	Escena_Sprite::Escena_Sprite(std::shared_ptr<Entidad> &pref) :
		CE::Escena{}, jugador_ref{pref}, cuerpo{std::make_shared<Entidad>()} {}

	void Escena_Sprite::onInit() {
		CE::GestorCamaras::Get().setCamaraActiva(1);
		CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador_ref);
		CE::GestorCamaras::Get().getCamaraActiva().setViewSize(800.f, 450.f);

		if (!inicializar)
			return;

		registrarBotones(sf::Keyboard::Scancode::W, "arriba");
		registrarBotones(sf::Keyboard::Scancode::Up, "arriba");
		registrarBotones(sf::Keyboard::Scancode::S, "abajo");
		registrarBotones(sf::Keyboard::Scancode::Down, "abajo");
		registrarBotones(sf::Keyboard::Scancode::A, "izquierda");
		registrarBotones(sf::Keyboard::Scancode::Left, "izquierda");
		registrarBotones(sf::Keyboard::Scancode::D, "derecha");
		registrarBotones(sf::Keyboard::Scancode::Right, "derecha");
		registrarBotones(sf::Keyboard::Scancode::Enter, "aceptar");
		registrarBotones(sf::Keyboard::Scancode::Space, "atacar"); // <-- ataque
		registrarBotones(sf::Keyboard::Scancode::LControl, "correr");
		registrarBotones(sf::Keyboard::Scancode::Q, "anterior_arma");
		registrarBotones(sf::Keyboard::Scancode::E, "siguiente_arma");
		registrarBotones(sf::Keyboard::Scancode::R, "recargar");

		// Tilemap
		tiles_layers.push_back(TileMap());
		tiles_layers.push_back(TileMap());
		tiles_layers[0].loadTileMap(ASSETS "/mapas/floors.txt");
		// tiles_layers[1].loadTileMap(ASSETS "/mapas/walls.txt");
		// pass both the objects pool and the spawn pool to the tilemap loader
		tiles_layers[1].loadTileMap(ASSETS "/mapas/walls_colisiones.txt", objetos, m_spawn_pool);
		tiles_layers[0].atlas_texture.setSmooth(true);
		tiles_layers[1].atlas_texture.setSmooth(true);

		CE::GestorAssets::Get().agregarSonido("slash01", ASSETS "/sfx/machete_slash_flesh_attack_01.wav");
		CE::GestorAssets::Get().agregarSonido("slash02", ASSETS "/sfx/machete_slash_flesh_attack_02.wav");
		CE::GestorAssets::Get().agregarSonido("slash03", ASSETS "/sfx/machete_slash_flesh_attack_03.wav");
		CE::GestorAssets::Get().agregarSonido("slash04", ASSETS "/sfx/machete_slash_flesh_attack_04.wav");
		CE::GestorAssets::Get().agregarSonido("swing01", ASSETS "/sfx/machete_swing_01.wav");
		CE::GestorAssets::Get().agregarSonido("swing02", ASSETS "/sfx/machete_swing_02.wav");
		CE::GestorAssets::Get().agregarSonido("swing03", ASSETS "/sfx/machete_swing_03.wav");
		CE::GestorAssets::Get().agregarSonido("swing04", ASSETS "/sfx/machete_swing_04.wav");
		CE::GestorAssets::Get().agregarSonido("pistol_shot", ASSETS "/sfx/pistol/pistol_shot.mp3");
		CE::GestorAssets::Get().agregarSonido("pistol_reload", ASSETS "/sfx/pistol/pistol_reload.mp3");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_01", ASSETS "/sfx/zombie_attack/attack_01.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_01", ASSETS "/sfx/zombie_hurt/hurt_01.wav");

		CE::GestorAssets::Get().agregarMusica("musica_fondo", ASSETS "/music/pausemusic.mp3");
		// reproducir musica de fondo (loop)
		CE::GestorAssets::Get().getMusica("musica_fondo").play();
		CE::GestorAssets::Get().getMusica("musica_fondo").setVolume(20.f);

		// Cargar texturas
		CE::GestorAssets::Get().agregarTextura("pies", ASSETS "/sprites/Top_Down_Survivor/feet/survivor-feet-atlas.png",
											   CE::Vector2D{0.f, 0.f}, CE::Vector2D{3456.f, 3456.f});

		CE::GestorAssets::Get().agregarTextura("cuerpoCuchillo",
											   ASSETS "/sprites/Top_Down_Survivor/knife/survivor-knife-atlas.png",
											   CE::Vector2D{0.f, 0.f}, CE::Vector2D{3072.f, 2688.f});

		CE::GestorAssets::Get().agregarTextura("cuerpoPistola",
											   ASSETS "/sprites/Top_Down_Survivor/handgun/survivor-handgun-atlas.png",
											   CE::Vector2D{0.f, 0.f}, CE::Vector2D{3456.f, 3456.f});
		CE::GestorAssets::Get().agregarTextura("zombie", ASSETS "/sprites/zombie/zombie-atlas.png",
											   CE::Vector2D{0.f, 0.f}, CE::Vector2D{2688.f, 2688.f});

		// METADATA DE ANIMACION
		auto clips_pies = CE::AnimLoader::cargar(ASSETS "/sprites/Top_Down_Survivor/feet/indices_animacion.json");
		auto clips_knife = CE::AnimLoader::cargar(ASSETS "/sprites/Top_Down_Survivor/knife/indices_animacion.json");
		auto clips_handgun = CE::AnimLoader::cargar(ASSETS "/sprites/Top_Down_Survivor/handgun/indices_animacion.json");
		clips_zombie = CE::AnimLoader::cargar(ASSETS "/sprites/zombie/indices_animacion.json");

		// PIES (jugador_ref)
		auto trans = jugador_ref->getTransformada();
		trans->velocidad = CE::Vector2D{200.f, 200.f};
		jugador_ref->setPosicion(1750.f, 1750.f);
		jugador_ref->getStats()->hp = 100;
		jugador_ref->getStats()->hp_max = 100;

		jugador_ref->addComponente(
				std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("pies"), 384, 384, .5f));

		jugador_ref->addComponente(std::make_shared<CE::IControl>());

		// caja de colisiones para el jugador
		jugador_ref->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f, 32.f}));

		// prefab
		static auto bala_prefab = std::make_shared<Entidad>();
		bala_prefab->getStats()->hp_max = 1;
		bala_prefab->getStats()->hp = 1;
		bala_prefab->getStats()->str = 64;
		bala_prefab->getTransformada()->velocidad = CE::Vector2D{750.f, 750.f};
		CE::GestorAssets::Get().agregarTextura("balas1", ASSETS "/fx/balas1.png",
											   CE::Vector2D{0.f, 0.f}, CE::Vector2D{48.f, 16.f});
		auto bala_sprite = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("balas1"), 16, 16, 3.f);
		bala_prefab->addComponente(bala_sprite);
		bala_prefab->addComponente(std::make_shared<CE::IBoundingBox>(16));
		bala_prefab->addComponente(std::make_shared<CE::ITimer>(180)); // 3seg a 60fps
		auto bala_me = std::make_shared<IVJ::IMaquinaEstado>();
		bala_prefab->addComponente(bala_me);
		auto emisor = std::make_shared<IEmisor>(*jugador_ref, *bala_prefab);
		jugador_ref->addComponente(emisor);
		//fin lab11 prefab

		auto fsm_pies = std::make_shared<IdlePies>(CE::AnimLoader::getClip(clips_pies, "idle"));

		auto mq_pies = std::make_shared<IMaquinaEstado>();
		mq_pies->fsm = fsm_pies;
		jugador_ref->addComponente(mq_pies);
		jugador_ref->setFSM(fsm_pies);

		// CUERPO
		// El cuerpo siempre se posiciona igual que los pies (en onUpdate)
		cuerpo->getTransformada()->posicion = trans->posicion;
		cuerpo->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};

		cuerpo->addComponente(
				std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("cuerpoCuchillo"), 384, 384, .5f));

		// El cuerpo necesita IControl para que inputFSM funcione.
		// Se sincroniza en onUpdate con el control de jugador_ref.
		cuerpo->addComponente(std::make_shared<CE::IControl>());

		cuerpo->addComponente(std::make_shared<IArmaMelee>(32.f, 32.f, 20));
		cuerpo->addComponente(std::make_shared<IPistola>());

		// crear e inicializar el Inventario del Jugador
		auto inventario = std::make_shared<IInventario>();

		auto item_cuchillo = std::make_shared<IItem>();
		item_cuchillo->tipo = IItem::Tipo::Cuchillo;
		item_cuchillo->nombre = "Cuchillo";
		item_cuchillo->key_textura = "cuerpoCuchillo";
		item_cuchillo->json_anims = ASSETS "/sprites/Top_Down_Survivor/knife/indices_animacion.json";
		item_cuchillo->sprite_w = 384;
		item_cuchillo->sprite_h = 384;
		item_cuchillo->sprite_escala = 0.5f;

		auto item_pistola = std::make_shared<IItem>();
		item_pistola->tipo = IItem::Tipo::Pistola;
		item_pistola->nombre = "Pistola";
		item_pistola->key_textura = "cuerpoPistola";
		item_pistola->json_anims = ASSETS "/sprites/Top_Down_Survivor/handgun/indices_animacion.json";
		item_pistola->sprite_w = 384;
		item_pistola->sprite_h = 384;
		item_pistola->sprite_escala = 0.5f;

		inventario->agregarItem(item_cuchillo);
		inventario->agregarItem(item_pistola);
		jugador_ref->addComponente(inventario);

		// FSM inicial del cuerpo (se establecera dinamicamente en actualizarArmaEquipada
		auto mq_cuerpo = std::make_shared<IMaquinaEstado>();
		// asginar una FSM temporal que luego se actualizara
		mq_cuerpo->fsm = std::make_shared<IdleCuerpoKnife>(CE::AnimLoader::getClip(clips_knife, "idle_knife"));;
		cuerpo->addComponente(mq_cuerpo);

		objetos.agregarPool(cuerpo);

		// Configuración visual del texto del HUD
		hud_texto = std::make_unique<sf::Text>(CE::GestorAssets::Get().getFont("default_font"), "", 24);
		hud_texto->setFillColor(sf::Color::White);
		hud_texto->setOutlineColor(sf::Color::Black);
		hud_texto->setOutlineThickness(1.5f);

		// Equipar el arma inicial activa
		actualizarArmaEquipada();

		if (sf::Joystick::isConnected(0))
			std::cout << "Gamepad conectado: " << sf::Joystick::getIdentification(0).name.toAnsiString() << std::endl;

		// Inicializar shader de niebla
		if (!fog_shader.loadFromFile(ASSETS "/shaders/fog.frag", sf::Shader::Type::Fragment)) {
			std::cerr << "No se pudo cargar el shader fog.frag" << std::endl;
		}

		sf::Image transImage;
		transImage.resize(sf::Vector2u(16, 16), sf::Color::Transparent);
		if (!fog_tex.loadFromImage(transImage)) {
			std::cerr << "Error al cargar transImage a fog_tex" << std::endl;
		}

		fog_rect.setSize({4000.f, 4000.f});
		fog_rect.setPosition({0.f, 0.f});
		fog_rect.setTexture(&fog_tex);

		inicializar = false;
	}

	void Escena_Sprite::onFinal() { CE::GestorCamaras::Get().setCamaraActiva(0); }

	void Escena_Sprite::onUpdate(float dt) {
		fog_tiempo += dt;

		// 1. Sincronizar control del cuerpo con el de los pies
		//	(ambas entidades reaccionan a los mismos inputs)
		auto ctrl_pies = jugador_ref->getComponente<CE::IControl>();
		auto ctrl_cuerpo = cuerpo->getComponente<CE::IControl>();

		// Polling del teclado SIEMPRE cada frame (evita teclas pegadas).
		// Usar Scancode (independiente del layout) y cubrir ambos mapeos
		// (WASD + flechas) para que soltar una tecla mientras la otra sigue
		// presionada no deje el flag pegado en true.
		if (ctrl_pies)
		{
			ctrl_pies->arr = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W) ||
							 sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up);
			ctrl_pies->abj = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S) ||
							 sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down);
			ctrl_pies->izq = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A) ||
							 sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left);
			ctrl_pies->der = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D) ||
							 sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right);
			ctrl_pies->acc = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space);
			ctrl_pies->run = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl);

			// Gamepad: se combina (OR) con el estado del teclado
			if (sf::Joystick::isConnected(0)) {
				float x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
				float y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
				constexpr float deadzone = 25.f;
				ctrl_pies->der = ctrl_pies->der || (x > deadzone);
				ctrl_pies->izq = ctrl_pies->izq || (x < -deadzone);
				ctrl_pies->arr = ctrl_pies->arr || (y < -deadzone);
				ctrl_pies->abj = ctrl_pies->abj || (y > deadzone);
				ctrl_pies->acc = ctrl_pies->acc || sf::Joystick::isButtonPressed(0, 10);
			}

				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
				ctrl_pies->acc = true;
			}
		}

		if (ctrl_pies && ctrl_cuerpo)
			*ctrl_cuerpo = *ctrl_pies;

		// 2a. Calcular dirección normalizada hacia el mouse para FSM de animación
		auto mp = CE::Render::Get().GetMouseGamePos();
		sf::Vector2f wPos = CE::Render::Get().GetTextura().mapPixelToCoords(
				sf::Vector2i((int) mp.x, (int) mp.y), CE::GestorCamaras::Get().getCamaraActiva().getView());

		auto &pos = jugador_ref->getTransformada()->posicion;
		float dx = wPos.x - pos.x;
		float dy = wPos.y - pos.y;
		float dist = std::sqrt(dx * dx + dy * dy);
		float mouse_nx = 0.f, mouse_ny = 0.f;
		if (dist > 0.f) {
			mouse_nx = dx / dist;
			mouse_ny = dy / dist;
		}
		// Pasar la dirección del mouse al jugador para que el FSM la use
		jugador_ref->setMouseDirection(mouse_nx, mouse_ny);
		cuerpo->setMouseDirection(mouse_nx, mouse_ny);

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
		if (cuerpo->getComponente<CE::ISprite>()) {
			cuerpo->getComponente<CE::ISprite>()->m_sprite.setRotation(sf::radians(cuerpo->getTransformada()->angulo));
		}

		// 6. Combate, disparo e inventario
        auto inv = jugador_ref->getComponente<IInventario>();
        IItem* item_activo = inv ? inv->getItemActivo() : nullptr;

        if (item_activo)
        {
            if (item_activo->tipo == IItem::Tipo::Cuchillo)
            {
                // Ejecutar sistema de ataque cuerpo a cuerpo
                SistemaAtaqueMelee(*cuerpo, objetos);
            }
            else if (item_activo->tipo == IItem::Tipo::Pistola)
            {
                auto arma = cuerpo->getComponente<IPistola>();
                if (arma) {
                    // Actualizar tiempos internos de la pistola (cooldowns, recargas)
                    arma->onUpdate(dt);

                    // Disparar si se presiona la acción y la pistola está lista
                    if (ctrl_pies && ctrl_pies->acc && arma->puedeDisparar()) {
                        auto& bala_tex = CE::GestorAssets::Get().getTextura("balas1");
                        SistemaDisparar(*jugador_ref, *cuerpo, *arma, objetos, bala_tex);
                    }
                }
            }
        }

        // Actualizar posición de balas y evaluar impactos contra zombies (ITarget)
        SistemaActualizarBalas(objetos, dt);
        SistemaColBalas(objetos);

        // Actualizar HUD
        if (item_activo)
        {
            std::wstring info = L"VIDA: " + std::to_wstring((int)jugador_ref->getStats()->hp) + L"\n" +
                                L"ARMA: " + std::wstring(item_activo->nombre.begin(), item_activo->nombre.end()) + L"\n";
            if (item_activo->tipo == IItem::Tipo::Pistola)
            {
                auto arma = cuerpo->getComponente<IPistola>();
                if (arma)
                {
                    if (arma->recargando)
                    {
                        info += L"MUNICION: RECARGANDO...";
                    }
                	else
                	{
                        info += L"MUNICION: " + std::to_wstring(arma->balas_cargador) + L" / " + std::to_wstring(arma->balas_reserva);
                    }
                }
            }
        	else
        	{
                info += L"MUNICION: N/A";
            }
            if (hud_texto)
	        	hud_texto->setString(info);
        }


		// 7. Sistema de Spawners
		SistemaSpawn(objetos, m_spawn_pool, clips_zombie, jugador_ref.get());

		// 6. Actualizar objetos del pool (NPCs, etc.)
		auto &pool = objetos.getPool();
		std::erase_if(pool, [](const std::shared_ptr<CE::Objeto> &e) {
			// Remove dead NPCs (targets) and also remove temporary FX whose ITemporal::done is set
			if (e->tieneComponente<ITarget>() && e->getStats()->hp <= 0) return true;
			auto temp = e->getComponente<ITemporal>();
			if (temp && temp->done) return true;
			// Eliminar balas desactivadas (por impacto o distancia máxima)
			auto bala = e->getComponente<IBala>();
			if (bala && !bala->activa) return true;
			return false;
		});

		for (auto &obj: pool) {
			obj->inputFSM();
			obj->onUpdate(dt);
			if (obj->tieneComponente<ITarget>()) {
				auto &tpos = *obj->getComponente<ITarget>()->pos;
				// SistemaNPCLookTarget(*obj, tpos); // Quitamos esto, ahora
				// SistemaZombieSeguir maneja la mirada
				SistemaZombieSeguir(*obj, tpos, objetos.getPool(), dt);
				// No colisionar balas como paredes sólidas contra el jugador
				if (!obj->tieneComponente<IBala>())
					SistemaColAABB(*obj, *jugador_ref, true);
			}
			// Colisión sólida jugador vs objetos (excluir balas)
			if (!obj->tieneComponente<IBala>())
				SistemaColAABB(*jugador_ref, *obj, true);
		}
		/*
		//lab11 prefab - Colisión de balas contra zombies (objetos con ITarget)
		for (auto &bala : jugador_ref->getComponente<IEmisor>()->getPool().getPool())
		{
			for (auto &obj : objetos.getPool())
			{
				// Verificar si el objeto tiene componente ITarget (es un zombie)
				if (obj->tieneComponente<ITarget>())
				{
					if (SistemaColAABBMid(*bala, *obj, true))
					{
						((Entidad &)*bala).setFSM(std::make_shared<BalasBoom>(0.33f));
						bala->getComponente<IMaquinaEstado>()->congelar = true;
						// daño una sola vez
						bala->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};
						if (obj->getStats()->congelar == false)
						{
							float mitigar = obj->getStats()->def / 255.f;
							obj->getStats()->hp -= (mitigar * bala->getStats()->str);
							obj->getStats()->congelar = true;
						}
					}

					if (bala->getComponente<CE::ITimer>()->frame_actual >= bala->getComponente<CE::ITimer>()->frame_maximo)
					{
						obj->getStats()->congelar = false;
					}

					// Aplicar shader si el objeto tiene uno
					if (obj->tieneComponente<CE::IShader>())
					{
						static float acum_tiempo = 0;
						acum_tiempo += dt;
						obj->getComponente<CE::IShader>()->setEscalar("dt", &acum_tiempo);
					}
				}
			}
		}
		// lab9 borra toda entidad que esta en el pool que hp sea <=0
		objetos.borrarPool();
		*/
	}

	void Escena_Sprite::onInputs(const CE::Botones &accion) {
		auto ctrl = jugador_ref->getComponente<CE::IControl>();
		if (!ctrl)
			return;

		bool pressed = (accion.getTipo() == CE::Botones::TipoAccion::OnPress);

		if (accion.getNombre() == "arriba")
			ctrl->arr = pressed;
		else if (accion.getNombre() == "abajo")
			ctrl->abj = pressed;
		else if (accion.getNombre() == "derecha")
			ctrl->der = pressed;
		else if (accion.getNombre() == "izquierda")
			ctrl->izq = pressed;
		else if (accion.getNombre() == "atacar") {
			ctrl->acc = pressed;
			jugador_ref->getComponente<IEmisor>()->crearParticula();
			// Nota: el ataque melee se procesa en onUpdate por el sistema de ataque,
			// no aquí, para que el timing de la hitbox se controle desde la FSM
		}
		else if (accion.getNombre() == "correr")
			ctrl->run = pressed;
		else if (accion.getNombre() == "recargar")
			ctrl->sacc = pressed;	// mapear boton de recarga a control.sacc

		// cambio de armas usando teclas Q y E
		if (pressed)
		{
			auto inv = jugador_ref->getComponente<IInventario>();
			if (inv) {
				if (accion.getNombre() == "siguiente_arma")
				{
					inv->ciclarSiguiente();
					actualizarArmaEquipada();
				}
				else if (accion.getNombre() == "anterior_arma")
				{
					inv->ciclarAnterior();
					actualizarArmaEquipada();
				}
			}
		}


		// Nota: el control del cuerpo se sincroniza al inicio de onUpdate
	}

	void Escena_Sprite::onRender() {
		CE::Render::Get().OnClearColor(sf::Color(0, 0, 0));

		// Tilemap al fondo
		for (auto &al: tiles_layers)
			CE::Render::Get().AddToDraw(al);

		// Pies primero (debajo), luego cuerpo (encima)
		CE::Render::Get().AddToDraw(*jugador_ref);
		CE::Render::Get().AddToDraw(*cuerpo);

		// Objetos del pool
		for (auto &obj: objetos.getPool()) {
			if (obj->tieneComponente<IRangoAggro>()) {
				auto rango = obj->getComponente<IRangoAggro>();
				rango->marcador.setPosition({obj->getTransformada()->posicion.x, obj->getTransformada()->posicion.y});
				CE::Render::Get().AddToDraw(rango->marcador);
			}
			CE::Render::Get().AddToDraw(*obj);

#if DEBUG
			if (obj->tieneComponente<CE::ITexto>()) {
				auto texto = obj->getComponente<CE::ITexto>();
				texto->m_texto.setPosition({obj->getTransformada()->posicion.x - 30.f,
										   obj->getTransformada()->posicion.y - 60.f});
				CE::Render::Get().AddToDraw(texto->m_texto);
			}

			auto arma = cuerpo->getComponente<IArmaMelee>();
			if (arma && arma->atacando) {
				CE::Render::Get().AddToDraw(arma->hitbox_debug);
			}
		}
		// Render debug info for spawn zones
		for (auto &spawn_zone : m_spawn_pool.getPool()) {
			auto respawn_data = spawn_zone->getComponente<CE::IRespawn>();
			if (respawn_data) {
				sf::RectangleShape spawn_rect;
				spawn_rect.setSize({(float)respawn_data->width, (float)respawn_data->height});
				spawn_rect.setOrigin({respawn_data->width / 2.f, respawn_data->height / 2.f});
				spawn_rect.setPosition({spawn_zone->getTransformada()->posicion.x, spawn_zone->getTransformada()->posicion.y});
				spawn_rect.setFillColor(sf::Color(0, 255, 0, 60)); // Translucent green
				spawn_rect.setOutlineThickness(1.f);
				spawn_rect.setOutlineColor(sf::Color::Green);
				CE::Render::Get().AddToDraw(spawn_rect);
			}
		}
#endif

		// Si el ataque está en proceso, dibujar la hitbox (efecto visual)
		if (ataque_procesado) {
			CE::Render::Get().AddToDraw(ataque_hitbox);
		}

		// Configurar y dibujar el shader de niebla (efecto tenebroso)
		if (sf::Shader::isAvailable()) {
			sf::Vector2f playerPos = {jugador_ref->getTransformada()->posicion.x,
									  jugador_ref->getTransformada()->posicion.y};
			fog_shader.setUniform("lampPos", sf::Vector2f(playerPos.x / 4000.f, playerPos.y / 4000.f));
			fog_shader.setUniform("lampRadio", 0.05f); // ~200px
			fog_shader.setUniform("lampSoft", 0.04f); // Suavizado del borde
			fog_shader.setUniform("fogColor", sf::Glsl::Vec4(0.01f, 0.01f, 0.01f, 0.98f)); // Oscuro casi negro
			fog_shader.setUniform("tiempo", fog_tiempo * 0.5f); // Humo denso y lento
			fog_shader.setUniform("ruido_escala", 15.0f);
			fog_shader.setUniform("ruido_min", 0.3f);
			fog_shader.setUniform("ruido_max", 0.7f);
			fog_shader.setUniform("opacidad_offset", 0.2f);

			CE::Render::Get().AddToDraw(fog_rect, &fog_shader);
		}

		/*
		//lab11 prefab
		for (auto &p : jugador_ref->getComponente<IEmisor>()->getPool().getPool())
		{
			CE::Render::Get().AddToDraw(*p);
		}
		CE::Render::Get().AddToDraw(*jugador_ref);
		*/

#if DEBUG
		auto cam = &CE::GestorCamaras::Get().getCamaraActiva();
		auto csv = (CE::CamaraSnapVentana *) cam;
		auto csvpos = csv->getTransformada().posicion;
		sf::RectangleShape debugcam{{csv->m_vdim.x, csv->m_vdim.y}};
		debugcam.setOrigin({csv->m_vdim.x / 2.f, csv->m_vdim.y / 2.f});
		debugcam.setPosition({csvpos.x, csvpos.y});
		debugcam.setOutlineThickness(3.f);
		debugcam.setOutlineColor(sf::Color::Yellow);
		debugcam.setFillColor(sf::Color::Transparent);
		CE::Render::Get().AddToDraw(debugcam);
#endif
		// Renderizar HUD estático (en coordenadas de pantalla)
		auto& tex = CE::Render::Get().GetTextura();
		sf::View vista_original = tex.getView();
		tex.setView(tex.getDefaultView()); // Cambiar a la vista estática

		if (hud_texto)
		{
			hud_texto->setPosition({30.f, 30.f});
			CE::Render::Get().AddToDraw(*hud_texto);
		}

		tex.setView(vista_original); // Restaurar vista del juego original
	}

	void Escena_Sprite::actualizarArmaEquipada()
	{
		auto inv = jugador_ref->getComponente<IInventario>();
		if (!inv) return;
		auto item = inv->getItemActivo();
		if (!item) return;

		// 1. Cambiar textura, dimensiones y escala en el componente ISprite delcuerpo
		auto spr = cuerpo->getComponente<CE::ISprite>();
		if (spr) {
			spr->m_sprite.setTexture(CE::GestorAssets::Get().getTextura(item->key_textura));
			spr->width = item->sprite_w;
			spr->height = item->sprite_h;
			spr->escala = item->sprite_escala;
			spr->m_sprite.setScale({item->sprite_escala, item->sprite_escala});
			spr->m_sprite.setOrigin({item->sprite_w / 2.f, item->sprite_h / 2.f});
		}

		// 2. Cargar clips correspondientes y reasignar la FSM del cuerpo
		if (item->tipo == IItem::Tipo::Cuchillo)
		{
			auto clips = CE::AnimLoader::cargar(item->json_anims);
			auto fsm_cuerpo = std::make_shared<IdleCuerpoKnife>(CE::AnimLoader::getClip(clips, "idle_knife"));
			cuerpo->getComponente<IMaquinaEstado>()->fsm = fsm_cuerpo;
			cuerpo->setFSM(fsm_cuerpo);
		}
		else if (item->tipo == IItem::Tipo::Pistola)
		{
			auto fsm_cuerpo = std::make_shared<IdleCuerpoPistola>(CE::AnimClip{0, 20}, 0.1f);
			cuerpo->getComponente<IMaquinaEstado>()->fsm = fsm_cuerpo;
			cuerpo->setFSM(fsm_cuerpo);
		}
	}

	void Escena_Sprite::cargarSonidos()
	{
		CE::GestorAssets::Get().agregarSonido("slash01", ASSETS "/sfx/machete_slash_flesh_attack_01.wav");
		CE::GestorAssets::Get().agregarSonido("slash02", ASSETS "/sfx/machete_slash_flesh_attack_02.wav");
		CE::GestorAssets::Get().agregarSonido("slash03", ASSETS "/sfx/machete_slash_flesh_attack_03.wav");
		CE::GestorAssets::Get().agregarSonido("slash04", ASSETS "/sfx/machete_slash_flesh_attack_04.wav");
		CE::GestorAssets::Get().agregarSonido("swing01", ASSETS "/sfx/machete_swing_01.wav");
		CE::GestorAssets::Get().agregarSonido("swing02", ASSETS "/sfx/machete_swing_02.wav");
		CE::GestorAssets::Get().agregarSonido("swing03", ASSETS "/sfx/machete_swing_03.wav");
		CE::GestorAssets::Get().agregarSonido("swing04", ASSETS "/sfx/machete_swing_04.wav");
		CE::GestorAssets::Get().agregarSonido("pistol_shot", ASSETS "/sfx/pistol/pistol_shot.mp3");
		CE::GestorAssets::Get().agregarSonido("pistol_reload", ASSETS "/sfx/pistol/pistol_reload.mp3");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_01", ASSETS "/sfx/zombie_attack/attack_01.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_02", ASSETS "/sfx/zombie_attack/attack_02.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_03", ASSETS "/sfx/zombie_attack/attack_03.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_04", ASSETS "/sfx/zombie_attack/attack_04.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_05", ASSETS "/sfx/zombie_attack/attack_05.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_06", ASSETS "/sfx/zombie_attack/attack_06.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_attack_07", ASSETS "/sfx/zombie_attack/attack_07.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_01", ASSETS "/sfx/zombie_hurt/hurt_01.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_02", ASSETS "/sfx/zombie_hurt/hurt_02.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_03", ASSETS "/sfx/zombie_hurt/hurt_03.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_04", ASSETS "/sfx/zombie_hurt/hurt_04.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_05", ASSETS "/sfx/zombie_hurt/hurt_05.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_06", ASSETS "/sfx/zombie_hurt/hurt_06.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_07", ASSETS "/sfx/zombie_hurt/hurt_07.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_08", ASSETS "/sfx/zombie_hurt/hurt_08.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_09", ASSETS "/sfx/zombie_hurt/hurt_09.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_10", ASSETS "/sfx/zombie_hurt/hurt_10.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_11", ASSETS "/sfx/zombie_hurt/hurt_11.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_12", ASSETS "/sfx/zombie_hurt/hurt_12.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_13", ASSETS "/sfx/zombie_hurt/hurt_13.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_14", ASSETS "/sfx/zombie_hurt/hurt_14.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_15", ASSETS "/sfx/zombie_hurt/hurt_15.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_16", ASSETS "/sfx/zombie_hurt/hurt_16.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_17", ASSETS "/sfx/zombie_hurt/hurt_17.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_18", ASSETS "/sfx/zombie_hurt/hurt_18.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_19", ASSETS "/sfx/zombie_hurt/hurt_19.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_20", ASSETS "/sfx/zombie_hurt/hurt_20.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_21", ASSETS "/sfx/zombie_hurt/hurt_21.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_22", ASSETS "/sfx/zombie_hurt/hurt_22.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_23", ASSETS "/sfx/zombie_hurt/hurt_23.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_24", ASSETS "/sfx/zombie_hurt/hurt_24.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_25", ASSETS "/sfx/zombie_hurt/hurt_25.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_26", ASSETS "/sfx/zombie_hurt/hurt_26.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_27", ASSETS "/sfx/zombie_hurt/hurt_27.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_28", ASSETS "/sfx/zombie_hurt/hurt_28.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_29", ASSETS "/sfx/zombie_hurt/hurt_29.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_30", ASSETS "/sfx/zombie_hurt/hurt_30.wav");
		CE::GestorAssets::Get().agregarSonido("zombie_hurt_31", ASSETS "/sfx/zombie_hurt/hurt_31.wav");

	}

} // namespace IVJ