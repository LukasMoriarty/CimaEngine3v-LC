#include "AttackJugador.hpp"
#include "IdleJugador.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    AttackJugador::AttackJugador(int max_frames, float frame_rate)
        : FSM{}, sprite{nullptr}, s_w{0}, s_h{0}, num_cols{8},
          id_frame{20}, first_frame{20},
          max_frames{max_frames},
          max_frame_time{frame_rate},
          cur_frame_time{frame_rate},
          ataque_completo{false}
    {
        nombre = "AttackJugador";
    }

    FSM* AttackJugador::onInputs(const CE::IControl& control)
    {
        (void)control;
        // Al terminar el ciclo de ataque volvemos a idle
        if (ataque_completo)
            return new IdleJugador(1, 0.05f);
        return nullptr;
    }

    void AttackJugador::onEntrar(const Entidad& obj)
    {
        auto iSprite = obj.getComponente<CE::ISprite>();
        sprite   = &iSprite->m_sprite;
        s_w      = iSprite->width;
        s_h      = iSprite->height;

        // calcular columnas del atlas en tiempo de ejecución
        auto texSize = iSprite->m_sprite.getTexture().getSize();
        num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : 8;

        id_frame       = first_frame;
        cur_frame_time = max_frame_time;
        ataque_completo = false;
    }

    void AttackJugador::onSalir(const Entidad& obj)
    {
        (void)obj;
    }

    void AttackJugador::onUpdate(const Entidad& obj, float dt)
    {
        (void)obj;
        cur_frame_time -= dt;
        if (cur_frame_time > 0) return;

        cur_frame_time = max_frame_time;

        int local = id_frame - first_frame;
        int col = id_frame % num_cols;
        int row = id_frame / num_cols;

        sprite->setTextureRect(sf::IntRect{
            {col * s_w, row * s_h},
            {s_w, s_h}
        });

        local++;
        if (local >= max_frames)
        {
            // ciclo completo: se queda en el último frame hasta que onInputs lo detecte
            ataque_completo = true;
            id_frame = first_frame; // reinicia para próxima vez
        }
        else
        {
            id_frame = first_frame + local;
        }
    }
}