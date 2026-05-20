#pragma once
#include "../FSM.hpp"
namespace IVJ
{
	class BalasBoom : public FSM
	{
	public:
		BalasBoom(float max_frame_t);
		virtual ~BalasBoom() = default;
		// No se ocupa para balas, ya que vamos a manipular por fuera
		// el cambio
		virtual FSM *onInputs(const CE::IControl &control) override
		{
			(void)control;
			return nullptr;
		};
		virtual void onEntrar(const Entidad &obj) override;
		virtual void onSalir(const Entidad &obj) override;
		virtual void onUpdate(const Entidad &obj, float dt) override;
	private:
		int max_frames;
		int id_frame;
		float max_frame_time;
		float cur_frame_time;
		int t_w;
		int t_h;
	};
} // namespace IVJ