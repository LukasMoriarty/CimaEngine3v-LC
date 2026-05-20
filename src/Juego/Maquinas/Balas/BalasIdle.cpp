#include "BalasIdle.hpp"
#include "Motor/Componentes/IComponentes.hpp"
namespace IVJ
{
	BalasIdle::BalasIdle(float max_frame_t)
		: max_frames{1}, id_frame{0}, max_frame_time{max_frame_t}, cur_frame_time{max_frame_t}
	{
	}
	void BalasIdle::onEntrar(const Entidad &obj)
	{
		t_w = obj.getComponente<CE::ISprite>()->width;
		t_h = obj.getComponente<CE::ISprite>()->height;
	}
	void BalasIdle::onSalir(const Entidad &obj)
	{
		obj.getComponente<CE::ITimer>()->frame_actual = 0;
	}
	void BalasIdle::onUpdate(const Entidad &obj, float dt)
	{
		cur_frame_time -= dt;
		if (cur_frame_time <= 0)
		{
			obj.getComponente<CE::ISprite>()->m_sprite.setTextureRect(sf::IntRect(
				{
					// pos
					t_w * (id_frame % max_frames),
					0 // segundo renglón
				},
				{// dimensinones
				 t_w, t_h}));
			cur_frame_time = max_frame_time;
			id_frame++;
		}
	}
} // namespace IVJ