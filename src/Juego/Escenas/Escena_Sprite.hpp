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

			// Pool para gestionar las zonas de spawn de enemigos (reserve capacity)
			CE::Pool m_spawn_pool{50};

			// Clips de animación de zombie para pasar al sistema de spawn
			std::map<std::string, CE::AnimClip> clips_zombie;

			// Player outline uniforms
			sf::Glsl::Vec4 zombie_outline_color{1.f, 0.f, 0.f, 1.f};
			float zombie_outline_thickness{0.015f};
			sf::Vector2f zombie_tex_size{384.f, 384.f};

			// Fog effect
			sf::Shader fog_shader;
			sf::RectangleShape fog_rect;
			sf::Texture fog_tex;
			float fog_tiempo{0.0f};

			// ── Attack constants ──────────────────────────────────────────────
			/**
			 * @brief Radius (in world units) of the melee attack hitbox.
			 *
			 * Tune this single value to adjust reach without hunting for
			 * magic numbers scattered across the update loop.
			 * The hitbox is centred ATTACK_HITBOX_RADIUS pixels ahead of the
			 * player along the mouse direction.
			 */
			static constexpr float ATTACK_HITBOX_RADIUS = 48.f;

			// ── Attack runtime state ──────────────────────────────────────────
			bool ataque_procesado{false};
			sf::CircleShape ataque_hitbox;
			std::unordered_set<CE::Objeto *> objs_golpeados;

			// ── Attack SFX state ──────────────────────────────────────────────
			/// True after the swing SFX has fired for the current attack swing.
			/// Resets when the player leaves the attack state.
			bool swing_sfx_played{false};
	};
} // namespace IVJ