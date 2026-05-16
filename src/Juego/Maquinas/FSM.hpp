#pragma once

#include "../Componentes/IJComponentes.hpp"
#include "../objetos/Entidad.hpp"
#include <SFML/Graphics.hpp>

namespace IVJ
{
    class Entidad; //referencia circular
    class FSM
    {
        public:
            virtual ~FSM()=default;
            virtual FSM* onInputs(const CE::IControl& control)=0;

            /**
             * @brief Procesa inputs con información de dirección del mouse normalizada.
             * @param control Estado de controles de entrada
             * @param mouse_dir_norm Dirección normalizada hacia donde apunta el mouse (componentes x,y)
             * @note Implementación por defecto simplemente ignora mouse_dir_norm y llama a onInputs(control)
             */
            virtual FSM* onInputs(const CE::IControl& control, [[maybe_unused]] float mouse_nx, [[maybe_unused]] float mouse_ny)
            {
                return onInputs(control);
            }

            virtual void onEntrar(const Entidad& obj)=0;
            virtual void onSalir(const Entidad& obj)=0;
            virtual void onUpdate(const Entidad& obj,float dt)=0;
            std::string nombre;
    };
}
