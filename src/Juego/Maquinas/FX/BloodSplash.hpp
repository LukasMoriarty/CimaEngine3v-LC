#pragma once

#include "../../Componentes/IJComponentes.hpp"
#include <Motor/Utils/AnimLoader.hpp>
#include "../FSM.hpp"

namespace IVJ {
    class BloodSplash : public FSM {
    public:
        BloodSplash(CE::AnimClip clip, float frame_rate);
        ~BloodSplash() override = default;

        FSM* onInputs(const CE::IControl& control) override { (void)control; return nullptr; }
        void onEntrar(const Entidad& obj) override;
        void onSalir(const Entidad& obj) override;
        void onUpdate(const Entidad& obj, float dt) override;

    private:
        CE::AnimClip m_clip;
        float max_frame_time;
        float cur_frame_time;
        int id_frame{0};
        int local_frame{0};
        int num_cols{1};
        int s_w{0}, s_h{0};
        sf::Sprite* sprite{nullptr};
        bool finished{false};
    };
}

