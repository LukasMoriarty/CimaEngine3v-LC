#pragma once
#include "../FSM.hpp"

namespace IVJ
{
    // Frames 35-54 (20 frames), filas 4-6
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
        int s_w;
        int s_h;
        int num_cols;
        int id_frame;       // frame absoluto (empieza en 35)
        int first_frame;    // 35
        int max_frames;     // 20
        float max_frame_time;
        float cur_frame_time;
    };
}