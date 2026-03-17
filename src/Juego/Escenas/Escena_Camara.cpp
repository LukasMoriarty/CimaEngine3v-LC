#include "Escena_Camara.hpp"
#include <Motor/Inputs/Botones.hpp>
#include <Motor/Render/Render.hpp>
#include <memory>

#include "Juego/objetos/Entidad.hpp"
#include "Juego/Sistemas/Sistemas.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"

namespace IVJ
{
    Escena_Camara::Escena_Camara(std::shared_ptr<Entidad>& pref)
        :CE::Escena{},jugador_ref{pref}
    {
    }
    void Escena_Camara::onInit()
    {
        // asignar primero que nada la camara, porque podemos entrar y salir de escenas
        // asi la camara regresa a la que deberia en la escena
        // id = 1 = LERP es en el orden que se agreguen
        CE::GestorCamaras::Get().setCamaraActiva(1);
        // indicar a quien persigue
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador_ref);

        if(!inicializar) return;

        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::A,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::Left,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::D,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Right,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Enter,"aceptar");

        auto trans = jugador_ref->getTransformada();
        trans->velocidad = CE::Vector2D{120.f, 120.f};
        jugador_ref->setPosicion(300.f, 300.f);

        auto triangulo = std::make_shared<ITriangulo>(40.f, 0.f);
        triangulo->tri_shape.setFillColor(sf::Color{255,191,0});
        triangulo->tri_shape.setOutlineColor(sf::Color::Black);
        triangulo->tri_shape.setOutlineThickness(3.f);

        jugador_ref->addComponente(triangulo);
        jugador_ref->addComponente(std::make_shared<CE::IControl>());

        inicializar=false;
    }
    void Escena_Camara::onFinal()
    {
        // resetear la camara a la estatica al salir/cambiar de escena
        CE::GestorCamaras::Get().setCamaraActiva(0);
    }
    void Escena_Camara::onUpdate(float dt)
    {
        jugador_ref->onUpdate(dt);
        SistemaMover(jugador_ref, dt);
        for(auto& obj: objetos.getPool())
        {
            obj->onUpdate(dt);
        }
    }
    void Escena_Camara::onInputs(const CE::Botones& accion)
    {
        switch(accion.getTipo())
        {
            case CE::Botones::TipoAccion::OnPress:
            {
                if (accion.getNombre() == "arriba")
                    jugador_ref->getComponente<CE::IControl>()->arr = true;
                if (accion.getNombre() == "abajo")
                    jugador_ref->getComponente<CE::IControl>()->abj = true;
                if (accion.getNombre() == "derecha")
                    jugador_ref->getComponente<CE::IControl>()->der = true;
                if (accion.getNombre() == "izquierda")
                    jugador_ref->getComponente<CE::IControl>()->izq = true;
                break;
            }
            case CE::Botones::TipoAccion::OnRelease:
            {
                if (accion.getNombre() == "arriba")
                    jugador_ref->getComponente<CE::IControl>()->arr = false;
                if (accion.getNombre() == "abajo")
                    jugador_ref->getComponente<CE::IControl>()->abj = false;
                if (accion.getNombre() == "derecha")
                    jugador_ref->getComponente<CE::IControl>()->der = false;
                if (accion.getNombre() == "izquierda")
                    jugador_ref->getComponente<CE::IControl>()->izq = false;
                break;
            }
            case CE::Botones::TipoAccion::None:
            {
                break;
            }
        }
    }
    void Escena_Camara::onRender()
    {
        CE::Render::Get().AddToDraw(*jugador_ref);
        for(auto& obj: objetos.getPool())
            CE::Render::Get().AddToDraw(*obj);
    }
}
