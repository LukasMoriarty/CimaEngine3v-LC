#include "Entidad.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include <memory>

#include "../Maquinas/FSMBoss.hpp"
#include "../Maquinas/FSMZombie.hpp"

namespace IVJ
{
	static int num_cpy = 0;
	Entidad::Entidad(const Entidad &cpy)
		: CE::Objeto{cpy}
	{
		nombre->nombre = cpy.getNombre()->nombre + "_" + std::to_string(++num_cpy);
		// copia profunda de los componentes
		// ya que no queremos la referencia sino un componente nuevo
		for (auto &comp : cpy.componentes)
		{
			componentes.push_back(comp->clonar());
		}
	}

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
		//	auto sprite = getComponente<CE::ISprite>();
		//	auto bound = getComponente<CE::IBoundingBox>();
		//	//sprite->m_sprite.setOrigin({bound->mitad.x,bound->mitad.y});
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

		// actualizar el angulo y posicion del sprite
		if (tieneComponente<CE::ISprite>())
		{
			auto sprite = getComponente<CE::ISprite>();
			auto pos = getTransformada()->posicion;
			sprite->m_sprite.setPosition({pos.x,pos.y});
			auto angulo = sf::radians(getTransformada()->angulo);
			sprite->m_sprite.setRotation(angulo);
		}

		transform->pos_prev =  transform->posicion;

	}
	void Entidad::inputFSM()
	{
		if(!getComponente<IMaquinaEstado>()
		|| !getComponente<IMaquinaEstado>()->fsm)
			return;
		auto mq = getComponente<IMaquinaEstado>();

		// Detectar si es Boss o Zombie usando el nombre del FSM
		if(mq->fsm->nombre.find("Boss") != std::string::npos)
		{
			auto target = getComponente<ITarget>()->pos;
			auto n_estado = std::static_pointer_cast<FSMBoss>(mq->fsm)->onInputs(*this, *target);
			std::shared_ptr<FSM> estado(n_estado);
			if(estado)
				setFSM(estado);
		}
		else if(mq->fsm->nombre.find("Zombie") != std::string::npos)
		{
			auto target = getComponente<ITarget>()->pos;
			auto n_estado = std::static_pointer_cast<FSMZombie>(mq->fsm)->onInputs(*this, *target);
			std::shared_ptr<FSM> estado(n_estado);
			if(estado)
				setFSM(estado);
		}
		else
		{
			if(!getComponente<CE::IControl>())  return;
			auto control = *getComponente<CE::IControl>();

			// Intentar llamar a onInputs con información del mouse si está disponible
			// (mouse_dir_x/y fueron establecidos previamente con setMouseDirection)
			auto n_estado = mq->fsm->onInputs(control, mouse_dir_x, mouse_dir_y);
			std::shared_ptr<FSM> estado(n_estado);
			if(estado)
				setFSM(estado);
		}
	}

	void Entidad::setFSM(const std::shared_ptr<FSM>& mq)
	{
		// cuando llamamos setFSM con make_shared como parámetro
		// hay que congelar para que no siempre lo cambie
		if (getComponente<IMaquinaEstado>()->congelar)
			return;
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
