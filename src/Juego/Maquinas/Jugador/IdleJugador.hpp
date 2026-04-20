#pragma once
#include "../FSM.hpp"

namespace IVJ {
    class IdleJugador : public FSM
    {
        public:
            explicit IdleJugador(int max_frames, float frame_rate);
            ~IdleJugador() override {};
            FSM* onInputs(const CE::IControl& control) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj, float dt) override;
        private:
            sf::Sprite* sprite;
            int s_w;
            int s_h;
            int num_cols;       // columnas del atlas
            int id_frame;       // frame absoluto (0..max_frames-1)
            int max_frames;     // 20
            float max_frame_time;
            float cur_frame_time;
    };
}