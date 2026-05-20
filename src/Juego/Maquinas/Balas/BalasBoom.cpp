#include "BalasBoom.hpp"
#include "Motor/Componentes/IComponentes.hpp"
namespace IVJ
{
	BalasBoom::BalasBoom(float max_frame_t)
		: max_frames{3}, id_frame{1}, max_frame_time{max_frame_t}, cur_frame_time{max_frame_t}
	{
	}
	void BalasBoom::onEntrar(const Entidad &obj)
	{
		t_w = obj.getComponente<CE::ISprite>()->width;
		t_h = obj.getComponente<CE::ISprite>()->height;
	}
	void BalasBoom::onSalir(const Entidad &obj)
	{
		(void)obj;
	}
	void BalasBoom::onUpdate(const Entidad &obj, float dt)
	{
		cur_frame_time -= dt;

		if (cur_frame_time <= 0)
		{
			obj.getComponente<CE::ISprite>()->m_sprite.setTextureRect(sf::IntRect(
				{
					// pos
					t_w * (id_frame % max_frames),
					0 //
				},
				{// dimensinones
				 t_w, t_h}));
			cur_frame_time = max_frame_time;
			if (id_frame > 2)
				id_frame = 1;
			id_frame++;
		}
	}
} // namespace IVJ