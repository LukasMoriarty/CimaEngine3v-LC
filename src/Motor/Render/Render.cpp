#include "Render.hpp"
#include "SFML/Window/ContextSettings.hpp"

namespace CE
{
    Render& Render::Get()
    {
        if(!instancia)
            instancia = new Render();
        return *instancia;
    }

    void Render::Terminar()
    {
        delete instancia;
        //instancia = nullptr;
    }
    void Render::CrearVentana(const MotorConfig& motor_info)
    {
        sf::ContextSettings videosettings;
        videosettings.antiAliasingLevel=3;
        //depende del driver, MacOs = 4.1 , windows/linux 4.6
        //videosettings.majorVersion = 4;
        //videosettings.minorVersion = 1;
        //videosettings.attributeFlags = sf::ContextSettings::Core;

        r_ventana = std::make_unique<sf::RenderWindow>(
                sf::VideoMode({motor_info.vW,motor_info.vH}, 32),
                sf::String(motor_info.titulo),
                sf::Style::Default,
                sf::State::Windowed,
                videosettings);

        r_ventana->setFramerateLimit(65);
        
        // Forzamos 16:9 para la textura de renderizado (puedes ajustar esta resolución)
        const unsigned int targetW = 1920;
        const unsigned int targetH = 1080;

        r_textura = std::make_unique<sf::RenderTexture>(
                sf::Vector2u{targetW, targetH}, videosettings);
        
        // Inicializar la vista de la ventana
        HandleResize(motor_info.vW, motor_info.vH);
    }

    void Render::HandleResize(unsigned int width, unsigned int height)
    {
        // Calcular aspect ratio 16:9
        float targetAspectRatio = 16.0f / 9.0f;
        float windowAspectRatio = (float)width / (float)height;
        
        float viewWidth = 1.0f;
        float viewHeight = 1.0f;
        float posX = 0.0f;
        float posY = 0.0f;

        if (windowAspectRatio > targetAspectRatio)
        {
            // Pillarboxing (barras negras a los lados)
            viewWidth = targetAspectRatio / windowAspectRatio;
            posX = (1.0f - viewWidth) / 2.0f;
        }
        else
        {
            // Letterboxing (barras negras arriba y abajo)
            viewHeight = windowAspectRatio / targetAspectRatio;
            posY = (1.0f - viewHeight) / 2.0f;
        }

        window_view.setViewport(sf::FloatRect({posX, posY}, {viewWidth, viewHeight}));
        window_view.setSize({1920.0f, 1080.0f});
        window_view.setCenter({960.0f, 540.0f});
    }

    void Render::DrawGameTexture()
    {
        // Dibujamos la textura en la ventana usando la vista configurada
        sf::Sprite sprite(r_textura->getTexture());
        r_ventana->setView(window_view);
        r_ventana->draw(sprite);
        
        // Restauramos la vista por defecto para ImGUI (que ocupa toda la ventana)
        r_ventana->setView(r_ventana->getDefaultView());
    }

    CE::Vector2D Render::GetMouseGamePos()
    {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(*r_ventana);
        sf::Vector2f gamePos = r_ventana->mapPixelToCoords(pixelPos, window_view);
        return CE::Vector2D{gamePos.x, gamePos.y};
    }

    void Render::AutoResizeTextura(unsigned int  w, unsigned int h)
    {
        
        auto cs = r_textura->getSize();
        unsigned cw = cs.x;
        unsigned ch = cs.y;
        //std::cout<<w<<"=="<<cw<<" && "<<h<<"=="<<ch<<"\n";
        //no update
        if( (cw==w && ch ==h) || (w==0 || h==0) ) return;

        sf::ContextSettings video;
        video.antiAliasingLevel=3;
        //video.majorVersion = 4;
        //video.minorVersion = 1;
        //video.attributeFlags = sf::ContextSettings::Core;
        r_textura->clear(sf::Color::Black);
        if(!r_textura->resize(sf::Vector2u{w,h},video))
            exit(EXIT_FAILURE);

    }

    void Render::AddToDraw(const sf::Drawable &obj)
    {
        r_textura->draw(obj);
    }

    void Render::AddToDraw(const sf::Drawable &obj, sf::RenderStates shader)
    {
        r_textura->draw(obj,shader);
    }

    void Render::OnClearColor(const sf::Color& clearcolor)
    {
        r_textura->clear(clearcolor);
        r_ventana->clear(sf::Color::Black); // Fondo para las barras negras
    }

    void Render::OnDisplayTextura(void)
    {
        r_textura->display();
    }

    void Render::OnDisplayVentana(void)
    {
        r_ventana->display();
    }

    sf::RenderWindow& Render::GetVentana(void)
    {
        return *r_ventana;
    }
    sf::RenderTexture& Render::GetTextura(void)
    {
        return *r_textura;
    }
}
