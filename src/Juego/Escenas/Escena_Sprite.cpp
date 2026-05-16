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

#include <Juego/Maquinas/Cuerpo/AtacarCuerpoKnife.hpp>
#include <Juego/Maquinas/Zombie/IdleZombie.hpp>

#include <algorithm>
#include <cmath>
#include <memory>

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

		// Tilemap
		tiles_layers.push_back(TileMap());
		tiles_layers.push_back(TileMap());
		tiles_layers[0].loadTileMap(ASSETS "/mapas/floors.txt");
		// tiles_layers[1].loadTileMap(ASSETS "/mapas/walls.txt");
		tiles_layers[1].loadTileMap(ASSETS "/mapas/walls_colisiones.txt", objetos);
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

		CE::GestorAssets::Get().agregarTextura("zombie", ASSETS "/sprites/zombie/zombie-atlas.png",
											   CE::Vector2D{0.f, 0.f}, CE::Vector2D{2688.f, 2688.f});

		// METADATA DE ANIMACION
		auto clips_pies = CE::AnimLoader::cargar(ASSETS "/sprites/Top_Down_Survivor/feet/indices_animacion.json");
		auto clips_knife = CE::AnimLoader::cargar(ASSETS "/sprites/Top_Down_Survivor/knife/indices_animacion.json");
		auto clips_zombie = CE::AnimLoader::cargar(ASSETS "/sprites/zombie/indices_animacion.json");

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

		// FSM de cuerpo
		auto fsm_cuerpo = std::make_shared<IdleCuerpoKnife>(CE::AnimLoader::getClip(clips_knife, "idle_knife"));

		auto mq_cuerpo = std::make_shared<IMaquinaEstado>();
		mq_cuerpo->fsm = fsm_cuerpo;
		cuerpo->addComponente(mq_cuerpo);
		cuerpo->setFSM(fsm_cuerpo);

		objetos.agregarPool(cuerpo);

		// ZOMBIE
		auto zombie = std::make_shared<Entidad>();
		zombie->getTransformada()->posicion = CE::Vector2D{2200.f, 2200.f};
		zombie->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};
		zombie->getStats()->hp = 100;
		zombie->getStats()->hp_max = 100;
		zombie->addComponente(
				std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("zombie"), 384, 384, .5f));

		auto fsm_zombie = std::make_shared<IdleZombie>(clips_zombie, 0.1f);

		auto mq_zombie = std::make_shared<IMaquinaEstado>();
		mq_zombie->fsm = fsm_zombie;
		zombie->addComponente(mq_zombie);
		zombie->setFSM(fsm_zombie);

		zombie->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{16.f, 16.f}));
		zombie->addComponente(std::make_shared<IRangoAggro>(ZombieConst::RANGO_AGGRO));
		zombie->addComponente(std::make_shared<IEntorno>(&objetos.getPool()));
		zombie->addComponente(std::make_shared<ITarget>(jugador_ref.get()));

		// Text HP debug
		auto texto_hp = std::make_shared<CE::ITexto>(CE::GestorAssets::Get().getFont("default_font"), "HP: 100");
		texto_hp->m_texto.setCharacterSize(20);
		texto_hp->m_texto.setFillColor(sf::Color::Red);
		texto_hp->m_texto.setOutlineColor(sf::Color::Black);
		texto_hp->m_texto.setOutlineThickness(1.f);
		zombie->addComponente(texto_hp);

		// Basic Shader for Player: Outline
		auto zombie_shader = std::make_shared<CE::IShader>("", ASSETS "/shaders/contorno.frag");
		zombie_shader->setVector4("uOutlineColor", &zombie_outline_color);
		zombie_shader->setEscalar("uThickness", &zombie_outline_thickness);
		zombie_shader->setVector2("uTexSize", &zombie_tex_size);
		zombie->addComponente(zombie_shader);

		// agregar a pool
		objetos.agregarPool(zombie);

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

		// LOGICA DE ATAQUE
		auto fsm_cuerpo = cuerpo->getComponente<IMaquinaEstado>()->fsm;
		auto *ataque = dynamic_cast<AtacarCuerpoKnife *>(fsm_cuerpo.get());

		if (ataque)
		{
			if (!ataque_procesado)
			{
				ataque_procesado = true;
				objs_golpeados.clear();
			}

			/*
			auto mp = CE::Render::Get().GetMouseGamePos();	// mp es mouse position: posición del mouse en coordenadas de ventana
			// wPos es la posición del mouse convertida a coordenadas del mundo (ajustada por la cámara)
			sf::Vector2f wPos = CE::Render::Get().GetTextura().mapPixelToCoords(
					sf::Vector2i((int) mp.x, (int) mp.y), CE::GestorCamaras::Get().getCamaraActiva().getView());

			auto &pos = jugador_ref->getTransformada()->posicion;
			float dx = wPos.x - pos.x;	// dx es la diferencia en x entre la posición del mouse y la posición del jugador
			float dy = wPos.y - pos.y;	// dy es la diferencia en y entre la posición del mouse y la posición del jugador
			float dist = std::sqrt(dx * dx + dy * dy);	// dist es la distancia entre el mouse y el jugador, usando Pitágoras
			*/

			float nx = 0;	// nx: componente normalizado en x de la dirección del ataque
			float ny = 0;	// ny: componente normalizado en y de la dirección del ataque
			if (dist > 0) {	// Para evitar división por cero, solo normalizamos si la distancia es mayor que cero
				nx = dx / dist;
				ny = dy / dist;
			}

			ataque_hitbox.setRadius(32.f);
			ataque_hitbox.setOrigin({32.f, 32.f});
			ataque_hitbox.setPosition({pos.x + nx * 32.f, pos.y + ny * 32.f});
			ataque_hitbox.setFillColor(sf::Color(255, 0, 0, 100));

			for (auto &obj: objetos.getPool()) {
				if (obj != jugador_ref && obj != cuerpo && obj->tieneComponente<ITarget>()) {
					if (objs_golpeados.find(obj.get()) == objs_golpeados.end()) {
						float zdx = obj->getTransformada()->posicion.x - ataque_hitbox.getPosition().x;
						float zdy = obj->getTransformada()->posicion.y - ataque_hitbox.getPosition().y;
						if (std::sqrt(zdx * zdx + zdy * zdy) < 48.f) {
							objs_golpeados.insert(obj.get());
							if (obj->getStats()->hp > 0) {
								int new_hp = obj->getStats()->hp - 15;
								obj->getStats()->hp = (new_hp < 0) ? 0 : new_hp;
								if (obj->tieneComponente<CE::ITexto>()) {
									obj->getComponente<CE::ITexto>()->m_texto.setString(
											"HP: " + std::to_string(obj->getStats()->hp));
								}
							}
						}
					}
				}
			}
		} else {
			ataque_procesado = false;
		}

		// 6. Actualizar objetos del pool (NPCs, etc.)
		auto &pool = objetos.getPool();
		std::erase_if(pool, [](const std::shared_ptr<CE::Objeto> &e) {
			return e->tieneComponente<ITarget>() && e->getStats()->hp <= 0;
		});

		for (auto &obj: pool) {
			obj->inputFSM();
			obj->onUpdate(dt);
			if (obj->tieneComponente<ITarget>()) {
				auto &tpos = *obj->getComponente<ITarget>()->pos;
				// SistemaNPCLookTarget(*obj, tpos); // Quitamos esto, ahora
				// SistemaZombieSeguir maneja la mirada
				SistemaZombieSeguir(*obj, tpos, objetos.getPool(), dt);
				SistemaColAABB(*obj, *jugador_ref, true);
			}

			// se agrega todas las cajas de colisiiones generadas del tilemap al sistema
			// de colisiones
			SistemaColAABB(*jugador_ref, *obj, true);
		}
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
		}
		else if (accion.getNombre() == "correr")
			ctrl->run = pressed;
		// Nota: el control del cuerpo se sincroniza al inicio de onUpdate
	}

	void Escena_Sprite::onRender() {
		CE::Render::Get().OnClearColor(sf::Color(0, 0, 0));

		// Tilemap al fondo
		for (auto &al: tiles_layers)
			CE::Render::Get().AddToDraw(al);

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
#endif
		}

		// Pies primero (debajo), luego cuerpo (encima)
		CE::Render::Get().AddToDraw(*jugador_ref);
		CE::Render::Get().AddToDraw(*cuerpo);

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
	}
} // namespace IVJ
