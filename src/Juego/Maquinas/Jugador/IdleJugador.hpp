#pragma once
#include "../FSM.hpp"

namespace IVJ {
    class IdleJugador : public FSM
    {
        public:
            IdleJugador(int max_frames, float frame_rate);
            ~IdleJugador() override{};
            FSM* onInputs(const CE::IControl& control)override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj, float dt) override;
        private:
            sf::Sprite *sprite;
            int s_w;    // sprite width
            int s_h;    // sprite height
            int id_frame;   // frame actual de animacion
            float max_frame_time;   // frame rate
            float cur_frame_time;   // frame rate actual
            int max_frames; // frames de animacion
    };
}