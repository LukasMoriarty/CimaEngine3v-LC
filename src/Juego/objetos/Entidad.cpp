#include "Entidad.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include <memory>

namespace IVJ
{
    void Entidad::onUpdate(float dt) 
    {
        //CE::Objeto::onUpdate(dt);

        //revisar si tiene ciertos componentes
        //para actualizarlos debidamente
        //o hacer una clase hija y ahí revisar
        
        if(tieneComponente<CE::ISprite>())
        {
            auto sprite = getComponente<CE::ISprite>();
            auto pos = getTransformada()->posicion;
            sprite->m_sprite.setPosition({pos.x,pos.y});
        }
        //if(tieneComponente<CE::ISprite>() && tieneComponente<CE::IBoundingBox>())
        //{
        //    auto sprite = getComponente<CE::ISprite>();
        //    auto bound = getComponente<CE::IBoundingBox>();
        //    //sprite->m_sprite.setOrigin({bound->mitad.x,bound->mitad.y});
        //}

        // si tiene un FSM actualizar el estado
        if(tieneComponente<IVJ::IMaquinaEstado>())
        {
            auto mq = getComponente<IVJ::IMaquinaEstado>();
            if(mq->fsm)
                mq->fsm->onUpdate(*this,dt);
        }

        if (tieneComponente<ITriangulo>()) {
            auto fig = getComponente<ITriangulo>();
            auto pos = getTransformada()->posicion;
            // dibujar triangulo en posicion correcta
            fig->tri_shape.setPosition({pos.x,pos.y});
            // aplicar rotacion dependiento de la direccion
            auto r = sf::radians(fig->angulo);
            fig->tri_shape.setRotation(r);
        }


        transform->pos_prev =  transform->posicion;

    }
    void Entidad::inputFSM()
    {
        if(!getComponente<IMaquinaEstado>() || !getComponente<CE::IControl>() ||
           !getComponente<IMaquinaEstado>()->fsm)
            return;
        auto mq = getComponente<IMaquinaEstado>();
        //crear el nuevo estado , si es nulo no hay nuevo estado aún
        auto control = *getComponente<CE::IControl>();
        auto n_estado = mq->fsm->onInputs(control);
        std::shared_ptr<FSM> estado(n_estado);
        if(estado)
            setFSM(estado);
    }
    
    void Entidad::setFSM(const std::shared_ptr<FSM>& mq)
    {
        auto &estado_actual = getComponente<IMaquinaEstado>()->fsm;
        //transición de salida
        estado_actual->onSalir(*this);
        //transición de entrar a otro estado
        estado_actual = mq;
        estado_actual->onEntrar(*this);
    }


    void Entidad::draw(sf::RenderTarget &target, sf::RenderStates state) const
    {
        state.transform *= getTransform();
#if DEBUG
        if(tieneComponente<CE::IBoundingBox>())
        {
            auto escala = 1;//getComponente<CE::ISprite>()->escala;
            auto bound = getComponente<CE::IBoundingBox>();
            sf::RectangleShape box{{bound->tam.x*escala,bound->tam.y*escala}};
            box.setFillColor(sf::Color::Transparent);
            box.setOutlineColor(sf::Color::Red);
            box.setOutlineThickness(1.5f);
            box.setOrigin({bound->mitad.x*escala,bound->mitad.y*escala});
            auto pos = transform->posicion;
            box.setPosition({pos.x,pos.y});
            target.draw(box);
        }
#endif
        //revisar si tiene ciertos componentes
        //para renderizarlos debidamente
        //o hacer una clase hija y ahí revisar
        if(tieneComponente<CE::ISprite>())
        {
            auto sprite = getComponente<CE::ISprite>();
            //SHADER  NO ME GUSTA ES LENTO
            if(tieneComponente<CE::IShader>())
            {
                target.draw(sprite->m_sprite,&getComponente<CE::IShader>()->m_shader);
            }
            else
                target.draw(sprite->m_sprite);
        }

        if (tieneComponente<ITriangulo>()) {
            auto fig = getComponente<ITriangulo>();
            target.draw(fig->tri_shape);
        }
    }
}
