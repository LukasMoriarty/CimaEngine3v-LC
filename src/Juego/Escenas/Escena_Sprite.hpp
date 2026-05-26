#pragma once
#include <Juego/objetos/Entidad.hpp>
#include <Juego/objetos/TileMap.hpp>
#include <Motor/Primitivos/Escena.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <unordered_set>
#include "Motor/Utils/AnimLoader.hpp"

namespace IVJ
{
	class Escena_Sprite : public CE::Escena
	{
		public:
			explicit Escena_Sprite(std::shared_ptr<Entidad> &pref);
			static void cargarSonidos();
			void actualizarArmaEquipada() const;
			virtual ~Escena_Sprite() {};
			void onInit() override;
			void onFinal() override;
			void onUpdate(float dt) override;
			void onInputs(const CE::Botones &accion) override;
			void onRender() override;
			std::shared_ptr<Entidad> getJugador() override { return jugador_ref; }

		private:
			int inicializar{1};

			int ronda_actual{1};
			int zombies_ronda_total{4};
			int zombies_por_generar_ronda{4};
			int zombies_matados_ronda{0};
			int puntaje_total{0};
			float tiempo_espera_ronda{0.f};

			bool jugador_muerto{false};
			bool en_pausa{false};
			int  opcion_pausa{0}; // 0 = Continuar, 1 = Salir

			std::unique_ptr<sf::Text> mensaje_pantalla;
			std::unique_ptr<sf::Text> hud_texto;

			std::shared_ptr<Entidad> &jugador_ref;
			std::shared_ptr<Entidad>  cuerpo;

			CE::Pool m_spawn_pool{50};
			std::map<std::string, CE::AnimClip> clips_zombie;

			// shaders / fog
			sf::Glsl::Vec4      zombie_outline_color{1.f, 0.f, 0.f, 1.f};
			sf::Vector2f        zombie_tex_size{384.f, 384.f};
			sf::Shader          fog_shader;
			sf::RectangleShape  fog_rect;
			sf::Texture         fog_tex;
			float               fog_tiempo{0.0f};

			static constexpr float ATTACK_HITBOX_RADIUS = 48.f;
			bool               ataque_procesado{false};
			sf::CircleShape    ataque_hitbox;
			std::unordered_set<CE::Objeto*> objs_golpeados;
		
			void resetJuego();
	};
} // namespace IVJ