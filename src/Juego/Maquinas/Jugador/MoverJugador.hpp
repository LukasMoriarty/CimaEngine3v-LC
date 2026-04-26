#pragma once
#include "../FSM.hpp"

namespace IVJ
{
	class MoverJugador : public FSM
	{
		public:
			explicit MoverJugador(int max_frames, float frame_rate);
			~MoverJugador() override {};
			FSM* onInputs(const CE::IControl& control) override;
			void onEntrar(const Entidad& obj) override;
			void onSalir(const Entidad& obj) override;
			void onUpdate(const Entidad& obj, float dt) override;
		private:
			sf::Sprite* sprite;
			int frameWidth;		// esto es el width del frame, no del atlas
			int frameHeight;	// esto es el height del frame, no del atlas
			int num_cols;		// numero de columnas del atlas = tex_w / s_w
			int id_frame;		// indica el frame actual a animar
			int first_frame;	// indica el indice del primer frame a animar
			int max_frames;
			float max_frame_time;
			float cur_frame_time;
	};
}
