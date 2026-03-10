#include "Escena_Epidemia.hpp"
#include <Juego/Figuras/Figuras.hpp>
#include <Juego/objetos/Texto.hpp>
#include <Motor/Render/Render.hpp>
#include <Motor/Primitivos/GestorAssets.hpp>
#include <Motor/GUI/GLogger.hpp>

namespace IVJ
{
    Escena_Epidemia::Escena_Epidemia(std::shared_ptr<Entidad>& pref)
        : CE::Escena{}, jugador_ref{pref}
    {
    }
            
    void Escena_Epidemia::onInit()
    {
        if(!inicializar)
            return;

        //Inicializar
        auto rect = std::make_shared<Rectangulo>(100.f,20.f,sf::Color::Red,sf::Color::Black);

        auto windowSize = CE::Render::Get().GetVentana().getSize();
		float x = static_cast<float>(windowSize.x);
		float y = static_cast<float>(windowSize.y);
        rect->centrarObjeto(x,y);

        auto texto = std::make_shared<IVJ::Texto>(CE::GestorAssets::Get().getFont("default_font"),"Escena Epidemia");
        texto->setFontSize(30u);

        objetos.agregarPool(rect);
        objetos.agregarPool(texto);
            
        inicializar=0;
    }
    void Escena_Epidemia::onFinal()
    {
    
    }
    void Escena_Epidemia::onUpdate(float dt)
    {   
        for(auto& obj: objetos.getPool())
        {
            obj->onUpdate(dt);
        }
        static float timer{0.f};
        timer+=dt;
        CE::GLogger::Get().agregarLog("Timer:"+std::to_string(timer),CE::GLogger::Niveles::LOG_DEBUG);

        
    }
    void Escena_Epidemia::onInputs(const CE::Botones& accion)
    {
        (void)accion;
    }
    void Escena_Epidemia::onRender()
    {
		CE::Render::Get().OnClearColor(sf::Color(255,255,255));
        for(auto& obj: objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
    }
}