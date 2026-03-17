#pragma once
#include "../../Motor/Primitivos/Objetos.hpp"
#include <SFML/Graphics.hpp>

namespace IVJ
{
    class Figuras:public CE::Objeto
    {
    public:
        explicit Figuras(int l, const sf::Color& relleno, const sf::Color& contorno);
        ~Figuras() override{};
        friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Figuras>&fig)
        {
            os<<"Lados: "<<fig->f_lados<<
            "\nRelleno: "<<fig->f_crelleno.toInteger()<<
            "\nContorno:"<<fig->f_ccontorno.toInteger()<<
            "\n";
            return os;
        }
    protected:
        int f_lados;
        sf::Color f_crelleno;
        sf::Color f_ccontorno;
    };


    class Rectangulo: public Figuras
    {
        public: 
            explicit Rectangulo(float ancho, float largo,const sf::Color& relleno,const  sf::Color& contorno);
            ~Rectangulo() override {};
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            void onUpdate(float dt) override;

            /**
             * @brief Actualiza colores de relleno y contorno en tiempo real.
             * Necesario para que Persona pueda cambiar de color según su estado epidémico.
             */
            void setColores(const sf::Color& relleno, const sf::Color& contorno)
            {
                f_crelleno  = relleno;
                f_ccontorno = contorno;
                rect_img.setFillColor(f_crelleno);
                rect_img.setOutlineColor(f_ccontorno);
            }

        private:
            sf::RectangleShape rect_img;
        public:
            float w;
            float h;
    };

    class Circulo: public Figuras
    {
        public:
            explicit Circulo(float radio,const sf::Color& relleno,const  sf::Color& contorno);
            ~Circulo() override {};
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            void onUpdate(float dt) override;
        private:
            sf::CircleShape circ_img;
        public:
            float radio;
    };

    class Triangulo: public Figuras {
        public:
            explicit Triangulo(float lado,const sf::Color& relleno,const  sf::Color& contorno);
            ~Triangulo() override {};
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            void onUpdate(float dt) override;
        private:
            sf::CircleShape circ_img;
        public:
            float lado;
    };
}