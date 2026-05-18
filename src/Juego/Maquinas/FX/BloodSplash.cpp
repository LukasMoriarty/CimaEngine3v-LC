#include "BloodSplash.hpp"
#include <Motor/Componentes/IComponentes.hpp>

namespace IVJ
{

	BloodSplash::BloodSplash(CE::AnimClip clip, float frame_rate)
	    : m_clip{clip}, max_frame_time{frame_rate}, cur_frame_time{frame_rate}
	{
	    nombre = "BloodSplash";
	}

	void BloodSplash::onEntrar(const Entidad& obj)
	{
	    auto spriteComp = obj.getComponente<CE::ISprite>();
	    if (!spriteComp) return;

	    sprite = &spriteComp->m_sprite;
	    s_w = spriteComp->width;
	    s_h = spriteComp->height;

	    auto texSize = sprite->getTexture().getSize();
	    num_cols = (s_w > 0) ? static_cast<int>(texSize.x) / s_w : 1;

	    id_frame = m_clip.first_frame;
	    cur_frame_time = max_frame_time;
	    local_frame = 0;
	    finished = false;
	}

	void BloodSplash::onSalir(const Entidad& obj)
	{
	    (void)obj;
	}

	void BloodSplash::onUpdate(const Entidad& obj, float dt)
	{
	    if (finished) return;
	    if (!sprite) return;

	    cur_frame_time -= dt;
	    if (cur_frame_time > 0.f) return;
	    cur_frame_time = max_frame_time;

	    int col = id_frame % num_cols;
	    int row = id_frame / num_cols;
	    sprite->setTextureRect(sf::IntRect{{col * s_w, row * s_h}, {s_w, s_h}});

	    local_frame++;
	    if (local_frame >= m_clip.frame_count)
	    {
	        finished = true;
	        // Mark temporal component if present
	        auto temp = const_cast<Entidad&>(obj).getComponente<ITemporal>();
	        if (temp) temp->done = true;
	    }
	    else
	    {
	        id_frame = m_clip.first_frame + local_frame;
	    }
	}
} // namespace IVJ

