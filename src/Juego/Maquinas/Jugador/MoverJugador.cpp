#include "MoverJugador.hpp"
#include "IdleJugador.hpp"
namespace IVJ
{
    MoverJugador::MoverJugador(int max_frames,float frame_rate)
        :FSM{},sprite{nullptr},s_w{0},s_h{0},
        max_tiempo{frame_rate},
        act_tiempo{frame_rate},
        max_frames{max_frames}
    {
        nombre = "MoverJugador";
    }
    FSM* MoverJugador::onInputs(const CE::IControl& control)
    {
        //si nada esta presionado regresar a idle
        if(!control.arr && !control.abj && !control.der && !control.izq)
            return new IdleJugador(4.f, .15f);

        return nullptr;
    }
    void MoverJugador::onEntrar(const Entidad& obj)
    {
        sprite = &obj.getComponente<CE::ISprite>()->m_sprite;
        s_w = obj.getComponente<CE::ISprite>()->width;
        s_h = obj.getComponente<CE::ISprite>()->height;
        id_frame=35;
    }
    void MoverJugador::onSalir(const Entidad& obj)
    {
        (void)obj;
    }
    void MoverJugador::onUpdate(const Entidad& obj,float dt)
    {
        (void)obj; //quitar el warning de no usar obj
        act_tiempo = act_tiempo - dt;
        //frame rate
        if (act_tiempo <=0)
        {
            //frame mostrando
            sprite->setTextureRect(
                    sf::IntRect{
                        {//posicion id*width = frame actual
                            s_w*(id_frame%max_frames),0
                        },
                        {//tamaño
                            s_w,s_h
                        }
                    });
            id_frame++;
            act_tiempo=max_tiempo;
        }
    }
}