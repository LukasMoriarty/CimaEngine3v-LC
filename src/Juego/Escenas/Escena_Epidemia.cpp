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
        if (!inicializar)
            return;

        // 1. Obtener las dimensiones de la ventana dinámicamente
        auto windowSize = CE::Render::Get().GetVentana().getSize();

        // 2. Definir las variables de tu cuadrícula
        int columnas = 15;
        int filas = 15;
        float tam_celda = 30.f;

        // 3. Calcular el tamaño total de la cuadrícula completa
        float ancho_total_cuadricula = columnas * tam_celda;
        float alto_total_cuadricula = filas * tam_celda;

        // 4. Calcular los márgenes (offsets) para centrarla
        float offset_x = (windowSize.x - ancho_total_cuadricula) / 2.0f;
        float offset_y = (windowSize.y - alto_total_cuadricula) / 2.0f;

        // 5. Hacer la cuadrícula 15x15 aplicando los offsets
        for (int i = 0; i < columnas; ++i)
        {
            for (int j = 0; j < filas; ++j)
            {
                auto rect = std::make_shared<Rectangulo>(tam_celda, tam_celda, sf::Color::White, sf::Color::Black);
                rect->setPosicion(
                    offset_x + (i * tam_celda),
                    offset_y + (j * tam_celda)
                );

                objetos.agregarPool(rect);
            }
        }

        auto texto = std::make_shared<IVJ::Texto>(CE::GestorAssets::Get().getFont("default_font"),"Escena Epidemia");
        texto->setFontSize(30u);

        texto->setPosicion(windowSize.x / 2.0f - 100.f, 20.f);

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