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

#include "Juego/Maquinas/Zombie/AtacarZombie.hpp"
#include "Motor/Primitivos/GestorEscenas.hpp"

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
		registrarBotones(sf::Keyboard::Scancode::Escape, "pausa");
		registrarBotones(sf::Keyboard::Scancode::Enter,  "confirmar");

		// Tilemap
		tiles_layers.push_back(TileMap());
		tiles_layers.push_back(TileMap());
		tiles_layers[0].loadTileMap(ASSETS "/mapas/floors.txt");
		// tiles_layers[1].loadTileMap(ASSETS "/mapas/walls.txt");
		// pass both the objects pool and the spawn pool to the tilemap loader
		tiles_layers[1].loadTileMap(ASSETS "/mapas/walls_colisiones.txt", objetos, m_spawn_pool);
		tiles_layers[0].atlas_texture.setSmooth(true);
		tiles_layers[1].atlas_texture.setSmooth(true);

		cargarSonidos();

		CE::GestorAssets::Get().agregarMusica("musica_fondo", ASSETS "/music/levelmusic.mp3");
		// reproducir musica de fondo (loop)
		CE::GestorAssets::Get().getMusica("musica_fondo").play();
		CE::GestorAssets::Get().getMusica("musica_fondo").setLooping(true);
		CE::GestorAssets::Get().getMusica("musica_fondo").setVolume(50.f);

		CE::GestorAssets::Get().agregarMusica("musica_pausa", ASSETS "/music/pausemusic.mp3");
		CE::GestorAssets::Get().getMusica("musica_pausa").setLooping(true);
		CE::GestorAssets::Get().getMusica("musica_pausa").setVolume(50.f);

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
		trans->velocidad = CE::Vector2D{300.f, 300.f};
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

		jugador_ref->getStats()->hp_max = 200;
		jugador_ref->getStats()->hp = 200;
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

		mensaje_pantalla = std::make_unique<sf::Text>(CE::GestorAssets::Get().getFont("default_font"), "", 50);
		mensaje_pantalla->setFillColor(sf::Color::Yellow);
		mensaje_pantalla->setOutlineColor(sf::Color::Black);
		mensaje_pantalla->setOutlineThickness(2.f);

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

	void Escena_Sprite::onFinal() {
		CE::GestorCamaras::Get().setCamaraActiva(0);
		CE::GestorAssets::Get().getMusica("musica_fondo").stop();
	}

	void Escena_Sprite::onUpdate(float dt) {

		// Botones:
		constexpr unsigned int BTN_A      = 0;
		constexpr unsigned int BTN_B      = 1;
		constexpr unsigned int BTN_X      = 3;  // recargar  ← era 2, ahora es 3
		constexpr unsigned int BTN_Y      = 4;              // ← era 3, ahora es 4
		constexpr unsigned int BTN_LB     = 6;  // arma anterior  ← era 4, ahora es 6
		constexpr unsigned int BTN_RB     = 7;  // arma siguiente ← era 5, ahora es 7
		constexpr unsigned int BTN_BACK   = 10; // select     ← era 6, ahora es 10
		constexpr unsigned int BTN_START  = 11; // pausa      ← era 7, ahora es 11

		// Ejes del stick derecho:
		constexpr auto AXIS_RSX = sf::Joystick::Axis::Z; // ← era Axis::U
		constexpr auto AXIS_RSY = sf::Joystick::Axis::R; // ← era Axis::V
		constexpr auto AXIS_LSX       = sf::Joystick::Axis::X;
		constexpr auto AXIS_LSY       = sf::Joystick::Axis::Y;

		constexpr auto AXIS_RT        = sf::Joystick::Axis::Z; // ← cambiar si diagnóstico indica otro
		constexpr float DEADZONE      = 25.f;
		constexpr float TRIGGER_UMBRAL = 50.f;

		// ── Diagnóstico de gamepad (pon en true para identificar ejes/botones) ──
		// Con el gamepad conectado, mueve cada palanca/gatillo y mira la consola.
		// Cuando tengas el mapeo correcto, vuelve a poner en false.
		constexpr bool GAMEPAD_DEBUG = true;
		if constexpr (GAMEPAD_DEBUG) {
			if (sf::Joystick::isConnected(0)) {
				static int frame_debug = 0;
				if (++frame_debug % 30 == 0) { // imprimir cada 30 frames
					std::cout << "=== GAMEPAD DEBUG ===\n";
					std::cout << "  Botones (" << sf::Joystick::getButtonCount(0) << " total): ";
					for (unsigned int b = 0; b < sf::Joystick::getButtonCount(0); ++b)
						if (sf::Joystick::isButtonPressed(0, b))
							std::cout << "B" << b << " ";
					std::cout << "\n";
					const sf::Joystick::Axis ejes[] = {
						sf::Joystick::Axis::X, sf::Joystick::Axis::Y,
						sf::Joystick::Axis::Z, sf::Joystick::Axis::R,
						sf::Joystick::Axis::U, sf::Joystick::Axis::V
					};
					const char* nombres[] = {"X","Y","Z","R","U","V", "PovX", "PovY"};
					for (int i = 0; i < 6; ++i) {
						float v = sf::Joystick::getAxisPosition(0, ejes[i]);
						if (std::abs(v) > 5.f)
							std::cout << "  Axis::" << nombres[i] << " = " << v << "\n";
					}
				}
			}
		}

		// ── Start/pausa: DEBE ir antes del early-return de en_pausa ─────────────
		// Si está aquí, funciona tanto para abrir como para cerrar la pausa.
		if (sf::Joystick::isConnected(0)) {
			static bool start_prev = false;
			bool start_now = sf::Joystick::isButtonPressed(0, BTN_START);
			if (start_now && !start_prev && !jugador_muerto) {
				en_pausa     = !en_pausa;
				opcion_pausa = 0;
				if (en_pausa) {
					CE::GestorAssets::Get().getMusica("musica_fondo").pause();
					CE::GestorAssets::Get().getMusica("musica_pausa").play();
				} else {
					CE::GestorAssets::Get().getMusica("musica_pausa").stop();
					CE::GestorAssets::Get().getMusica("musica_fondo").play();
				}
			}
			start_prev = start_now;
		}

		if (en_pausa) return;
		if (jugador_muerto) return;

		if (jugador_ref->getStats()->hp <= 0) {
			jugador_muerto = true;
			return;
		}

		fog_tiempo += dt;

		// 1. Sincronizar control del cuerpo con el de los pies
		//	(ambas entidades reaccionan a los mismos inputs)
		auto ctrl_pies = jugador_ref->getComponente<CE::IControl>();
		auto ctrl_cuerpo = cuerpo->getComponente<CE::IControl>();

		// Polling del teclado SIEMPRE cada frame (evita teclas pegadas).
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

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				ctrl_pies->acc = true;

			// ── Gamepad Xbox ──────────────────────────────────────────────────────
			if (sf::Joystick::isConnected(0)) {
				// Palanca izquierda: movimiento
				float lx = sf::Joystick::getAxisPosition(0, AXIS_LSX);
				float ly = sf::Joystick::getAxisPosition(0, AXIS_LSY);
				ctrl_pies->der = ctrl_pies->der || (lx >  DEADZONE);
				ctrl_pies->izq = ctrl_pies->izq || (lx < -DEADZONE);
				ctrl_pies->arr = ctrl_pies->arr || (ly < -DEADZONE);
				ctrl_pies->abj = ctrl_pies->abj || (ly >  DEADZONE);

				// RT: atacar
				float rt = sf::Joystick::getAxisPosition(0, AXIS_RT);
				ctrl_pies->acc = ctrl_pies->acc || (rt > TRIGGER_UMBRAL);

				// X: recargar
				ctrl_pies->sacc = sf::Joystick::isButtonPressed(0, BTN_X);

				// LB / RB: ciclar armas (flanco de bajada)
				static bool lb_prev = false, rb_prev = false;
				bool lb_now = sf::Joystick::isButtonPressed(0, BTN_LB);
				bool rb_now = sf::Joystick::isButtonPressed(0, BTN_RB);
				if (auto inv = jugador_ref->getComponente<IInventario>()) {
					if (lb_now && !lb_prev) { inv->ciclarAnterior(); actualizarArmaEquipada(); }
					if (rb_now && !rb_prev) { inv->ciclarSiguiente(); actualizarArmaEquipada(); }
				}
				lb_prev = lb_now;
				rb_prev = rb_now;
			}
		}

		if (ctrl_pies && ctrl_cuerpo)
			*ctrl_cuerpo = *ctrl_pies;

		// 2a. Dirección de apuntado: palanca derecha si supera la deadzone, si no mouse
		auto &pos = jugador_ref->getTransformada()->posicion;
		float aim_nx = 0.f, aim_ny = 0.f;
		bool uso_stick_derecho = false;

		if (sf::Joystick::isConnected(0)) {
			float rx   = sf::Joystick::getAxisPosition(0, AXIS_RSX);
			float ry   = sf::Joystick::getAxisPosition(0, AXIS_RSY);
			float rmag = std::sqrt(rx * rx + ry * ry);
			if (rmag > DEADZONE) {
				aim_nx = rx / rmag;
				aim_ny = ry / rmag;
				uso_stick_derecho = true;
			}
		}

		if (!uso_stick_derecho) {
			auto mp = CE::Render::Get().GetMouseGamePos();
			sf::Vector2f wPos = CE::Render::Get().GetTextura().mapPixelToCoords(
					sf::Vector2i((int)mp.x, (int)mp.y),
					CE::GestorCamaras::Get().getCamaraActiva().getView());
			float dx   = wPos.x - pos.x;
			float dy   = wPos.y - pos.y;
			float dist = std::sqrt(dx * dx + dy * dy);
			if (dist > 0.f) { aim_nx = dx / dist; aim_ny = dy / dist; }
		}

		jugador_ref->setMouseDirection(aim_nx, aim_ny);
		cuerpo->setMouseDirection(aim_nx, aim_ny);

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

		// Flanco de RT/Mouse para IEmisor (el teclado lo maneja onInputs con Space)
		{
			static bool acc_prev = false;
			bool acc_now = ctrl_pies && ctrl_pies->acc;
			if (acc_now && !acc_prev) {
				if (auto emisor = jugador_ref->getComponente<IEmisor>())
					emisor->crearParticula();
			}
			acc_prev = acc_now;
		}

        if (item_activo)
        {
            if (item_activo->tipo == IItem::Tipo::Cuchillo)
            {
                // Ejecutar sistema de ataque cuerpo a cuerpo
                SistemaAtaqueMelee(*cuerpo, objetos, puntaje_total, zombies_matados_ronda);
            }
            else if (item_activo->tipo == IItem::Tipo::Pistola)
            {
				if (auto arma = cuerpo->getComponente<IPistola>()) {
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
        SistemaColBalas(objetos, puntaje_total, zombies_matados_ronda);

        // Actualizar HUD
        if (item_activo)
        {
            std::wstring info = L"VIDA: " + std::to_wstring((int)jugador_ref->getStats()->hp) + L"\n" +
                                L"ARMA: " + std::wstring(item_activo->nombre.begin(), item_activo->nombre.end()) + L"\n";
            if (item_activo->tipo == IItem::Tipo::Pistola)
            {
				if (auto arma = cuerpo->getComponente<IPistola>())
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
			
            info += L"\nPUNTAJE: " + std::to_wstring(puntaje_total) +
                    L"\nRONDA: " + std::to_wstring(ronda_actual);

            if (hud_texto)
	        	hud_texto->setString(info);
        }


		// Sistema de Rondas
		if (zombies_matados_ronda >= zombies_ronda_total) {
			if (tiempo_espera_ronda == 0.f) {
				auto float_txt = std::make_shared<Entidad>();
				float_txt->setPosicion(jugador_ref->getTransformada()->posicion.x, jugador_ref->getTransformada()->posicion.y - 60.f);
				float_txt->addComponente(std::make_shared<IFloatingText>(3.0f));
				float_txt->addComponente(std::make_shared<ITemporal>());
				auto text_comp = std::make_shared<CE::ITexto>(CE::GestorAssets::Get().getFont("default_font"),
					"Ronda " + std::to_string(ronda_actual) + " terminada");
				text_comp->m_texto.setCharacterSize(24);
				text_comp->m_texto.setFillColor(sf::Color::Yellow);
				text_comp->m_texto.setOutlineColor(sf::Color::Black);
				text_comp->m_texto.setOutlineThickness(2.f);
				float_txt->addComponente(text_comp);
				objetos.agregarPool(float_txt);
			}
			tiempo_espera_ronda += dt;
			if (tiempo_espera_ronda >= 3.0f) {
				ronda_actual++;
				zombies_ronda_total = static_cast<int>(std::round(zombies_ronda_total * 1.2f));
				zombies_por_generar_ronda = zombies_ronda_total;
				zombies_matados_ronda = 0;
				tiempo_espera_ronda = 0.f;
			}
		} else {
			SistemaSpawn(objetos, m_spawn_pool, clips_zombie, jugador_ref.get(), zombies_por_generar_ronda);
		}

		// 6. Actualizar objetos del pool (NPCs, etc.)
		auto &pool = objetos.getPool();
		pool.erase(std::remove_if(pool.begin(), pool.end(), [](const std::shared_ptr<CE::Objeto> &e)
		{
			// Remove dead NPCs (targets) and also remove temporary FX whose ITemporal::done is set
			if (e->tieneComponente<ITarget>() && e->getStats()->hp <= 0) {
				if (e->tieneComponente<IZombieSpawnerRef>()) {
					if (auto sp_ref = e->getComponente<IZombieSpawnerRef>()) {
						if (auto sp = sp_ref->spawner_ptr) {
							if (auto resp = sp->getComponente<CE::IRespawn>()) {
								resp->timer_maximo = 120;
								resp->timer_actual = 0;
							}
						}
					}
				}
				return true;
			}
			auto temp = e->getComponente<ITemporal>();
			if (temp && temp->done) return true;
			// Eliminar balas desactivadas (por impacto o distancia máxima)
			auto bala = e->getComponente<IBala>();
			if (bala && !bala->activa) return true;
			return false;
		}), pool.end());

		int salud_count = 0;
		int municion_count = 0;

		// Contar items actuales para los limites
		for (auto &obj: pool) {
			if (auto drop = obj->getComponente<IDropItem>()) {
				if (drop->tipo == IDropItem::Tipo::Salud) salud_count++;
				if (drop->tipo == IDropItem::Tipo::Municion) municion_count++;
			}
		}

		SistemaPaths(pool);

		for (auto &obj: pool) {
			obj->inputFSM();
			obj->onUpdate(dt);

			// Logica de Spawners (IDelayedSpawn)
			if (auto delSpawn = obj->getComponente<IDelayedSpawn>()) {
				delSpawn->delay_timer -= dt;
				if (delSpawn->delay_timer <= 0.f) {
					if (!obj->tieneComponente<ITemporal>()) obj->addComponente(std::make_shared<ITemporal>());
					obj->getComponente<ITemporal>()->done = true; // remover el spawner invisible
					bool can_spawn = false;
					if (delSpawn->tipo == IDropItem::Tipo::Salud && salud_count < 2) can_spawn = true;
					if (delSpawn->tipo == IDropItem::Tipo::Municion && municion_count < 3) can_spawn = true;

					if (can_spawn) {
						auto item = std::make_shared<Entidad>();
						item->setPosicion(obj->getTransformada()->posicion.x, obj->getTransformada()->posicion.y);
						item->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f, 32.f}));
						item->addComponente(std::make_shared<IDropItem>(delSpawn->tipo));
						
						// Marcador visual
						auto txt = std::make_shared<CE::ITexto>(CE::GestorAssets::Get().getFont("default_font"), delSpawn->tipo == IDropItem::Tipo::Salud ? "[ + HP + ]" : "[ + AMMO + ]");
						txt->m_texto.setFillColor(delSpawn->tipo == IDropItem::Tipo::Salud ? sf::Color::Green : sf::Color::Yellow);
						txt->m_texto.setCharacterSize(16);
						txt->m_texto.setOutlineColor(sf::Color::Black);
						txt->m_texto.setOutlineThickness(1.f);
						item->addComponente(txt);

						objetos.agregarPool(item);
						if (delSpawn->tipo == IDropItem::Tipo::Salud) salud_count++;
						else municion_count++;
					}
				}
			}

			// Logica de recoleccion (IDropItem)
			if (auto drop = obj->getComponente<IDropItem>()) {
				if (SistemaColAABBMid(*jugador_ref, *obj, false)) {
					if (!obj->tieneComponente<ITemporal>()) obj->addComponente(std::make_shared<ITemporal>());
					obj->getComponente<ITemporal>()->done = true; // remover item

					auto float_txt = std::make_shared<Entidad>();
					float_txt->setPosicion(obj->getTransformada()->posicion.x, obj->getTransformada()->posicion.y - 40.f);
					float_txt->addComponente(std::make_shared<IFloatingText>(2.0f));
					float_txt->addComponente(std::make_shared<ITemporal>());
					auto text_comp = std::make_shared<CE::ITexto>(CE::GestorAssets::Get().getFont("default_font"), "");
					text_comp->m_texto.setCharacterSize(24);
					text_comp->m_texto.setOutlineColor(sf::Color::Black);
					text_comp->m_texto.setOutlineThickness(2.f);

					if (drop->tipo == IDropItem::Tipo::Salud) {
						auto stats = jugador_ref->getStats();
						stats->hp = std::min(stats->hp_max, (std::uint8_t)(stats->hp + 50));
						text_comp->m_texto.setFillColor(sf::Color::Green);
						text_comp->m_texto.setString("+50 HP");
					} else {
						if (auto arma = cuerpo->getComponente<IPistola>()) {
							arma->balas_reserva = std::min(80, arma->balas_reserva + 16); // 20% de 80, capeado en 80
						}
						text_comp->m_texto.setFillColor(sf::Color::Yellow);
						text_comp->m_texto.setString("+16 Municion");
					}
					float_txt->addComponente(text_comp);
					objetos.agregarPool(float_txt);
				}
			}

			// Lógica de texto flotante (IFloatingText)
			if (auto floatTxt = obj->getComponente<IFloatingText>()) {
				floatTxt->timer -= dt;
				obj->getTransformada()->posicion.y -= 30.f * dt;
				if (auto text_comp = obj->getComponente<CE::ITexto>()) {
					sf::Color c = text_comp->m_texto.getFillColor();
					c.a = (std::uint8_t)(255.f * std::max(0.f, floatTxt->timer / floatTxt->max_timer));
					text_comp->m_texto.setFillColor(c);
				}
				if (floatTxt->timer <= 0.f) {
					if (auto temp = obj->getComponente<ITemporal>()) temp->done = true;
				}
			}

			if (obj->tieneComponente<ITarget>()) {
				auto &tpos = *obj->getComponente<ITarget>()->pos;
				SistemaZombieSeguir(*obj, tpos, pool, dt);

				if (obj->tieneComponente<IMaquinaEstado>())
				{
					auto mq = obj->getComponente<IMaquinaEstado>();
					if (mq->fsm)
					{
						auto* atk = dynamic_cast<AtacarZombie*>(mq->fsm.get());
						if (atk && atk->shouldDealDamage())
						{
							auto target_stats = jugador_ref->getStats();
							if (target_stats->hp > 0)
							{
								target_stats->hp = (target_stats->hp > 25)
												 ? target_stats->hp - 25 : 0;
								target_stats->timer_damage_shader = 1.0f;
							}
							atk->markDamageDealt();
						}
					}
				}

				// No colisionar balas como paredes sólidas contra el jugador
				if (!obj->tieneComponente<IBala>())
					SistemaColAABB(*obj, *jugador_ref, true);

				// Colisión sólida de zombie contra paredes físicas (evita que las atraviesen)
				for (auto &other : pool) {
					if (other.get() == obj.get()) continue;

					bool es_pared = other->tieneComponente<CE::IBoundingBox>() &&
									!other->tieneComponente<ITarget>() &&
									!other->tieneComponente<IBala>() &&
									!other->tieneComponente<IDropItem>() &&
									!other->tieneComponente<IFloatingText>() &&
									!other->tieneComponente<IDelayedSpawn>();
					if (es_pared) {
						SistemaColAABBMid(*obj, *other, true);
					}
				}
			}

			// Colisión sólida jugador vs objetos (excluir balas, drops y textos)
			if (!obj->tieneComponente<IBala>() && !obj->tieneComponente<IDropItem>() && !obj->tieneComponente<IFloatingText>() && !obj->tieneComponente<IDelayedSpawn>())
				SistemaColAABB(*jugador_ref, *obj, true);
		}

		// Reducir temporizador de daño visual
		auto stats = jugador_ref->getStats();
		if (stats->timer_damage_shader > 0.f) {
			stats->timer_damage_shader -= dt;
		}
	}

	void Escena_Sprite::onInputs(const CE::Botones& accion)
	{
		bool pressed = (accion.getTipo() == CE::Botones::TipoAccion::OnPress);

		if (accion.getNombre() == "pausa" && pressed)
		{
			if (!jugador_muerto) // no abrir pausa en game over
			{
				en_pausa     = !en_pausa;
				opcion_pausa = 0;

				if (en_pausa)
				{
					CE::GestorAssets::Get().getMusica("musica_fondo").pause();
					CE::GestorAssets::Get().getMusica("musica_pausa").play();
				}
				else
				{
					CE::GestorAssets::Get().getMusica("musica_pausa").stop();
					CE::GestorAssets::Get().getMusica("musica_fondo").play();
				}
			}
			return;
		}

		if (accion.getNombre() == "confirmar" && pressed)
		{
			// Game Over → reintentar
			if (jugador_muerto)
			{
				resetJuego();
				return;
			}

			// Pausa → navegar menú
			if (en_pausa)
			{
				CE::GestorAssets::Get().getMusica("musica_pausa").stop();
				if (opcion_pausa == 0)          // Continuar
				{
					CE::GestorAssets::Get().getMusica("musica_fondo").play();
					en_pausa = false;
				}
				else                            // Salir a menú
				{
					en_pausa = false;
					CE::GestorAssets::Get().getMusica("musica_fondo").stop();
					CE::GestorEscenas::Get().cambiarEscena("Menu");
				}
				return;
			}
		}

		if (en_pausa && pressed)
		{
			if (accion.getNombre() == "arriba")
				opcion_pausa = 0;
			if (accion.getNombre() == "abajo")
				opcion_pausa = 1;
			return;
		}

		if (en_pausa || jugador_muerto) return;

		auto ctrl = jugador_ref->getComponente<CE::IControl>();
		if (!ctrl) return;

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
		}
		else if (accion.getNombre() == "correr")
			ctrl->run = pressed;
		else if (accion.getNombre() == "recargar")
			ctrl->sacc = pressed;

		if (pressed)
		{
			if (auto inv = jugador_ref->getComponente<IInventario>()) {
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
			if (obj->tieneComponente<CE::ITexto>()) {
				auto texto = obj->getComponente<CE::ITexto>();
				texto->m_texto.setPosition({obj->getTransformada()->posicion.x - 30.f,
										   obj->getTransformada()->posicion.y - 60.f});
				CE::Render::Get().AddToDraw(texto->m_texto);
			}
			CE::Render::Get().AddToDraw(*obj);
		}

#if DEBUG
		for (auto &obj: objetos.getPool()) {
			if (obj->tieneComponente<IRangoAggro>()) {
				auto rango = obj->getComponente<IRangoAggro>();
				rango->marcador.setPosition({obj->getTransformada()->posicion.x, obj->getTransformada()->posicion.y});
				CE::Render::Get().AddToDraw(rango->marcador);
			}
			// Hitbox de melee de zombies atacando
			if (obj->tieneComponente<IArmaMelee>()) {
				auto arma_obj = obj->getComponente<IArmaMelee>();
				if (arma_obj && arma_obj->atacando) {
					CE::Render::Get().AddToDraw(arma_obj->hitbox_debug);
				}
			}
		}
		// Hitbox del cuerpo del jugador
		{
			auto arma = cuerpo->getComponente<IArmaMelee>();
			if (arma && arma->atacando)
				CE::Render::Get().AddToDraw(arma->hitbox_debug);
		}
		// Render debug info for spawn zones
		for (auto &spawn_zone : m_spawn_pool.getPool()) {
			if (auto respawn_data = spawn_zone->getComponente<CE::IRespawn>()) {
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
			
			auto stats = jugador_ref->getStats();
			if (stats->timer_damage_shader > 0.f) {
				float intensidad = std::min(1.0f, stats->timer_damage_shader);
				fog_shader.setUniform("fogColor", sf::Glsl::Vec4(0.01f + 0.6f * intensidad, 0.01f, 0.01f, 0.98f)); // Rojizo
			} else {
				fog_shader.setUniform("fogColor", sf::Glsl::Vec4(0.01f, 0.01f, 0.01f, 0.98f)); // Oscuro casi negro
			}

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

		if (jugador_muerto)
		{
			mensaje_pantalla->setString(
				"GAME OVER\n"
				"Ronda alcanzada: " + std::to_string(ronda_actual) + "\n"
				"Puntaje total:   " + std::to_string(puntaje_total) + "\n\n"
				"Reintentar  (presiona Enter)");
			mensaje_pantalla->setCharacterSize(48);
			mensaje_pantalla->setFillColor(sf::Color::Yellow);
			// Centrar aproximado en 1920×1080
			mensaje_pantalla->setPosition({1920.f / 2.f - 380.f, 1080.f / 2.f - 140.f});
			CE::Render::Get().AddToDraw(*mensaje_pantalla);
		}
		else if (zombies_matados_ronda >= zombies_ronda_total)
		{
			// Mensaje de ronda completada (ya existía, lo dejamos)
			mensaje_pantalla->setPosition({1920.f / 2.f - 300.f, 1080.f / 2.f - 100.f});
			CE::Render::Get().AddToDraw(*mensaje_pantalla);
		}

		if (en_pausa)
		{
			// Fondo semitransparente
			sf::RectangleShape fondo_pausa{{600.f, 300.f}};
			fondo_pausa.setFillColor({0, 0, 0, 180});
			fondo_pausa.setOutlineColor(sf::Color::White);
			fondo_pausa.setOutlineThickness(2.f);
			fondo_pausa.setPosition({1920.f / 2.f - 300.f, 1080.f / 2.f - 150.f});
			CE::Render::Get().AddToDraw(fondo_pausa);

			// Título
			mensaje_pantalla->setCharacterSize(56);
			mensaje_pantalla->setFillColor(sf::Color::White);
			mensaje_pantalla->setString("PAUSA");
			mensaje_pantalla->setPosition({1920.f / 2.f - 80.f, 1080.f / 2.f - 130.f});
			CE::Render::Get().AddToDraw(*mensaje_pantalla);

			// Opciones
			const sf::Color color_sel  = sf::Color::Yellow;
			const sf::Color color_norm = sf::Color(180, 180, 180);

			mensaje_pantalla->setCharacterSize(38);

			mensaje_pantalla->setString(opcion_pausa == 0 ? "> Continuar" : "  Continuar");
			mensaje_pantalla->setFillColor(opcion_pausa == 0 ? color_sel : color_norm);
			mensaje_pantalla->setPosition({1920.f / 2.f - 200.f, 1080.f / 2.f - 30.f});
			CE::Render::Get().AddToDraw(*mensaje_pantalla);

			mensaje_pantalla->setString(opcion_pausa == 1 ? "> Salir (Menu)" : "  Salir (Menu)");
			mensaje_pantalla->setFillColor(opcion_pausa == 1 ? color_sel : color_norm);
			mensaje_pantalla->setPosition({1920.f / 2.f - 200.f, 1080.f / 2.f + 30.f});
			CE::Render::Get().AddToDraw(*mensaje_pantalla);
		}

		tex.setView(vista_original); // Restaurar vista del juego original
	}

	void Escena_Sprite::actualizarArmaEquipada() const {
		auto inv = jugador_ref->getComponente<IInventario>();
		if (!inv) return;
		auto item = inv->getItemActivo();
		if (!item) return;

		// 1. Cambiar textura, dimensiones y escala en el componente ISprite delcuerpo
		if (auto spr = cuerpo->getComponente<CE::ISprite>()) {
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

	void Escena_Sprite::cargarSonidos() {
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

	void Escena_Sprite::resetJuego()
	{
		jugador_ref->getStats()->hp     = 200;
		jugador_ref->getStats()->hp_max = 200;

		jugador_ref->setPosicion(1750.f, 1750.f);
		cuerpo->getTransformada()->posicion = jugador_ref->getTransformada()->posicion;

		if (auto pistola = cuerpo->getComponente<IVJ::IPistola>())
		{
			pistola->balas_cargador     = pistola->balas_max_cargador; // 8
			pistola->balas_reserva      = 32;
			pistola->recargando         = false;
			pistola->disparando         = false;
			pistola->timer_disparo      = 0.f;
			pistola->timer_recarga      = 0.f;
			pistola->gatillo_liberado   = true;
		}

		ronda_actual              = 1;
		zombies_ronda_total       = 4;
		zombies_por_generar_ronda = 4;
		zombies_matados_ronda     = 0;
		puntaje_total             = 0;
		tiempo_espera_ronda       = 0.f;

		jugador_muerto = false;
		en_pausa       = false;
		opcion_pausa   = 0;
		fog_tiempo     = 0.f;

		auto& pool = objetos.getPool();
		pool.erase(
			std::remove_if(pool.begin(), pool.end(),
				[](const std::shared_ptr<CE::Objeto>& e) {
					// Eliminar todo menos paredes estáticas
					bool es_pared = e->tieneComponente<CE::IBoundingBox>()
					             && !e->tieneComponente<IVJ::ITarget>()
					             && !e->tieneComponente<IVJ::IBala>()
					             && !e->tieneComponente<IVJ::IDropItem>()
					             && !e->tieneComponente<IVJ::IFloatingText>()
					             && !e->tieneComponente<IVJ::IDelayedSpawn>()
					             && !e->tieneComponente<CE::ISprite>(); // paredes no tienen sprite
					return !es_pared;
				}),
			pool.end());

		for (auto& spawn : m_spawn_pool.getPool())
		{
			if (auto resp = spawn->getComponente<CE::IRespawn>())
			{
				resp->timer_maximo = 120;
				resp->timer_actual = 0;
			}
		}

		// ── FSM del cuerpo de vuelta a idle ───────────────────────────────────
		actualizarArmaEquipada();
	}
} // namespace IVJ