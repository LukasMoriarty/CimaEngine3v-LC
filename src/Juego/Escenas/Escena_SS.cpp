#include "Escena_SS.hpp"
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/GUI/GLogger.hpp>
#include <fstream>
#include <iostream>
#include <Juego/Sistemas/Sistemas.hpp>

namespace IVJ
{
    Escena_SS::Escena_SS(std::shared_ptr<Entidad>& pref)
        : CE::Escena{}, jugador_ref{pref}
    {
    }
            
    void Escena_SS::onInit()
    {
        if(!inicializar)
            return;

        /*
        //Inicializar todo lo que contendra la escena 
        auto rect = std::make_shared<Rectangulo>(100.f,20.f,sf::Color::Red,sf::Color::Black);
        rect->setPosicion(200.f,300.f);

        auto texto = std::make_shared<IVJ::Texto>(CE::GestorAssets::Get().getFont("default_font"),"Escena de Prueba");
        texto->setFontSize(30u);

        objetos.agregarPool(rect);
        objetos.agregarPool(texto);
        */

        auto ptr_int = std::make_shared<int>();
        *ptr_int = 10;

        std::cout<<ptr_int<<": "<< *ptr_int <<"\n";

        figura = std::make_shared<Rectangulo>(
            100.f, 50.f, sf::Color::Red, sf::Color::Yellow);

        figura->setPosicion(350.f, 400.f);

        objetos.agregarPool(figura);

        figura2 = std::make_shared<Circulo>(
            50.f, sf::Color::Green, sf::Color::Blue);

        figura2->setPosicion(250.f, 200.f);

        objetos.agregarPool(figura2);

        std::ifstream miarchivo(ASSETS "/config/figuras.txt");
        std::string tipo;
        float px, py;
        float w, h;
        int fr,fg,fb, cr, cg, cb;

        while (miarchivo >> tipo)
        {

            if (tipo == "Rectangulo")
            {
                miarchivo >> px >> py >> w >> h >> fr >> fg >> fb >> cr >> cg >> cb;
                auto nuevaFigura = std::make_shared<Rectangulo>(w, h,
                    sf::Color(fr,fg,fb,255),
                    sf::Color(cr,cg,cb,255));
                nuevaFigura->setPosicion(px,py);
                nuevaFigura->addComponente(std::make_shared<IArribaAbajo>(py-50.f, py+50.f, 50.f));
                objetos.agregarPool(nuevaFigura);
            }
            else if (tipo == "Circulo")
            {
                miarchivo >> px >> py >> w >> fr >> fg >> fb >> cr >> cg >> cb;
                auto nuevaFigura = std::make_shared<Circulo>(w,
                    sf::Color(fr,fg,fb,255),
                    sf::Color(cr,cg,cb,255));
                nuevaFigura->setPosicion(px,py);
                nuevaFigura->addComponente(std::make_shared<ICirculos>(2.f, 1.f));
                objetos.agregarPool(nuevaFigura);
            }
            else if (tipo == "Triangulo")
            {
                miarchivo >> px >> py >> w >> fr >> fg >> fb >> cr >> cg >> cb;
                auto nuevaFigura = std::make_shared<Triangulo>(w,
                    sf::Color(fr,fg,fb,255),
                    sf::Color(cr,cg,cb,255));
                nuevaFigura->setPosicion(px,py);
                nuevaFigura->addComponente(std::make_shared<IWave>(5.f, 1.f, 1.f));
                objetos.agregarPool(nuevaFigura);
            }
            else
            {
                continue;
            }

        }

        inicializar = 0;
    }
    void Escena_SS::onFinal()
    {
    
    }
    void Escena_SS::onUpdate(float dt)
    {
        for(auto& obj: objetos.getPool())
        {
            obj->onUpdate(dt);
            SistemaRectanguloMoverUpDown(*obj, dt);
            SistemaTrianguloWave(*obj, dt);
            SistemaCirculoMoverEnCirculos(*obj, dt);
        }
        static float timer{0.f};
        timer+=dt;
        CE::GLogger::Get().agregarLog("Timer:"+std::to_string(timer),CE::GLogger::Niveles::LOG_DEBUG);
    }
    void Escena_SS::onInputs(const CE::Botones& accion)
    {
        (void)accion;
    }
    void Escena_SS::onRender()
    {
        for(auto& obj: objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
    }
}
