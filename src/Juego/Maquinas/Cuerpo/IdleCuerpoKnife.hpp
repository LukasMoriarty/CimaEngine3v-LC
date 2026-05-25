#pragma once
#include "Motor/Componentes/IComponentes.hpp"
#include "../../Componentes/IJComponentes.hpp"
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
    /**
     * @class IdleCuerpoKnife
     * @brief Estado idle del cuerpo con cuchillo (frames 0-19 del atlas knife).
     *
     * Transiciones:
     *  - control.acc              → AtacarCuerpoKnife
     *  - movimiento (arr/abj...)  → MoverCuerpoKnife
     */
    class IdleCuerpoKnife : public FSM
    {
    public:
        explicit IdleCuerpoKnife(CE::AnimClip clip, float frame_rate = 0.1f);
        ~IdleCuerpoKnife() override = default;

        FSM* onInputs(const CE::IControl& control) override;
        void onEntrar(const Entidad& obj)          override;
        void onSalir(const Entidad& obj)           override;
        void onUpdate(const Entidad& obj, float dt) override;

    private:
        sf::Sprite* sprite{nullptr};
        int s_w{0};
        int s_h{0};
        int num_cols{8};        ///< Columnas del atlas knife (3072/384 = 8)

        IVJ::IArmaMelee* mi_arma{nullptr};
        CE::AnimClip m_clip;
        int id_frame{0};

        float max_frame_time;
        float cur_frame_time;
    };
}