#pragma once
#include "../FSM.hpp"
#include "../../../Motor/Utils/AnimLoader.hpp"

namespace IVJ {
	class IdleZombie : public FSM {
		public:
			explicit IdleZombie(CE::AnimClip clip, float frame_rate = 0.05f);
			~IdleZombie() override = default;

			void onEntrar(const Entidad &obj) override;
			void onSalida(const Entidad &obj) override;
			void onUpdate(const Entidad &obj) override;
		private:
			sf::Sprite* sprite{nullptr};
			int s_w{0};
			int s_h{0};
			int num_cols{7};

			CE::AnimClip m_clip;    ///< Rango de frames de esta animación
			int id_frame{0};        ///< Frame absoluto actual (dentro del clip)

			float max_frame_time;
			float cur_frame_time;
	};
}