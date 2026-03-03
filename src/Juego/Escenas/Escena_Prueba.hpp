#pragma once
#include <Motor/Primitivos/Escena.hpp>
#include <memory>

namespace IVJ
{
    class Escena_Prueba : public CE::Escena
    {
        public:

            explicit Escena_Prueba(std::shared_ptr<Entidad>& pref);
            virtual ~Escena_Prueba(){};
            void onInit() override;
            void onFinal() override;
            void onUpdate(float dt) override;
            void onInputs(const CE::Botones& accion) override;
            void onRender() override;
            std::shared_ptr<Entidad> getJugador() override {return jugador_ref;}
        private:
            int inicializar{1};
            std::shared_ptr<Entidad>& jugador_ref;
    };
}