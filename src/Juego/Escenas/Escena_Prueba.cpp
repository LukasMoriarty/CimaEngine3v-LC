#include "Escena_Prueba.hpp"
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/GUI/GLogger.hpp>

namespace IVJ
{
    Escena_Prueba::Escena_Prueba(std::shared_ptr<Entidad>& pref)
        : CE::Escena{}, jugador_ref{pref}
    {
    }
            
    void Escena_Prueba::onInit()
    {
        if(!inicializar)
            return;

        //Inicializar todo lo que contendra la escena 
        auto rect = std::make_shared<Rectangulo>(100.f,20.f,sf::Color::Red,sf::Color::Black);
        rect->setPosicion(200.f,300.f);

        auto texto = std::make_shared<IVJ::Texto>(CE::GestorAssets::Get().getFont("default_font"),"Escena de Prueba");
        texto->setFontSize(30u);

        objetos.agregarPool(rect);
        objetos.agregarPool(texto);
            
        inicializar=0;
    }
    void Escena_Prueba::onFinal()
    {
    
    }
    void Escena_Prueba::onUpdate(float dt)
    {   
        for(auto& obj: objetos.getPool())
        {
            obj->onUpdate(dt);
        }
        static float timer{0.f};
        timer+=dt;
        CE::GLogger::Get().agregarLog("Timer:"+std::to_string(timer),CE::GLogger::Niveles::LOG_DEBUG);

        
    }
    void Escena_Prueba::onInputs(const CE::Botones& accion)
    {
        (void)accion;
    }
    void Escena_Prueba::onRender()
    {
        for(auto& obj: objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
    }
}