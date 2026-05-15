#pragma once
#include "FSM.hpp"
#include "../objetos/Entidad.hpp"

namespace IVJ
{
    // ------------------- Shared Zombie Constants -------------------
    struct ZombieConst
    {
        static constexpr float RANGO_AGGRO = 250.f;          // detection range
        static constexpr float VELOCIDAD = 150.f;          // movement speed
        static constexpr float DISTANCIA_ATAQUE = 45.f;     // distance to stop and attack
        static constexpr float DISTANCIA_IDLE = 250.f;      // distance to return to idle
        static constexpr float FRAME_RATE_ATTACK = 0.08f;   // attack animation frame time
        static constexpr float FRAME_RATE_MOVE = 0.04f;     // move/idle animation frame time
    };

	/**

	 * @class FSMZombie
	 * @brief Clase base para máquinas de estado de zombies.
	 *
	 * Análoga a FSMBoss: los zombies no reciben IControl del jugador,
	 * sino que reaccionan a la posición del target (jugador).
	 *
	 * Entidad::inputFSM detecta el nombre "Zombie" y llama a
	 * onInputs(Entidad&, Vector2D&) en lugar del onInputs de IControl.
	 */
	class FSMZombie : public FSM
	{
	public:
		virtual ~FSMZombie() = default;

		// Ignorado — zombies no usan IControl
		virtual FSM* onInputs(const CE::IControl& control) override
		{ (void)control; return nullptr; }

		virtual FSM* onInputs(Entidad& self, CE::Vector2D& target) = 0;

		virtual void onEntrar(const Entidad& obj) override = 0;
		virtual void onSalir(const Entidad& obj) override = 0;
		virtual void onUpdate(const Entidad& obj, float dt) override = 0;
	};
}