#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <Motor/Primitivos/Escena.hpp>
#include <memory>
#include <unordered_set>

namespace IVJ
{
	class Escena_Sprite : public CE::Escena
	{
		public:
			explicit Escena_Sprite(std::shared_ptr<Entidad> &pref);
			virtual ~Escena_Sprite() {};
			void onInit() override;
			void onFinal() override;
			void onUpdate(float dt) override;
			void onInputs(const CE::Botones &accion) override;
			void onRender() override;
			std::shared_ptr<Entidad> getJugador() override { return jugador_ref; }

		private:
			int inicializar{1};

			/// Pies del jugador — tiene el sprite atlas de pies y su propio FSM.
			std::shared_ptr<Entidad> &jugador_ref;

			/// Cuerpo del jugador — tiene el sprite atlas de knife y su propio FSM.
			/// Siempre se posiciona encima de jugador_ref (misma posición).
			std::shared_ptr<Entidad> cuerpo;

			// Player outline uniforms
			sf::Glsl::Vec4 zombie_outline_color{1.f, 0.f, 0.f, 1.f};
			float zombie_outline_thickness{0.015f};
			sf::Vector2f zombie_tex_size{384.f, 384.f};

			// Fog effect
			sf::Shader fog_shader;
			sf::RectangleShape fog_rect;
			sf::Texture fog_tex;
			float fog_tiempo{0.0f};

			// Attack variables
			bool ataque_procesado{false};
			sf::CircleShape ataque_hitbox;
			std::unordered_set<CE::Objeto*> objs_golpeados;
	};
} // namespace IVJ
