#include "Figuras.hpp"

namespace IVJ {
    Figuras::Figuras(int l, const sf::Color& relleno, const sf::Color& contorno)
        :CE::Objeto{},f_lados{l},f_crelleno{relleno},f_ccontorno{contorno}
    {
    }


    Rectangulo::Rectangulo(float ancho, float largo, const sf::Color& relleno, const sf::Color& contorno)
    :Figuras{4,relleno,contorno}, rect_img{sf::RectangleShape({ancho,largo})},w{ancho},h{largo}
    {
        rect_img.setFillColor(f_crelleno);
        rect_img.setOutlineColor(f_ccontorno);
        rect_img.setOutlineThickness(3);
        rect_img.setOrigin({ancho/2.f,largo/2.f});
    }


    void Rectangulo::draw(sf::RenderTarget& target, sf::RenderStates state) const 
    {
        state.transform *= getTransform();
        target.draw(rect_img);
        sf::CircleShape pivote{10.f};
        pivote.setFillColor(f_ccontorno);
        pivote.setOrigin({10.f,10.f});
        pivote.setPosition(rect_img.getPosition());

        target.draw(pivote);

    }
    void Rectangulo::onUpdate(float dt)
    {
        rect_img.setPosition({transform->posicion.x,transform->posicion.y});
        (void)dt;
    }

    Circulo::Circulo(float radio, const sf::Color &relleno, const sf::Color &contorno)
        :Figuras{32, relleno, contorno}, circ_img{radio, 32}, radio{radio}
    {
        circ_img.setFillColor(f_crelleno);
        circ_img.setOutlineColor(f_ccontorno);
        circ_img.setOutlineThickness(3);
        circ_img.setOrigin({radio, radio});
    }

    void Circulo::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        state.transform *= getTransform();
        target.draw(circ_img);
        sf::CircleShape pivote{10.f};
        pivote.setFillColor(f_ccontorno);
        pivote.setOrigin({10.f,10.f});
        pivote.setPosition(circ_img.getPosition());

        target.draw(pivote);
    }

    void Circulo::onUpdate(float dt)
    {
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        (void)dt;
    }

    Triangulo::Triangulo(float lado, const sf::Color &relleno, const sf::Color &contorno)
        :Figuras{3, relleno, contorno}, circ_img(lado, 3), lado{lado}
    {
        circ_img.setFillColor(f_crelleno);
        circ_img.setOutlineColor(f_ccontorno);
        circ_img.setOutlineThickness(3);
        circ_img.setOrigin({lado, lado});
    }

    void Triangulo::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        state.transform *= getTransform();
        target.draw(circ_img);
        sf::CircleShape pivote{10.f};
        pivote.setFillColor(f_ccontorno);
        pivote.setOrigin({10.f,10.f});
        pivote.setPosition(circ_img.getPosition());

        target.draw(pivote);
    }

    void Triangulo::onUpdate(float dt)
    {
        circ_img.setPosition({transform->posicion.x,transform->posicion.y});
        (void)dt;
    }
}