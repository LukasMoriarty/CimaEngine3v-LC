#include "Objetos.hpp"
#include <iostream>
namespace CE
{
    int Objeto::num_objetos = 0;
    Objeto::Objeto()
    {
        nombre    = std::make_shared<INombre>("Objeto "+std::to_string(Objeto::num_objetos+1));
        transform = std::make_shared<ITransform>();
        stats = std::make_shared<IStats>();
        Objeto::num_objetos++;
    }
	Objeto::Objeto(const Objeto &cpy)
	{
		// se cre espacio de memoria y se copia el contenido
    	nombre = std::make_shared<INombre>(*cpy.nombre);
    	transform = std::make_shared<ITransform>(*cpy.transform);
    	stats = std::make_shared<IStats>(*cpy.stats);
    }
    void Objeto::setPosicion(float x, float y)
    {
        transform->pos_prev = transform->posicion;
        transform->posicion.x = x;
        transform->posicion.y = y;
    }

    void Objeto::centrarObjeto(float w, float h) {
        transform->pos_prev = transform->posicion;
        transform->posicion.x = w / 2.0f;
        transform->posicion.y = h / 2.0f;
    }


    Objeto& Objeto::addComponente(const std::shared_ptr<IComponentes>& componente)
    {
        componentes.push_back(componente);
        return *this;
    }

    void Objeto::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        std::cout<<"Objeto::Draw()\n";
        (void)target; (void)state; //quitar warnings
    }
}
