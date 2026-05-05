#pragma once
#include "FSM.hpp"
#include "../objetos/Entidad.hpp"

namespace IVJ
{
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