#include "IdleJugador.hpp"
#include "MoverJugador.hpp"
#include "AttackJugador.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    IdleJugador::IdleJugador(int max_frames, float frame_rate)
        : FSM{}, sprite{nullptr}, s_w{0}, s_h{0}, num_cols{8},
          id_frame{0}, max_frames{max_frames},
          max_frame_time{frame_rate}, cur_frame_time{frame_rate}
    {
        nombre = "IdleJugador";
    }

    FSM* IdleJugador::onInputs(const CE::IControl& control)
    {
        if (control.arr || control.abj || control.der || control.izq)
            return new MoverJugador(20, 0.05f);

        // acc = atacar
        if (control.acc)
            return new AttackJugador(15, 0.05f);

        return nullptr;
    }

    void IdleJugador::onEntrar(const Entidad& obj)
    {
        auto iSprite = obj.getComponente<CE::ISprite>();
        sprite   = &iSprite->m_sprite;
        s_w      = iSprite->width;
        s_h      = iSprite->height;

        auto texSize = iSprite->m_sprite.getTexture().getSize();
        num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : 8;

        id_frame       = 0;
        cur_frame_time = max_frame_time;
    }

    void IdleJugador::onSalir(const Entidad& obj)
    {
        (void)obj;
    }

    void IdleJugador::onUpdate(const Entidad& obj, float dt)
    {
        (void)obj;
        cur_frame_time -= dt;
        if (cur_frame_time > 0) return;

        cur_frame_time = max_frame_time;

        int col = id_frame % num_cols;
        int row = id_frame / num_cols;

        sprite->setTextureRect(sf::IntRect{
            {col * s_w, row * s_h},
            {s_w, s_h}
        });

        id_frame = (id_frame + 1) % max_frames;
    }
}