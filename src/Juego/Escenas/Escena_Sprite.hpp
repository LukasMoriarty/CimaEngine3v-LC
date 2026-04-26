#pragma once
#include <Motor/Primitivos/Escena.hpp>
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <memory>

namespace IVJ
{
	class Escena_Sprite : public CE::Escena
	{
		public:
			explicit Escena_Sprite(std::shared_ptr<Entidad>& pref);
			virtual ~Escena_Sprite(){};
			void onInit() override;
			void onFinal() override;
			void onUpdate(float dt) override;
			void onInputs(const CE::Botones& accion) override;
			void onRender() override;
			std::shared_ptr<Entidad> getJugador() override { return jugador_ref; }
		private:
			int inicializar{1};

			/// Pies del jugador — tiene el sprite atlas de pies y su propio FSM.
			std::shared_ptr<Entidad>& jugador_ref;

			/// Cuerpo del jugador — tiene el sprite atlas de knife y su propio FSM.
			/// Siempre se posiciona encima de jugador_ref (misma posición).
			std::shared_ptr<Entidad> cuerpo;
	};
}