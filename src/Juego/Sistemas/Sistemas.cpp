// ──────────────────────────────────────────────────────────────────────────
// src/Juego/Sistemas/Sistemas.cpp — full file with all systems implemented
// ──────────────────────────────────────────────────────────────────────────
#include "Sistemas.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../objetos/Entidad.hpp"
#include <cmath>
#include <cstdlib>
#include "../../Motor/Utils/Lerp.hpp"
#include "../../Motor/Render/Render.hpp"
#include <memory>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Maquinas/FSMZombie.hpp>
#include "../../Motor/Camaras/CamarasGestor.hpp"
#include <SFML/Window/Mouse.hpp>

#include <Juego/Maquinas/Zombie/IdleZombie.hpp>
#include <Juego/Maquinas/FX/BloodSplash.hpp>

namespace IVJ
{
	// ─────────────────────────────────────────────────────────────────────────────
	//  MOVEMENT & CONTROL
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaControl(CE::Objeto& ente, float dt)
	{
		auto p = ente.getTransformada();
		auto c = ente.getComponente<CE::IControl>();
		(void)dt;
		if (c->arr) p->velocidad.y = -200;
		if (c->abj) p->velocidad.y =  200;
		if (c->der) p->velocidad.x =  200;
		if (c->izq) p->velocidad.x = -200;
		if (c->run) p->velocidad.escala(2.f);
	}

