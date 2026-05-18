#include "Sistemas.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../objetos/Entidad.hpp"
#include <math.h>
#include <cstdlib>
#include "../../Motor/Utils/Lerp.hpp"
#include "../../Motor/Render/Render.hpp"
#include <memory>
#include <Juego/Componentes/IJComponentes.hpp>
#include <Juego/Maquinas/FSMZombie.hpp>
#include "../../Motor/Camaras/CamarasGestor.hpp"
#include <SFML/Window/Mouse.hpp>

#include <Juego/Maquinas/Zombie/IdleZombie.hpp>

namespace IVJ
{
	void SistemaControl(CE::Objeto&ente, float dt)
	{
		auto p = ente.getTransformada();
		auto c = ente.getComponente<CE::IControl>();
		(void)dt;
		if(c->arr)
			p->velocidad.y=-200;
		if(c->abj)
			p->velocidad.y=200;
		if(c->der)
			p->velocidad.x=200;
		if(c->izq)
			p->velocidad.x=-200;
		if(c->run)
			p->velocidad.escala(2.f);
	}

	void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt)
	{
		for(auto& ente : entes)
		{
			auto trans = ente->getTransformada();
			trans->posicion.suma(trans->velocidad.escala(dt));
		}
	}

	void SistemaMover(const std::shared_ptr<CE::Objeto>& objeto, float dt) {
		auto trans = objeto->getTransformada();

		auto control = objeto->getComponente<CE::IControl>();

		// si no tiene control o el control no esta activo, salir
		if (!control || !control->isActivo()) return;

		// Calcular la dirección basada en los inputs
		auto dir = CE::Vector2D{0.f, 0.f};
		if (control->arr) dir.y -= 1.f;
		if (control->abj) dir.y += 1.f;
		if (control->der) dir.x += 1.f;
		if (control->izq) dir.x -= 1.f;

		auto vel = CE::Vector2D{0.f, 0.f};

		// Si hay movimiento, normalizar para que las diagonales no sean más rápidas
		if (dir.longitud() > 0.f) {
			dir.normalizacion();

			// Definir la velocidad final usando la velocidad base del transform
			vel.x = dir.x * trans->velocidad.x;
			vel.y = dir.y * trans->velocidad.y;

			// Si el botón de correr está presionado, multiplicar la velocidad por 1.5x
			if (control->run) {
				vel.escala(1.5f);
			}
		}

		// calcular la dirección para la rotación (omnimovimiento: mirar al cursor)
		if (objeto->getComponente<ITriangulo>() || objeto->getComponente<CE::ISprite>()) {
			// Obtenemos la posición del ratón proyectada al espacio de juego (1920x1080)
			CE::Vector2D gamePos = CE::Render::Get().GetMouseGamePos();
			
			// Ahora mapeamos esa posición de la textura al mundo usando la cámara
			sf::Vector2f worldPos = CE::Render::Get().GetTextura().mapPixelToCoords(
				sf::Vector2i(gamePos.x, gamePos.y), 
				CE::GestorCamaras::Get().getCamaraActiva().getView());
			
			CE::Vector2D targetPos{worldPos.x, worldPos.y};
			CE::Vector2D dir_mouse = targetPos - trans->posicion;

			auto trian = objeto->getComponente<ITriangulo>();
			if (trian)
				trian->angulo = std::atan2(dir_mouse.x, -dir_mouse.y);
			trans->angulo = std::atan2(dir_mouse.x, -dir_mouse.y);
		}

		// actualizar posición del ente
		trans->posicion.suma(vel.escala(dt));
	}

	bool SistemaColAABB(CE::Objeto& A, CE::Objeto& B, bool resolucion)
	{
		if(!A.tieneComponente<CE::IBoundingBox>() || !B.tieneComponente<CE::IBoundingBox>())
			return false;
		auto bA = A.getComponente<CE::IBoundingBox>()->tam;
		auto mA = A.getComponente<CE::IBoundingBox>()->mitad;
		auto *pa = &A.getTransformada()->posicion;
		auto prevA = A.getTransformada()->pos_prev;
		auto bB = B.getComponente<CE::IBoundingBox>()->tam;
		auto mB = B.getComponente<CE::IBoundingBox>()->mitad;
		auto pb = B.getTransformada()->posicion;
		bool H = pa->y-mA.y < pb.y+bB.y - mB.y && pb.y-mB.y < pa->y+bA.y - mA.y;
		bool V = pa->x-mA.x < pb.x+bB.x - mB.x && pb.x-mB.x < pa->x+bA.x - mA.x;
		bool hay_colision = H && V;
		if(hay_colision && resolucion)
			*pa = prevA;
		return hay_colision;
	}

	bool SistemaColAABBMid(CE::Objeto& A, CE::Objeto& B, bool resolucion)
	{
		if(!A.tieneComponente<CE::IBoundingBox>() || !B.tieneComponente<CE::IBoundingBox>())
			return false;
		auto midA = A.getComponente<CE::IBoundingBox>()->mitad;
		auto *pa = &A.getTransformada()->posicion;
		auto prevA = A.getTransformada()->pos_prev;
		auto midB = B.getComponente<CE::IBoundingBox>()->mitad;
		auto *pb = &B.getTransformada()->posicion;
		float dX = std::abs(pb->x - pa->x);
		float dY = std::abs(pb->y - pa->y);
		float sumMidX = midA.x + midB.x;
		float sumMidY = midA.y + midB.y;
		bool V = sumMidX - dX > 0;
		bool H = sumMidY - dY > 0;
		bool hay_colision = V && H;
		if(resolucion && hay_colision)
			*pa = prevA;
		return hay_colision;
	}

	bool checkRayoHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2)
	{
		if(!npc.tieneComponente<CE::IBoundingBox>()) return false;
		auto midnpc = npc.getComponente<CE::IBoundingBox>()->mitad;
		auto posnpc = npc.getTransformada()->posicion;
		auto linea = (p2-p1);
		float dx = std::abs(posnpc.x - p1.x);
		float dy = std::abs(posnpc.y - p1.y);
		float sumMidX = midnpc.x + std::abs(linea.x) ;
		float sumMidY = midnpc.y + std::abs(linea.y) ;
		return (sumMidX - dx >0) && (sumMidY - dy >0);
	}

	void SistemaSpawn(CE::Pool& game_pool, CE::Pool& spawn_zones, const std::map<std::string, CE::AnimClip>& zombie_clips, CE::Objeto* player_target)
	{
		for(auto &s : spawn_zones.getPool())
		{
			auto respawndata = s->getComponente<CE::IRespawn>();
			if(!respawndata || respawndata->curr_spawn >= respawndata->max_spawn)
				continue;
			if(respawndata->timer_actual >= respawndata->timer_maximo)
			{
				auto zombie = std::make_shared<Entidad>();
				// Set initial position
				int size_x = respawndata->width;
				int size_y = respawndata->height;
				float x0 = s->getTransformada()->posicion.x - size_x/2.f;
				float y0 = s->getTransformada()->posicion.y - size_y/2.f;
				float x = x0 + (rand() % size_x);
				float y = y0 + (rand() % size_y);
				zombie->setPosicion(x,y);

				// Basic Stats
				zombie->getStats()->hp = 100;
				zombie->getStats()->hp_max = 100;
				zombie->getTransformada()->velocidad = CE::Vector2D{0.f, 0.f};

				// Sprite
				zombie->addComponente(
					std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("zombie"), 384, 384, .5f));

				// FSM
				auto fsm_zombie = std::make_shared<IdleZombie>(zombie_clips, 0.1f);
				auto mq_zombie = std::make_shared<IMaquinaEstado>();
				mq_zombie->fsm = fsm_zombie;
				zombie->addComponente(mq_zombie);
				zombie->setFSM(fsm_zombie);

				// Bounding Box
				zombie->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{16.f, 16.f}));
				// Aggro Range
				zombie->addComponente(std::make_shared<IRangoAggro>(ZombieConst::RANGO_AGGRO));
				// Environment (for LOS)
				zombie->addComponente(std::make_shared<IEntorno>(&game_pool.getPool()));
				// Target (player)
				zombie->addComponente(std::make_shared<ITarget>(player_target));

				// Text HP debug
				auto texto_hp = std::make_shared<CE::ITexto>(CE::GestorAssets::Get().getFont("default_font"), "HP: 100");
				texto_hp->m_texto.setCharacterSize(20);
				texto_hp->m_texto.setFillColor(sf::Color::Red);
				texto_hp->m_texto.setOutlineColor(sf::Color::Black);
				texto_hp->m_texto.setOutlineThickness(1.f);
				zombie->addComponente(texto_hp);

				// Basic Shader: Outline (default red for enemies)
				auto zombie_outline_color = sf::Glsl::Vec4{1.f, 0.f, 0.f, 1.f};
				float zombie_outline_thickness = 0.015f;
				sf::Vector2f zombie_tex_size = {384.f, 384.f};

				auto zombie_shader = std::make_shared<CE::IShader>("", ASSETS "/shaders/contorno.frag");
				zombie_shader->setVector4("uOutlineColor", &zombie_outline_color);
				zombie_shader->setEscalar("uThickness", &zombie_outline_thickness);
				zombie_shader->setVector2("uTexSize", &zombie_tex_size);
				zombie->addComponente(zombie_shader);

				game_pool.agregarPool(zombie);
				respawndata->timer_actual=0;
				respawndata->curr_spawn++;
			}
			respawndata->timer_actual++;
		}
	}
	void SistemaPaths(std::vector<std::shared_ptr<CE::Objeto>>& obj)
	{
		for(auto &o: obj)
		{
			if(o->tieneComponente<CE::IPaths>())
			{
				auto path = o->getComponente<CE::IPaths>();
				int num_curvas = path->puntos.size()/3;
				if(path->puntos.size()==0) continue;
				if(path->id_curva >= num_curvas) continue;
				float t = (path->frame_actual_curva%(path->frame_total_curva+1))/static_cast<float>(path->frame_total_curva);
				CE::Vector2D P0 = path->puntos[0+path->id_curva*(path->offset-0)];
				CE::Vector2D P1 = path->puntos[1+path->id_curva*(path->offset-0)];
				CE::Vector2D P2 = path->puntos[2+path->id_curva*(path->offset-0)];
				auto posiciones_actual = CE::lerp2(P0, P1, P2, t);
				path->frame_actual_curva++;
				o->setPosicion(posiciones_actual.x,posiciones_actual.y);
				if(t == 1.f )
					path->id_curva++;
				if(path->id_curva == 2 && o->tieneComponente<CE::IShader>())
				{
					auto shader = o->getComponente<CE::IShader>();
					shader->cambiarShader("",ASSETS "/shaders/color.frag");
					static sf::Glsl::Vec4 color{0.f,1.f,0.f,1.f};
					shader->m_vars.clear();
					shader->setVector4("color",&color);
				}
				else
				{
					auto shader = o->getComponente<CE::IShader>();
					shader->cambiarShader("",ASSETS "/shaders/contorno.frag");
					static sf::Glsl::Vec4 outlinecolor{0.f,1.f,0.f,1.f};
					static float thickness = 0.1;
					static sf::Glsl::Vec2 size{63,92};
					shader->m_vars.clear();
					shader->setVector4("uOutlineColor",&outlinecolor);
					shader->setEscalar("uThickness",&thickness);
					shader->setVector2("uTexSize",&size);
				}
			}
		}
	}

	void SistemaDialogos(IVJ::IDialogo* dialogo,CE::Objeto& obj)
	{
		if(!dialogo) return;
		dialogo->activo = true;
		dialogo->onInteractuar(obj);
	}

	bool revisarDistanciaInteraccion(CE::Objeto& jugador, CE::Objeto& npc, float distancia_maxima)
	{
		auto pos_jugador = jugador.getTransformada()->posicion;
		auto pos_npc = npc.getTransformada()->posicion;
		float distancia = pos_jugador.distancia(pos_npc);
		return distancia <= distancia_maxima;
	}

	void pintarLinea(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color)
	{
		sf::CircleShape cp1{2.f};
		cp1.setFillColor({0,0,0,255});
		sf::CircleShape cp2{2.f};
		cp2.setFillColor({0,0,0,255});
		cp1.setPosition({p1.x,p1.y});
		cp2.setPosition({p2.x,p2.y});
		for(float t=0;t<=1;t+=0.0015)
		{
			sf::CircleShape pixel{1};
			pixel.setFillColor(color);
			auto puntopos  = CE::lerp(p1,p2,t);
			pixel.setPosition({puntopos.x,puntopos.y});
			CE::Render::Get().AddToDraw(pixel);
		}
		CE::Render::Get().AddToDraw(cp1);
		CE::Render::Get().AddToDraw(cp2);
	}

	void SistemaGirar(CE::Objeto &ente, float dt) {
		if (!ente.getComponente<IGirar>()) return;
		auto componente = ente.getComponente<IGirar>();
		auto pos = ente.getTransformada()->posicion;
		float radio = componente->radio;
		float x = radio *cos(componente->angulo);
		float y = radio *sin(componente->angulo);
		ente.setPosicion(pos.x + x, pos.y + y);
		componente->angulo += 3.1416f * dt;
	}

	void SistemaRectanguloMoverUpDown(CE::Objeto &ente, float dt) {
		if (!ente.getComponente<IArribaAbajo>()) return;
		auto componente = ente.getComponente<IArribaAbajo>();
		auto pos = ente.getTransformada()->posicion;
		float vel = componente->velocidad;
		float minY = componente->minY;
		float maxY = componente->maxY;
		pos.y += vel * dt;
		if (pos.y < minY) {
			pos.y = minY;
			componente->velocidad *= -1.f;
		}
		else if (pos.y > maxY) {
			pos.y = maxY;
			componente->velocidad *= -1.f;
		}
		ente.setPosicion(pos.x, pos.y);
	}

	void SistemaTrianguloWave(CE::Objeto &ente, float dt) {
		if (!ente.getComponente<IWave>()) return;
		auto componente = ente.getComponente<IWave>();
		auto pos = ente.getTransformada()->posicion;
		float amp = componente->amp;
		float freq = componente->freq;
		float phase = componente->phase;
		pos.x += amp * sin(freq * phase);
		pos.y += amp/2 * cos(2 * freq * phase);
		componente->phase += dt;
		ente.setPosicion(pos.x, pos.y);
	}

	void SistemaCirculoMoverEnCirculos(CE::Objeto &ente, float dt) {
		if (!ente.getComponente<ICirculos>()) return;
		auto componente = ente.getComponente<ICirculos>();
		auto pos = ente.getTransformada()->posicion;
		float radio = componente->radio;
		float velocidad = componente->velocidad;
		float x = radio * cos(velocidad);
		float y = radio * sin(velocidad);
		pos.x += x;
		pos.y += y;
		componente->velocidad += dt;
		ente.setPosicion(pos.x, pos.y);
	}

	void SistemaEpidemia(CE::Pool& pool, int columnas, float dt)
	{
		auto& objetos = pool.getPool();
		int total  = static_cast<int>(objetos.size());
		int filas  = (columnas > 0) ? total / columnas : 0;

		using Estado = IVJ::IEstadoEpidemia::Estado;

		// actualizar timers y transiciones de estado
		for (auto& obj : objetos)
		{
			auto epidemia = obj->getComponente<IVJ::IEstadoEpidemia>();
			if (!epidemia) continue;

			epidemia->pendiente_contagiar = false;

			switch (epidemia->estado_actual)
			{
				// ────────────────────────────────────────────────
				case Estado::Enfermo:
				{
					epidemia->timer_estado += dt;
					epidemia->timer_tick   += dt;

					// Dispara un tick de contagio cada segundo (máx 3)
					while (epidemia->timer_tick  >= IVJ::IEstadoEpidemia::TICK_INTERVALO
						&& epidemia->ticks_realizados < 3)
					{
						epidemia->timer_tick	   -= IVJ::IEstadoEpidemia::TICK_INTERVALO;
						epidemia->ticks_realizados++;
						epidemia->pendiente_contagiar = true;
					}

					// Tras 3 s → recuperación
					if (epidemia->timer_estado >= IVJ::IEstadoEpidemia::DUR_ENFERMO)
					{
						epidemia->estado_actual	= Estado::Recuperacion;
						epidemia->timer_estado	 = 0.f;
						epidemia->ticks_realizados = 0;
						epidemia->timer_tick	   = 0.f;
					}
					break;
				}

				// ────────────────────────────────────────────────
				case Estado::Recuperacion:
				{
					epidemia->timer_estado += dt;
					if (epidemia->timer_estado >= IVJ::IEstadoEpidemia::DUR_RECUPERACION)
					{
						epidemia->timer_estado = 0.f;

						// Muere?
						float r_muerte = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						if (r_muerte < IVJ::IEstadoEpidemia::PROB_MUERTE)
						{
							epidemia->estado_actual   = Estado::Muerto;
							obj->getStats()->hp = 0;
						}
						else
						{
							// Calcula si se obtiene inmunidad permanente
							float r_inm = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
							if (r_inm < epidemia->prob_inmunidad_permanente)
							{
								epidemia->estado_actual = Estado::Inmune;
							}
							else
							{
								// Inmunidad parcial temporal (2 s)
								epidemia->estado_actual		   = Estado::Normal;
								epidemia->tiene_inmunidad_parcial = true;
								epidemia->timer_inmunidad_parcial = 0.f;
							}
						}
					}
					break;
				}

				// ────────────────────────────────────────────────
				case Estado::Normal:
				{
					// Descontar inmunidad parcial
					if (epidemia->tiene_inmunidad_parcial)
					{
						epidemia->timer_inmunidad_parcial += dt;
						if (epidemia->timer_inmunidad_parcial >= IVJ::IEstadoEpidemia::DUR_INMUNIDAD_PARCIAL)
						{
							epidemia->tiene_inmunidad_parcial	= false;
							epidemia->timer_inmunidad_parcial	= 0.f;
						}
					}
					break;
				}

				// Inmune y Muerto no hacen nada
				case Estado::Inmune:
				case Estado::Muerto:
				default:
					break;
			}
		}

		// contagio a vecinos adyacentes (arriba, abajo, izquierda, derecha)
		// Solo las personas con pendiente_contagiar = true intentan infectar
		for (auto& obj : objetos)
		{
			auto ep = obj->getComponente<IVJ::IEstadoEpidemia>();
			if (!ep || !ep->pendiente_contagiar) continue;

			int f = ep->fila;
			int c = ep->columna;

			// Deltas para: arriba, abajo, izquierda, derecha

			for (int k = 0; k < 4; ++k)
			{
				constexpr int dc[4] = {  0,  0, -1,  1 };
				constexpr int df[4] = { -1,  1,  0,  0 };

				int vf = f + df[k];
				int vc = c + dc[k];

				// Comprobar límites de la cuadrícula
				if (vf < 0 || vf >= filas || vc < 0 || vc >= columnas) continue;

				int idx = vf * columnas + vc;
				if (idx < 0 || idx >= total) continue;

				auto ep_v = objetos[idx]->getComponente<IVJ::IEstadoEpidemia>();
				if (!ep_v || !ep_v->puedeSerInfectado()) continue;

				// Probabilidad individual del vecino susceptible
				float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				if (r < ep_v->prob_contagio)
				{
					ep_v->infectar();
				}
			}
		}
	}

	bool SistemaEpidemiaTerminada(CE::Pool& pool)
	{
		using Estado = IVJ::IEstadoEpidemia::Estado;
		for (auto& obj : pool.getPool())
		{
			auto ep = obj->getComponente<IVJ::IEstadoEpidemia>();
			if (!ep) continue;
			if (ep->estado_actual == Estado::Enfermo ||
				ep->estado_actual == Estado::Recuperacion)
			{
				return false;  // Todavía hay casos activos
			}
		}
		return true;  // No quedan enfermos ni en recuperación
	}

	void SistemaNPCLookTarget(CE::Objeto& ente, CE::Vector2D& target)
	{
		if (!ente.tieneComponente<IRangoAggro>()) return;

		auto rango = ente.getComponente<IRangoAggro>();
		auto trans = ente.getTransformada();

		// Solo girar si está dentro del rango de aggro
		if (!rango->estaDentroRango(ente, target)) {
			return;
		}

		auto pos = trans->posicion;
		auto ente_a_target = target - pos;

		// Calculamos el ángulo hacia el target
		// Sumamos PI/2 (90°) porque el sprite base mira hacia ARRIBA
		float angulo_radianes = atan2(ente_a_target.y, ente_a_target.x) + (M_PI / 2.0f);

		trans->angulo = angulo_radianes;

		// Opcional: Si usas sprites que requieren flip horizontal en lugar de rotación completa
		// podrías usar el signo de ente_a_target.x, pero para FSM Top-Down la rotación es mejor.
	}

	/*
	 * checkLOS: Verifica si hay línea de visión directa entre p1 y p2, considerando obstáculos.
	 */
	bool checkLOS(const CE::Vector2D& p1, const CE::Vector2D& p2, const std::vector<std::shared_ptr<CE::Objeto>>& obstaculos, CE::Objeto* ignore)
	{
		for (auto& obs : obstaculos)
		{
			if (obs.get() == ignore) continue;
			if (!obs->tieneComponente<CE::IBoundingBox>()) continue;

			auto bb = obs->getComponente<CE::IBoundingBox>();
			auto trans = obs->getTransformada();

			// Definir los límites del AABB (considerando el offset 'mitad')
			float minX = trans->posicion.x - bb->mitad.x;
			float maxX = minX + bb->tam.x;
			float minY = trans->posicion.y - bb->mitad.y;
			float maxY = minY + bb->tam.y;

			// Algoritmo de intersección Rayo-AABB (Slab method simplificado para t=[0,1])
			float dx = p2.x - p1.x;
			float dy = p2.y - p1.y;

			float tmin = 0.f;
			float tmax = 1.f;

			if (std::abs(dx) > 0.0001f) {
				float tx1 = (minX - p1.x) / dx;
				float tx2 = (maxX - p1.x) / dx;
				tmin = std::max(tmin, std::min(tx1, tx2));
				tmax = std::min(tmax, std::max(tx1, tx2));
			} else {
				if (p1.x < minX || p1.x > maxX) continue;
			}

			if (std::abs(dy) > 0.0001f) {
				float ty1 = (minY - p1.y) / dy;
				float ty2 = (maxY - p1.y) / dy;
				tmin = std::max(tmin, std::min(ty1, ty2));
				tmax = std::min(tmax, std::max(ty1, ty2));
			} else {
				if (p1.y < minY || p1.y > maxY) continue;
			}

			if (tmax >= tmin && tmax > 0.f) {
				return false; // Rayo bloqueado
			}
		}
		return true; // Camino despejado
	}

	void SistemaZombieSeguir(CE::Objeto& zombie, CE::Vector2D& target_pos, const std::vector<std::shared_ptr<CE::Objeto>>& obstaculos, float dt)
	{
		auto trans = zombie.getTransformada();
		auto rango = zombie.getComponente<IRangoAggro>();

		if (!rango) return;

		CE::Vector2D pos_actual = trans->posicion;
		float distancia_al_objetivo = pos_actual.distancia(target_pos);

		// 1. Verificación de Rango y Visión
		bool en_rango = rango->estaDentroRango(zombie, target_pos);
		bool tiene_vision = checkLOS(pos_actual, target_pos, obstaculos, &zombie);

		// 3. Decisión de Movimiento
		CE::Vector2D destino = target_pos;
		bool persiguiendo = false;

		if (en_rango && tiene_vision) {
			persiguiendo = true;
			destino = target_pos;
		}

		auto vel = CE::Vector2D{0.f, 0.f};

		if (persiguiendo) {
			if (distancia_al_objetivo < ZombieConst::DISTANCIA_ATAQUE && tiene_vision) {
				// En rango de ataque, quieto
				vel = {0.f, 0.f};
			} else {
				// Perseguir destino (jugador o memoria)
				CE::Vector2D direccion = destino - pos_actual;
				direccion.normalizacion();

				// Evitación de obstáculos (Rayos de proximidad)
				// Lanzamos un rayo corto hacia adelante para detectar si chocará
				CE::Vector2D offset = direccion;
				offset.escala(30.f);
				CE::Vector2D probe = pos_actual + offset;
				
				if (!checkLOS(pos_actual, probe, obstaculos, &zombie)) {
					// Hay una pared enfrente, intentar desviarse (sacarle la vuelta)
					// Probamos 45 grados a la izquierda y derecha
					float angulo = std::atan2(direccion.y, direccion.x);
					
					CE::Vector2D dirIzq{std::cos(angulo - 0.78f), std::sin(angulo - 0.78f)};
					CE::Vector2D dirDer{std::cos(angulo + 0.78f), std::sin(angulo + 0.78f)};

					CE::Vector2D probeIzq = dirIzq; probeIzq.escala(40.f);
					CE::Vector2D probeDer = dirDer; probeDer.escala(40.f);

					if (checkLOS(pos_actual, pos_actual + probeIzq, obstaculos, &zombie)) {
						direccion = dirIzq;
					} else if (checkLOS(pos_actual, pos_actual + probeDer, obstaculos, &zombie)) {
						direccion = dirDer;
					}
				}


				direccion.escala(ZombieConst::VELOCIDAD);
				vel = direccion;
			}
		} else {
			// Fricción si se detiene
			vel = trans->velocidad;
			vel.escala(0.9f);
			if (vel.longitud() < 1.f) vel = {0.f, 0.f};

		}

		trans->velocidad = vel;

		// 4. Rotación (Mirada)
		if (zombie.tieneComponente<CE::ISprite>()) {
			// Si tiene visión directa, SIEMPRE mira al jugador (aunque esté esquivando o quieto)
			if (tiene_vision && en_rango) {
				CE::Vector2D mirar = target_pos - pos_actual;
				mirar.normalizacion();
				trans->angulo = std::atan2(mirar.x, -mirar.y);
			} 
			// Si no tiene visión pero se mueve (hacia memoria o esquivando), mira hacia donde camina
			else if (vel.longitud() > 0.1f) {

				CE::Vector2D caminando = vel;
				caminando.normalizacion();
				trans->angulo = std::atan2(caminando.x, -caminando.y);
			}
		}

		// 5. Actualizar Posición
		trans->posicion.suma(vel.escala(dt));
	}

	void SistemaAtaqueMelee(CE::Objeto& atacante, const std::vector<std::shared_ptr<CE::Objeto>>& posibles_victimas)
	{
	    if (!atacante.tieneComponente<IArmaMelee>()) return;

	    auto arma = atacante.getComponente<IArmaMelee>();

		if (!arma->atacando) {
			arma->objs_golpeados.clear();
			arma->swing_played = false; // listo para el próximo ataque
			return;
		}

		// Sonido de swing: se reproduce una sola vez al comenzar cada ataque
		if (!arma->swing_played) {
			std::string skey = "swing0" + std::to_string((std::rand() % 4) + 1);
			CE::GestorAssets::Get().getSonido(skey).play();
			arma->swing_played = true;
		}

		auto trans = atacante.getTransformada();

	    // 1. Calcular el centro de la hitbox frente al jugador
		// angulo = atan2(dir.x, -dir.y)  →  dir_x = sin(a), dir_y = -cos(a)
		CE::Vector2D centro_hitbox = {
			trans->posicion.x + std::sin(trans->angulo) * arma->offset_adelante,
			trans->posicion.y - std::cos(trans->angulo) * arma->offset_adelante
		};

	    // Actualizar posición gráfica (para debug)
	    arma->hitbox_debug.setPosition({centro_hitbox.x, centro_hitbox.y});

	    bool ha_golpeado = false;

	    // 2. Comprobar colisiones
	    for (const auto& obj_ptr : posibles_victimas) {
	        CE::Objeto& obj = *obj_ptr;

	        // Ignorar a sí mismo y a objetos muertos o sin vida
	        if (&obj == &atacante) continue;
	        if (!obj.tieneComponente<ITarget>()) continue;

	        // Ignorar si ya lo golpeó en este mismo swing
	        if (arma->objs_golpeados.count(&obj)) continue;

	    	CE::Vector2D pos_enemigo = obj.getTransformada()->posicion;

	    	// Teorema de Pitágoras (sin la raíz)
	    	float dx = pos_enemigo.x - centro_hitbox.x;
	    	float dy = pos_enemigo.y - centro_hitbox.y;
	    	float dist_sq = (dx * dx) + (dy * dy);

	    	float radio_enemigo = 20.f;

	    	if (dist_sq < (arma->radio + radio_enemigo) * (arma->radio + radio_enemigo))
	        {
	            // IMPACTO
	            auto stats = obj.getStats(); // o obj.getComponente<IStats>()
	            if (stats->hp > 0)
	            {
	                stats->hp = (stats->hp > arma->damage) ? stats->hp - arma->damage : 0;
	                arma->objs_golpeados.insert(&obj); // Registrar que ya fue golpeado
	                ha_golpeado = true;

	                if (obj.tieneComponente<CE::ITexto>()) {
	                    obj.getComponente<CE::ITexto>()->m_texto.setString("HP: " + std::to_string(stats->hp));
	                }
	            }
	        }
	    }

		// 3. Efectos de Sonido
		if (ha_golpeado) {
			std::string key = "slash0" + std::to_string((std::rand() % 4) + 1);
			CE::GestorAssets::Get().getSonido(key).play();
		}
	}
} // namespace IVJ
