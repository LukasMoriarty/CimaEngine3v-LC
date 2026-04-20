#include "IdleJugador.hpp"
#include "Motor/Componentes/IComponentes.hpp"
#include "AttackJugador.hpp"
#include "MoverJugador.hpp"

namespace IVJ {
    IdleJugador::IdleJugador(int max_frames, float frame_rate)
        : FSM{}, sprite{nullptr}, s_w{0}, s_h{0}, id_frame{0},
        max_frame_time{frame_rate},
        cur_frame_time{frame_rate},
        max_frames{max_frames}
    {
        nombre = "IdleJugador";
    }
    FSM* IdleJugador::onInputs(const CE::IControl& control)
    {
        if (control.arr || control.abj || control.der || control.izq) {
            std::cout << "Cambiar a MoverJugador\n";
            return new MoverJugador(4, 0.15f);
        }
        // todo: implementar ataque
        /*
        if (control.attack) {
            std::cout << "Cambiar a AttackJugador\n";
            return new AttackJugador(4, 0.1f);
         */
        return nullptr;
    }

    void IdleJugador::onEntrar(const Entidad& obj) {
        sprite = &obj.getComponente<CE::ISprite>()->m_sprite;
        s_w = obj.getComponente<CE::ISprite>()->width;
        s_h = obj.getComponente<CE::ISprite>()->height;
        id_frame = 0;
    }

    void IdleJugador::onSalir(const Entidad& obj) {
        (void)obj; // evitar warning de variable no usada
    }

    void IdleJugador::onUpdate(const Entidad& obj, float dt) {
        (void)obj; // evitar warning de variable no usada
        cur_frame_time -= dt;
        if (cur_frame_time <= 0) {
            sprite->setTextureRect(
                sf::IntRect{
                    {s_w * (id_frame % max_frames), 0},
                    {s_w, s_h}
                }
            );
            id_frame++;
            cur_frame_time = max_frame_time;
        }
    }
}
