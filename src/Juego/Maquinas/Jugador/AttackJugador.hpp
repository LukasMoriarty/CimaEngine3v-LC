#pragma once
#include "../FSM.hpp"

namespace IVJ
{
    // Frames 20-34 (15 frames), fila 2 col 4 hasta fila 4 col 2
    class AttackJugador : public FSM
    {
        public:
            explicit AttackJugador(int max_frames, float frame_rate);
            ~AttackJugador() override {};
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj, float dt) override;
        private:
            sf::Sprite* sprite;
            int s_w;
            int s_h;
            int num_cols;       // columnas del atlas = tex_w / s_w
            int id_frame;       // frame absoluto actual (empieza en first_frame)
            int first_frame;    // 20
            int max_frames;     // 15
            float max_frame_time;
            float cur_frame_time;
            bool ataque_completo; // true cuando termina el ciclo
    };
}