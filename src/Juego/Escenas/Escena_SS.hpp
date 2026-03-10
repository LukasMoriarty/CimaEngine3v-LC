#pragma once
#include <Motor/Primitivos/Escena.hpp>
#include <Juego/Figuras/Figuras.hpp>
#include <memory>

namespace IVJ
{
    class EnteVibora;  // Forward declaration

    class Escena_SS : public CE::Escena
    {
    public:
        explicit Escena_SS(std::shared_ptr<Entidad>& pref);
        virtual ~Escena_SS(){};

        void onInit() override;
        void onFinal() override;
        void onUpdate(float dt) override;
        void onInputs(const CE::Botones& accion) override;
        void onRender() override;

        std::shared_ptr<Entidad> getJugador() override {return jugador_ref;}

    private:
        int inicializar{1};
        std::shared_ptr<Entidad>& jugador_ref;

        // Figuras de prueba
        std::shared_ptr<Figuras> figura;
        std::shared_ptr<Figuras> figura2;

        // La víbora (EJERCICIO)
        std::shared_ptr<EnteVibora> vibora;

        // Timer para agregar partes cada 3 segundos
        float timer_agregar_parte;
    };
}