	void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt)
	{
		for (auto& ente : entes)
		{
			auto trans = ente->getTransformada();
			trans->posicion.suma(trans->velocidad.escala(dt));
		}
	}

	void SistemaMover(const std::shared_ptr<CE::Objeto>& objeto, float dt)
	{
		auto trans   = objeto->getTransformada();
		auto control = objeto->getComponente<CE::IControl>();

		if (!control || !control->isActivo()) return;

		CE::Vector2D dir{0.f, 0.f};
		if (control->arr) dir.y -= 1.f;
		if (control->abj) dir.y += 1.f;
		if (control->der) dir.x += 1.f;
		if (control->izq) dir.x -= 1.f;

		CE::Vector2D vel{0.f, 0.f};
		if (dir.longitud() > 0.f)
		{
			dir.normalizacion();
			vel.x = dir.x * trans->velocidad.x;
			vel.y = dir.y * trans->velocidad.y;
			if (control->run) vel.escala(1.5f);
		}

		// Rotation toward mouse
		if (objeto->getComponente<ITriangulo>() || objeto->getComponente<CE::ISprite>())
		{
			CE::Vector2D gamePos = CE::Render::Get().GetMouseGamePos();
			sf::Vector2f worldPos = CE::Render::Get().GetTextura().mapPixelToCoords(
				sf::Vector2i((int)gamePos.x, (int)gamePos.y),
				CE::GestorCamaras::Get().getCamaraActiva().getView());

			CE::Vector2D dir_mouse = CE::Vector2D{worldPos.x, worldPos.y} - trans->posicion;

			auto trian = objeto->getComponente<ITriangulo>();
			if (trian) trian->angulo = std::atan2(dir_mouse.x, -dir_mouse.y);
			trans->angulo = std::atan2(dir_mouse.x, -dir_mouse.y);
		}

		trans->posicion.suma(vel.escala(dt));
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  COLLISION
	// ─────────────────────────────────────────────────────────────────────────────

	bool SistemaColAABB(CE::Objeto& A, CE::Objeto& B, bool resolucion)
	{
		if (!A.tieneComponente<CE::IBoundingBox>() || !B.tieneComponente<CE::IBoundingBox>())
			return false;

		auto bA = A.getComponente<CE::IBoundingBox>()->tam;
		auto mA = A.getComponente<CE::IBoundingBox>()->mitad;
		auto *pa = &A.getTransformada()->posicion;
		auto prevA = A.getTransformada()->pos_prev;

		auto bB = B.getComponente<CE::IBoundingBox>()->tam;
		auto mB = B.getComponente<CE::IBoundingBox>()->mitad;
		auto pb = B.getTransformada()->posicion;

		bool H = pa->y - mA.y < pb.y + bB.y - mB.y && pb.y - mB.y < pa->y + bA.y - mA.y;
		bool V = pa->x - mA.x < pb.x + bB.x - mB.x && pb.x - mB.x < pa->x + bA.x - mA.x;
		bool colision = H && V;

		if (colision && resolucion) *pa = prevA;
		return colision;
	}

	bool SistemaColAABBMid(CE::Objeto& A, CE::Objeto& B, bool resolucion)
	{
		if (!A.tieneComponente<CE::IBoundingBox>() || !B.tieneComponente<CE::IBoundingBox>())
			return false;

		auto midA = A.getComponente<CE::IBoundingBox>()->mitad;
		auto *pa = &A.getTransformada()->posicion;
		auto prevA = A.getTransformada()->pos_prev;
		auto midB = B.getComponente<CE::IBoundingBox>()->mitad;
		auto *pb = &B.getTransformada()->posicion;

		float dX = std::abs(pb->x - pa->x);
		float dY = std::abs(pb->y - pa->y);
		bool V = (midA.x + midB.x - dX) > 0;
		bool H = (midA.y + midB.y - dY) > 0;
		bool colision = V && H;

		if (resolucion && colision) *pa = prevA;
		return colision;
	}

	bool checkRayoHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2)
	{
		if (!npc.tieneComponente<CE::IBoundingBox>()) return false;
		auto midnpc = npc.getComponente<CE::IBoundingBox>()->mitad;
		auto posnpc = npc.getTransformada()->posicion;
		auto linea  = p2 - p1;
		float dx = std::abs(posnpc.x - p1.x);
		float dy = std::abs(posnpc.y - p1.y);
		return (midnpc.x + std::abs(linea.x) - dx > 0) &&
			(midnpc.y + std::abs(linea.y) - dy > 0);
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  SPAWNING
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaSpawn(CE::Pool& game_pool, CE::Pool& spawn_zones,
					const std::map<std::string, CE::AnimClip>& zombie_clips,
					CE::Objeto* player_target)
	{
		for (auto& s : spawn_zones.getPool())
		{
			auto respawndata = s->getComponente<CE::IRespawn>();
			if (!respawndata || respawndata->curr_spawn >= respawndata->max_spawn) continue;
			if (respawndata->timer_actual < respawndata->timer_maximo)
			{
				respawndata->timer_actual++;
				continue;
			}

			auto zombie = std::make_shared<Entidad>();

			int size_x = respawndata->width;
			int size_y = respawndata->height;
			float x0 = s->getTransformada()->posicion.x - size_x / 2.f;
			float y0 = s->getTransformada()->posicion.y - size_y / 2.f;
			zombie->setPosicion(x0 + (rand() % size_x), y0 + (rand() % size_y));

			zombie->getStats()->hp = 100;
			zombie->getStats()->hp_max = 100;
			zombie->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};

			zombie->addComponente(
				std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("zombie"), 384, 384, .5f));

			auto fsm_zombie = std::make_shared<IdleZombie>(zombie_clips, 0.1f);
			auto mq_zombie  = std::make_shared<IMaquinaEstado>();
			mq_zombie->fsm  = fsm_zombie;
			zombie->addComponente(mq_zombie);
			zombie->setFSM(fsm_zombie);

			zombie->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{16.f, 16.f}));
			zombie->addComponente(std::make_shared<IRangoAggro>(ZombieConst::RANGO_AGGRO));
			zombie->addComponente(std::make_shared<IEntorno>(&game_pool.getPool()));
			zombie->addComponente(std::make_shared<ITarget>(player_target));

			auto texto_hp = std::make_shared<CE::ITexto>(CE::GestorAssets::Get().getFont("default_font"), "HP: 100");
			texto_hp->m_texto.setCharacterSize(20);
			texto_hp->m_texto.setFillColor(sf::Color::Red);
			texto_hp->m_texto.setOutlineColor(sf::Color::Black);
			texto_hp->m_texto.setOutlineThickness(1.f);
			zombie->addComponente(texto_hp);

			static sf::Glsl::Vec4 zombie_outline_color{1.f, 0.f, 0.f, 1.f};
			static float zombie_outline_thickness = 0.015f;
			static sf::Vector2f zombie_tex_size{384.f, 384.f};

			auto zombie_shader = std::make_shared<CE::IShader>("", ASSETS "/shaders/contorno.frag");
			zombie_shader->setVector4("uOutlineColor", &zombie_outline_color);
			zombie_shader->setEscalar("uThickness", &zombie_outline_thickness);
			zombie_shader->setVector2("uTexSize", &zombie_tex_size);
			zombie->addComponente(zombie_shader);

			game_pool.agregarPool(zombie);
			respawndata->timer_actual = 0;
			respawndata->curr_spawn++;
		}
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  PATHS
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaPaths(std::vector<std::shared_ptr<CE::Objeto>>& obj)
	{
		for (auto& o : obj)
		{
			if (!o->tieneComponente<CE::IPaths>()) continue;
			auto path = o->getComponente<CE::IPaths>();
			int num_curvas = (int)path->puntos.size() / 3;
			if (path->puntos.empty() || path->id_curva >= num_curvas) continue;

			float t = (path->frame_actual_curva % (path->frame_total_curva + 1))
					/ (float)path->frame_total_curva;

			CE::Vector2D P0 = path->puntos[0 + path->id_curva * (path->offset - 0)];
			CE::Vector2D P1 = path->puntos[1 + path->id_curva * (path->offset - 0)];
			CE::Vector2D P2 = path->puntos[2 + path->id_curva * (path->offset - 0)];
			auto pos = CE::lerp2(P0, P1, P2, t);
			path->frame_actual_curva++;
			o->setPosicion(pos.x, pos.y);

			if (t == 1.f) path->id_curva++;

			if (path->id_curva == 2 && o->tieneComponente<CE::IShader>())
			{
				auto shader = o->getComponente<CE::IShader>();
				shader->cambiarShader("", ASSETS "/shaders/color.frag");
				static sf::Glsl::Vec4 color{0.f, 1.f, 0.f, 1.f};
				shader->m_vars.clear();
				shader->setVector4("color", &color);
			}
			else if (o->tieneComponente<CE::IShader>())
			{
				auto shader = o->getComponente<CE::IShader>();
				shader->cambiarShader("", ASSETS "/shaders/contorno.frag");
				static sf::Glsl::Vec4 outlinecolor{0.f, 1.f, 0.f, 1.f};
				static float thickness = 0.1f;
				static sf::Glsl::Vec2 size{63, 92};
				shader->m_vars.clear();
				shader->setVector4("uOutlineColor", &outlinecolor);
				shader->setEscalar("uThickness", &thickness);
				shader->setVector2("uTexSize", &size);
			}
		}
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  DIALOG & INTERACTION
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaDialogos(IVJ::IDialogo* dialogo, CE::Objeto& obj)
	{
		if (!dialogo) return;
		dialogo->activo = true;
		dialogo->onInteractuar(obj);
	}

	bool revisarDistanciaInteraccion(CE::Objeto& jugador, CE::Objeto& npc, float distancia_maxima)
	{
		auto pos_jugador = jugador.getTransformada()->posicion;
		auto pos_npc	 = npc.getTransformada()->posicion;
		return pos_jugador.distancia(pos_npc) <= distancia_maxima;
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  DEBUG
	// ─────────────────────────────────────────────────────────────────────────────

	void pintarLinea(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color)
	{
		sf::CircleShape cp1{2.f}; cp1.setFillColor({0,0,0,255}); cp1.setPosition({p1.x, p1.y});
		sf::CircleShape cp2{2.f}; cp2.setFillColor({0,0,0,255}); cp2.setPosition({p2.x, p2.y});
		for (float t = 0; t <= 1; t += 0.0015f)
		{
			sf::CircleShape pixel{1};
			pixel.setFillColor(color);
			auto pp = CE::lerp(p1, p2, t);
			pixel.setPosition({pp.x, pp.y});
			CE::Render::Get().AddToDraw(pixel);
		}
		CE::Render::Get().AddToDraw(cp1);
		CE::Render::Get().AddToDraw(cp2);
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  FIGURE SYSTEMS (lab)
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaGirar(CE::Objeto& ente, float dt)
	{
		if (!ente.getComponente<IGirar>()) return;
		auto comp = ente.getComponente<IGirar>();
		auto pos  = ente.getTransformada()->posicion;
		ente.setPosicion(pos.x + comp->radio * std::cos(comp->angulo),
						pos.y + comp->radio * std::sin(comp->angulo));
		comp->angulo += 3.1416f * dt;
	}

	void SistemaRectanguloMoverUpDown(CE::Objeto& ente, float dt)
	{
		if (!ente.getComponente<IArribaAbajo>()) return;
		auto comp = ente.getComponente<IArribaAbajo>();
		auto pos  = ente.getTransformada()->posicion;
		pos.y += comp->velocidad * dt;
		if	  (pos.y < comp->minY) { pos.y = comp->minY; comp->velocidad *= -1.f; }
		else if (pos.y > comp->maxY) { pos.y = comp->maxY; comp->velocidad *= -1.f; }
		ente.setPosicion(pos.x, pos.y);
	}

	void SistemaTrianguloWave(CE::Objeto& ente, float dt)
	{
		if (!ente.getComponente<IWave>()) return;
		auto comp = ente.getComponente<IWave>();
		auto pos  = ente.getTransformada()->posicion;
		pos.x += comp->amp * std::sin(comp->freq * comp->phase);
		pos.y += comp->amp / 2.f * std::cos(2.f * comp->freq * comp->phase);
		comp->phase += dt;
		ente.setPosicion(pos.x, pos.y);
	}

	void SistemaCirculoMoverEnCirculos(CE::Objeto& ente, float dt)
	{
		if (!ente.getComponente<ICirculos>()) return;
		auto comp = ente.getComponente<ICirculos>();
		auto pos  = ente.getTransformada()->posicion;
		pos.x += comp->radio * std::cos(comp->velocidad);
		pos.y += comp->radio * std::sin(comp->velocidad);
		comp->velocidad += dt;
		ente.setPosicion(pos.x, pos.y);
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  NPC / ZOMBIE AI
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaNPCLookTarget(CE::Objeto& ente, CE::Vector2D& target)
	{
		if (!ente.tieneComponente<IRangoAggro>()) return;
		auto rango = ente.getComponente<IRangoAggro>();
		if (!rango->estaDentroRango(ente, target)) return;
		auto pos = ente.getTransformada()->posicion;
		CE::Vector2D dir = target - pos;
		ente.getTransformada()->angulo = std::atan2(dir.y, dir.x) + (float)M_PI / 2.f;
	}

	bool checkLOS(const CE::Vector2D& p1, const CE::Vector2D& p2,
				const std::vector<std::shared_ptr<CE::Objeto>>& obstaculos,
				CE::Objeto* ignore)
	{
		for (auto& obs : obstaculos)
		{
			if (obs.get() == ignore) continue;
			if (!obs->tieneComponente<CE::IBoundingBox>()) continue;

			auto bb	= obs->getComponente<CE::IBoundingBox>();
			auto trans = obs->getTransformada();

			float minX = trans->posicion.x - bb->mitad.x;
			float maxX = minX + bb->tam.x;
			float minY = trans->posicion.y - bb->mitad.y;
			float maxY = minY + bb->tam.y;

			float dx = p2.x - p1.x;
			float dy = p2.y - p1.y;
			float tmin = 0.f, tmax = 1.f;

			if (std::abs(dx) > 0.0001f)
			{
				float tx1 = (minX - p1.x) / dx;
				float tx2 = (maxX - p1.x) / dx;
				tmin = std::max(tmin, std::min(tx1, tx2));
				tmax = std::min(tmax, std::max(tx1, tx2));
			}
			else { if (p1.x < minX || p1.x > maxX) continue; }

			if (std::abs(dy) > 0.0001f)
			{
				float ty1 = (minY - p1.y) / dy;
				float ty2 = (maxY - p1.y) / dy;
				tmin = std::max(tmin, std::min(ty1, ty2));
				tmax = std::min(tmax, std::max(ty1, ty2));
			}
			else { if (p1.y < minY || p1.y > maxY) continue; }

			if (tmax >= tmin && tmax > 0.f) return false;
		}
		return true;
	}

	void SistemaZombieSeguir(CE::Objeto& zombie, CE::Vector2D& target_pos,
							const std::vector<std::shared_ptr<CE::Objeto>>& obstaculos, float dt)
	{
		auto trans = zombie.getTransformada();
		auto rango = zombie.getComponente<IRangoAggro>();
		if (!rango) return;

		CE::Vector2D pos_actual = trans->posicion;
		float dist = pos_actual.distancia(target_pos);
		bool en_rango  = rango->estaDentroRango(zombie, target_pos);
		bool tiene_los = checkLOS(pos_actual, target_pos, obstaculos, &zombie);

		CE::Vector2D vel{0.f, 0.f};

		if (en_rango && tiene_los)
		{
			if (dist < ZombieConst::DISTANCIA_ATAQUE)
			{
				vel = {0.f, 0.f};
			}
			else
			{
				CE::Vector2D dir = target_pos - pos_actual;
				dir.normalizacion();

				// Forward obstacle probe
				CE::Vector2D probe = pos_actual;
				CE::Vector2D probe_offset{dir.x * 30.f, dir.y * 30.f};
				probe.suma(probe_offset);

				if (!checkLOS(pos_actual, probe, obstaculos, &zombie))
				{
					float ang = std::atan2(dir.y, dir.x);
					CE::Vector2D dirL{std::cos(ang - 0.78f), std::sin(ang - 0.78f)};
					CE::Vector2D dirR{std::cos(ang + 0.78f), std::sin(ang + 0.78f)};
					CE::Vector2D pL{pos_actual.x + dirL.x * 40.f, pos_actual.y + dirL.y * 40.f};
					CE::Vector2D pR{pos_actual.x + dirR.x * 40.f, pos_actual.y + dirR.y * 40.f};

					if	  (checkLOS(pos_actual, pL, obstaculos, &zombie)) dir = dirL;
					else if (checkLOS(pos_actual, pR, obstaculos, &zombie)) dir = dirR;
				}

				dir.escala(ZombieConst::VELOCIDAD);
				vel = dir;
			}
		}
		else
		{
			// Friction when losing target
			vel = trans->velocidad;
			vel.escala(0.9f);
			if (vel.longitud() < 1.f) vel = {0.f, 0.f};
		}

		trans->velocidad = vel;

		// Rotation
		if (zombie.tieneComponente<CE::ISprite>())
		{
			if (tiene_los && en_rango)
			{
				CE::Vector2D mirar = target_pos - pos_actual;
				mirar.normalizacion();
				trans->angulo = std::atan2(mirar.x, -mirar.y);
			}
			else if (vel.longitud() > 0.1f)
			{
				CE::Vector2D cam = vel;
				cam.normalizacion();
				trans->angulo = std::atan2(cam.x, -cam.y);
			}
		}

		trans->posicion.suma(CE::Vector2D{vel.x * dt, vel.y * dt});
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  COMBAT — MELEE
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaAtaqueMelee(CE::Objeto& atacante, CE::Pool& game_pool)
	{
		if (!atacante.tieneComponente<IArmaMelee>()) return;
		auto arma = atacante.getComponente<IArmaMelee>();

		if (!arma->atacando)
		{
			arma->objs_golpeados.clear();
			arma->swing_played = false;
			return;
		}

		auto trans = atacante.getTransformada();
		CE::Vector2D centro_hitbox{
			trans->posicion.x + std::sin(trans->angulo) * arma->offset_adelante,
			trans->posicion.y - std::cos(trans->angulo) * arma->offset_adelante
		};
		arma->hitbox_debug.setPosition({centro_hitbox.x, centro_hitbox.y});

		bool ha_golpeado = false;

		for (const auto& obj_ptr : game_pool.getPool())
		{
			CE::Objeto& obj = *obj_ptr;
			if (&obj == &atacante) continue;
			if (!obj.tieneComponente<ITarget>()) continue;
			if (arma->objs_golpeados.count(&obj)) continue;

			CE::Vector2D pos_enemigo = obj.getTransformada()->posicion;
			float dx = pos_enemigo.x - centro_hitbox.x;
			float dy = pos_enemigo.y - centro_hitbox.y;
			float dist_sq = dx * dx + dy * dy;
			float radio_sum = arma->radio + 20.f;

			if (dist_sq < radio_sum * radio_sum)
			{
				auto stats = obj.getStats();
				if (stats->hp > 0)
				{
					stats->hp = (stats->hp > arma->damage) ? stats->hp - arma->damage : 0;
					
					// Dropear salud al matar con cuchillo
					if (stats->hp == 0 && (std::rand() % 100) < 50) {
						auto spawner = std::make_shared<Entidad>();
						spawner->setPosicion(pos_enemigo.x, pos_enemigo.y);
						spawner->addComponente(std::make_shared<IDelayedSpawn>(2.0f, IDropItem::Tipo::Salud));
						game_pool.agregarPool(spawner);
					}

					int num = std::rand() % 31 + 1;
					std::string key_zombie = std::string("zombie_hurt_") + (num < 10 ? "0" : "") + std::to_string(num);
					CE::GestorAssets::Get().getSonido(key_zombie).setVolume(20.f);
					CE::GestorAssets::Get().getSonido(key_zombie).play();
					arma->objs_golpeados.insert(&obj);
					ha_golpeado = true;

					if (obj.tieneComponente<CE::ITexto>())
						obj.getComponente<CE::ITexto>()->m_texto.setString(
							"HP: " + std::to_string((int)stats->hp));

					// Blood splash FX
					static bool blood_loaded = false;
					if (!blood_loaded)
					{
						CE::GestorAssets::Get().agregarTextura("blood_splash",
							ASSETS "/fx/BloodFXBatch1/BloodSplash/bloodsplash-atlas.png",
							CE::Vector2D{0.f, 0.f}, CE::Vector2D{440.f, 279.f});
						blood_loaded = true;
					}

					auto sangre = std::make_shared<Entidad>();
					sangre->addComponente(std::make_shared<CE::ISprite>(
						CE::GestorAssets::Get().getTextura("blood_splash"), 110, 93, 1.f));
					sangre->setPosicion(pos_enemigo.x, pos_enemigo.y);
					sangre->addComponente(std::make_shared<ITemporal>());

					auto fsm_sangre = std::make_shared<BloodSplash>(CE::AnimClip{0, 11}, 0.03f);
					auto mq = std::make_shared<IMaquinaEstado>();
					mq->fsm = fsm_sangre;
					sangre->addComponente(mq);
					sangre->setFSM(fsm_sangre);
					game_pool.agregarPool(sangre);
				}
			}
		}

		if (!arma->swing_played)
		{
			std::string key = ha_golpeado
				? "slash0"  + std::to_string((std::rand() % 4) + 1)
				: "swing0"  + std::to_string((std::rand() % 4) + 1);
			CE::GestorAssets::Get().getSonido(key).setVolume(50.f);
			CE::GestorAssets::Get().getSonido(key).play();
			arma->swing_played = true;
		}
	}

	// ─────────────────────────────────────────────────────────────────────────────
	//  COMBAT — GUN / BULLETS
	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaDisparar(CE::Objeto& tirador, CE::Objeto& cuerpo,
						IPistola& arma, CE::Pool& game_pool,
						const sf::Texture& bala_tex)
	{
		if (!arma.puedeDisparar()) return;
		arma.iniciarDisparo();

		auto trans   = cuerpo.getTransformada();
		float angulo = trans->angulo;

		// Bullet direction from cuerpo angle  (same convention as hitbox: sin/cos)
		float dir_x = std::sin(angulo);
		float dir_y = -std::cos(angulo);

		constexpr float BALA_SPEED = 800.f;

		auto bala_ent = std::make_shared<Entidad>();
		bala_ent->getStats()->hp	 = 1;
		bala_ent->getStats()->hp_max = 1;

		// Spawn slightly ahead of the muzzle so it doesn't immediately self-collide
		constexpr float MUZZLE_OFFSET = 24.f;
		bala_ent->setPosicion(
			trans->posicion.x + dir_x * MUZZLE_OFFSET,
			trans->posicion.y + dir_y * MUZZLE_OFFSET);

		bala_ent->getTransformada()->velocidad = CE::Vector2D{dir_x * BALA_SPEED, dir_y * BALA_SPEED};
		bala_ent->getTransformada()->angulo	= angulo;

		// Sprite (16×16 tile from atlas, scaled ×3)
		auto bala_sprite = std::make_shared<CE::ISprite>(bala_tex, 16, 16, 3.f);
		bala_sprite->m_sprite.setRotation(sf::radians(angulo));
		bala_ent->addComponente(bala_sprite);
		bala_ent->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{12.f, 12.f}));

		auto bala_comp = std::make_shared<IBala>();
		bala_comp->damage	= 25;
		bala_comp->rango_max = 600.f;
		bala_comp->origen	= &tirador;   // avoid hitting the shooter
		bala_comp->activa	= true;
		bala_ent->addComponente(bala_comp);

		game_pool.agregarPool(bala_ent);
	}

	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaActualizarBalas(CE::Pool& game_pool, float dt)
	{
		for (auto& obj_ptr : game_pool.getPool())
		{
			CE::Objeto& obj = *obj_ptr;
			if (!obj.tieneComponente<IBala>()) continue;

			auto bala  = obj.getComponente<IBala>();
			if (!bala->activa) continue;

			auto trans = obj.getTransformada();

			// Displacement this frame
			CE::Vector2D disp{trans->velocidad.x * dt, trans->velocidad.y * dt};
			bala->distancia_viajada += disp.longitud();
			trans->posicion.suma(disp);

			// Update sprite position and rotation
			if (obj.tieneComponente<CE::ISprite>())
			{
				auto& spr = obj.getComponente<CE::ISprite>()->m_sprite;
				spr.setPosition({trans->posicion.x, trans->posicion.y});
				spr.setRotation(sf::radians(trans->angulo));
			}

			// Deactivate when max range reached
			if (bala->distancia_viajada >= bala->rango_max)
			{
				bala->activa	   = false;
				obj.getStats()->hp = 0;   // marks for pool cleanup
			}
		}
	}

	// ─────────────────────────────────────────────────────────────────────────────

	void SistemaColBalas(CE::Pool& game_pool)
	{
		// Collect active bullets first (avoid iterator invalidation from pool additions)
		std::vector<CE::Objeto*> balas_activas;
		for (auto& ptr : game_pool.getPool())
		{
			if (ptr->tieneComponente<IBala>())
			{
				auto b = ptr->getComponente<IBala>();
				if (b->activa) balas_activas.push_back(ptr.get());
			}
		}

		for (CE::Objeto* bala_ptr : balas_activas)
		{
			auto bala = bala_ptr->getComponente<IBala>();
			if (!bala->activa) continue;   // may have been hit this frame already

			for (auto& target_ptr : game_pool.getPool())
			{
				CE::Objeto& target = *target_ptr;
				if (&target == bala_ptr)	  continue;   // skip self
				if (&target == bala->origen)  continue;   // skip shooter
				if (!target.tieneComponente<ITarget>()) continue;

				if (SistemaColAABBMid(*bala_ptr, target, false))
				{
					// Deactivate bullet
					bala->activa		  = false;
					bala_ptr->getStats()->hp = 0;

					// Apply damage
					auto stats = target.getStats();
					if (stats->hp > 0)
					{
						stats->hp = (stats->hp > bala->damage) ? stats->hp - bala->damage : 0;
						
						// Dropear municion al matar con bala
						if (stats->hp == 0 && (std::rand() % 100) < 25) {
							auto spawner = std::make_shared<Entidad>();
							spawner->setPosicion(target.getTransformada()->posicion.x, target.getTransformada()->posicion.y);
							spawner->addComponente(std::make_shared<IDelayedSpawn>(2.0f, IDropItem::Tipo::Municion));
							game_pool.agregarPool(spawner);
						}

						// key random entre el 01 y el 31
						int num = std::rand() % 31 + 1;
						std::string key = std::string("zombie_hurt_") + (num < 10 ? "0" : "") + std::to_string(num);
						CE::GestorAssets::Get().getSonido(key).setVolume(20.f);
						CE::GestorAssets::Get().getSonido(key).play();

						if (target.tieneComponente<CE::ITexto>())
							target.getComponente<CE::ITexto>()->m_texto.setString(
								"HP: " + std::to_string((int)stats->hp));

						// Blood splash FX
						static bool blood_loaded = false;
						if (!blood_loaded)
						{
							CE::GestorAssets::Get().agregarTextura("blood_splash",
								ASSETS "/fx/BloodFXBatch1/BloodSplash/bloodsplash-atlas.png",
								CE::Vector2D{0.f, 0.f}, CE::Vector2D{440.f, 279.f});
							blood_loaded = true;
						}

						auto sangre = std::make_shared<Entidad>();
						sangre->addComponente(std::make_shared<CE::ISprite>(
							CE::GestorAssets::Get().getTextura("blood_splash"), 110, 93, 1.f));
						sangre->setPosicion(target.getTransformada()->posicion.x,
											target.getTransformada()->posicion.y);
						sangre->addComponente(std::make_shared<ITemporal>());

						auto fsm_sangre = std::make_shared<BloodSplash>(CE::AnimClip{0, 11}, 0.03f);
						auto mq = std::make_shared<IMaquinaEstado>();
						mq->fsm = fsm_sangre;
						sangre->addComponente(mq);
						sangre->setFSM(fsm_sangre);
						game_pool.agregarPool(sangre);
					}
					break;   // each bullet hits only one target per frame
				}
			}
		}
	}

} // namespace IVJ
