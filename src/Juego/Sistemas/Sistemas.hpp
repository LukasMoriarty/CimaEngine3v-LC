#pragma once
#include "../../Motor/Primitivos/Objetos.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../Componentes/IJComponentes.hpp"
#include "../../Motor/Primitivos/CEPool.hpp"
#include <Motor/Utils/AnimLoader.hpp>
#

namespace IVJ
{
    [[maybe_unused]]void SistemaControl(CE::Objeto&ente, float dt);
    [[maybe_unused]]void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt);
    [[maybe_unused]]void SistemaMover(const std::shared_ptr<CE::Objeto>& objeto, float dt);
    [[maybe_unused]]bool SistemaColAABB(CE::Objeto& A,CE::Objeto& B, bool resolucion =false);
    [[maybe_unused]]bool SistemaColAABBMid(CE::Objeto& A, CE::Objeto& B, bool resolucion =false);

    [[maybe_unused]]void SistemaSpawn(CE::Pool& game_pool, CE::Pool& spawn_zones, const std::map<std::string, CE::AnimClip>& zombie_clips, CE::Objeto* player_target, int& zombies_por_generar);
    [[maybe_unused]]void SistemaPaths(std::vector<std::shared_ptr<CE::Objeto>>& obj);
    [[maybe_unused]]bool checkRayoHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2);

    [[maybe_unused]]void SistemaDialogos(IVJ::IDialogo* dialogo, CE::Objeto& obj);

    bool revisarDistanciaInteraccion(CE::Objeto& jugador, CE::Objeto& npc, float distancia_maxima);

    //DEBUG
    [[maybe_unused]]void pintarLinea(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color=sf::Color::Red);

    [[maybe_unused]] void SistemaGirar(CE::Objeto &ente, float dt);

    // lab 2 y 3
    [[maybe_unused]] void SistemaRectanguloMoverUpDown(CE::Objeto &ente, float dt);
    [[maybe_unused]] void SistemaTrianguloWave(CE::Objeto &ente, float dt);
    [[maybe_unused]] void SistemaCirculoMoverEnCirculos(CE::Objeto &ente, float dt);

    // lab 8
    [[maybe_unused]] void SistemaNPCLookTarget(CE::Objeto& ente, CE::Vector2D& target);

    // zombie
    [[maybe_unused]] bool checkLOS(const CE::Vector2D& p1, const CE::Vector2D& p2, const std::vector<std::shared_ptr<CE::Objeto>>& obstaculos, CE::Objeto* ignore = nullptr);


    [[maybe_unused]] void SistemaZombieSeguir(CE::Objeto& zombie, CE::Vector2D& target, const std::vector<std::shared_ptr<CE::Objeto>>& obstaculos, float dt);

    [[maybe_unused]] void SistemaAtaqueMelee(CE::Objeto& atacante, CE::Pool& game_pool, int& puntaje, int& zombies_matados);

	[[maybe_unused]] void SistemaDisparar(
		CE::Objeto& tirador,	// jugador (posicion fuente)
		CE::Objeto& cuerpo,		// cuerpo (angulo/direccion)
		IPistola& arma,
		CE::Pool& game_pool,
		const sf::Texture& bala_tex
	);
	/*
	 * Mueve todas las Entidades con IBala en game_pool
	 * Marca activa=false (y hp=0) cuando distancia_viajada >= rango_max
	 */
	[[maybe_unused]] void SistemaActualizarBalas(CE::Pool& game_pool, float dt);
	/*
	 * Detecta colision bala vs cualquier entidad con ITarget (es decir, las que targetean al jugador)
	 * Al impactar: aplica daño, genera blood splash, desactiva bala (hp=0)
	 */
	[[maybe_unused]] void SistemaColBalas(CE::Pool& game_pool, int& puntaje, int& zombies_matados);
}