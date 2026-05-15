#include "Camaras.hpp"
#include "../GUI/GLogger.hpp"
#include "../Utils/Lerp.hpp"

namespace CE
{
	int Camara::num_camaras =0;
	Camara::Camara(float x, float y,float w, float h)
		:cam_width{w},cam_height{h},esta_activa{false}
	{
		m_transform = std::make_shared<ITransform>(
			Vector2D{x,y},Vector2D{0,0},0);
		m_view = std::make_shared<sf::View>(
			sf::FloatRect{{x,y},{w,h}});
		m_view->setCenter({cam_width/2.f,cam_height/2.f});
		nombre = "Camara Default #"+std::to_string(Camara::num_camaras);
		Camara::num_camaras++;
	}

	Camara::Camara(const Vector2D& pos, const Vector2D& dim)
		:cam_width{dim.x},cam_height{dim.y},esta_activa{false}
	{
		m_transform = std::make_shared<ITransform>(
			pos,Vector2D{0,0},0);
		m_view = std::make_shared<sf::View>(
			sf::FloatRect{{pos.x,pos.y},{cam_width,cam_height}});

		m_view->setCenter({cam_width/2.f,cam_height/2.f});
		Camara::num_camaras++;
		nombre = "Camara Default #"+std::to_string(Camara::num_camaras);
	}

	void Camara::setViewSize(float x, float y)
	{
		m_view->setSize({x,y});
	}

	void Camara::lockEnObjeto(const std::shared_ptr<Objeto>& obj)
	{
		m_lockObj = obj;
	}

	void Camara::onUpdate(float dt)
	{
		(void)dt;
		//ajustamos el pivote de la camara
		m_view->setCenter(
				{m_transform->posicion.x,
				m_transform->posicion.y});
	}
	/*
	 * CAMARACUADRO
	 *
	*/
	CamaraCuadro::CamaraCuadro(const Vector2D& pos, const Vector2D& dim)
		:Camara{pos,dim},limitex{dim.x},limitey{dim.y}
	{

		nombre = "Camara Cuadro #"+std::to_string(Camara::num_camaras);
	}

	void CamaraCuadro::onUpdate(float dt)
	{
		Camara::onUpdate(dt);
		//safe guard de nulo
		if(!m_lockObj.lock()) return;

		auto mitad = Vector2D(cam_width,cam_height).escala(0.5f);

		auto obj_trans = m_lockObj.lock()->getTransformada();
		auto opos = obj_trans->posicion;
		std::string log = "ObJ: ("+std::to_string(opos.x)+","+std::to_string(opos.y)+")";
		std::string log2 = nombre+": "+std::to_string(m_transform->posicion.x)+","+std::to_string(m_transform->posicion.y)+")";

		GLogger::Get().agregarLog(log,GLogger::Niveles::LOG_DEBUG);
		GLogger::Get().agregarLog(log2,GLogger::Niveles::LOG_DEBUG);
		if(obj_trans->posicion.x > (m_transform->posicion.x + mitad.x))
		{
			m_transform->posicion.x+= cam_width;
			limitex+=limitex;
		}
		if(obj_trans->posicion.y > (m_transform->posicion.y + mitad.y))
		{
			m_transform->posicion.y+= cam_height;
			limitey+=limitey;
		}
	}
	/*Camar LERP
	 */
	CamaraLERP::CamaraLERP(const Vector2D& pos, const Vector2D& dim)
		:Camara{pos,dim},vel{4.f}
	{
		nombre = "Camara LERP #"+std::to_string(Camara::num_camaras);
	}
	void CamaraLERP::onUpdate(float dt)
	{
		Camara::onUpdate(dt);
		if(!m_lockObj.lock()) return;
		Vector2D pos_init  = m_transform->posicion;
		auto pos_lock = m_lockObj.lock()->getTransformada()->posicion;

		m_transform->posicion = lerp(pos_init,pos_lock,vel*dt);
		GLogger::Get().agregarLog("("+std::to_string(m_transform->posicion.x)+","+std::to_string(m_transform->posicion.y)+")",
				GLogger::Niveles::LOG_SEVERO);
	}

	CamaraSnapVentana::CamaraSnapVentana(const Vector2D &pos, const Vector2D &dim, const Vector2D &vdim)
		: Camara{pos,dim}, m_vdim{vdim}
	{
		nombre = "Camara SNAP VENTANA #"+std::to_string(Camara::num_camaras);
	}

	void CamaraSnapVentana::onUpdate(float dt) {
		// llamar comportamiento base
		Camara::onUpdate(dt);
		// si no esta siguiendo a nadie
		if (!m_lockObj.lock()) return;
		if (!lock)  // si no se le ha asignado el jugador
		{
			m_transform->posicion = m_lockObj.lock()->getTransformada()->posicion;
			lock = true;
		}
		// verificaciones
		auto jpos = m_lockObj.lock()->getTransformada()->posicion;
		float ld = {m_transform->posicion.x + m_vdim.x/2.f};	// lado derecho
		float li = {m_transform->posicion.x - m_vdim.x/2.f};	// lado izquierdo
		float lup = {m_transform->posicion.y - m_vdim.y/2.f};   // lado arriba
		float ldn = {m_transform->posicion.y + m_vdim.y/2.f};   // lado abajo

		// eje x
		if (jpos.x >= ld)
			m_transform->posicion.x += (jpos.x - ld);
		else if (jpos.x <= li)
			m_transform->posicion.x += (jpos.x - li);

		// eje y
		if (jpos.y >= ldn)
			m_transform->posicion.y += (jpos.y - ldn);
		else if (jpos.y <= lup)
			m_transform->posicion.y += (jpos.y - lup);
	}

	CamaraSnapLR::CamaraSnapLR(const Vector2D& pos, const Vector2D& dim, const Vector2D& vdim)
	:Camara{pos,dim},m_vdim{vdim}, vel{10.f}
	{
		nombre = "Camara SNAP MARIO WORLD #"+std::to_string(Camara::num_camaras);
	}

	void CamaraSnapLR::onUpdate(float dt)
	{
		Camara::onUpdate(dt);
		if(!m_lockObj.lock()) return;

		if(!lock)
		{
			m_transform->posicion = m_lockObj.lock()->getTransformada()->posicion;
			lock = true;
			estado = 1; // aseguramos que inicie en un estado válido
		}

		Vector2D pos_actual = m_transform->posicion;
		auto jpos = m_lockObj.lock()->getTransformada()->posicion;

		// calculamos los márgenes basados en la posición ACTUAL de la cámara
		float rrm = (pos_actual.x + m_vdim.x / 2.f);	// rrm significa "right right margin", el límite derecho extremo
		float rlm = (pos_actual.x + m_vdim.x / 5.f);	// rlm significa "right left margin", el límite derecho cercano al centro
		float lrm = (pos_actual.x - m_vdim.x / 5.f);	// lrm significa "left right margin", el límite izquierdo cercano al centro
		float llm = (pos_actual.x - m_vdim.x / 2.f);	// llm significa "left left margin", el límite izquierdo extremo

		// calculamos los márgenes para y
		// float rrm_y = (pos_actual.y + m_vdim.y / 2.f);	// bottom margin
		float rlm_y = (pos_actual.y + m_vdim.y / 5.f);	// bottom near center
		float lrm_y = (pos_actual.y - m_vdim.y / 5.f);	// top near center
		// float llm_y = (pos_actual.y - m_vdim.y / 2.f);	// top margin

		switch (estado) {
			case 1: // jugador en la zona izquierda, cámara sigue hacia la DERECHA
				if (jpos.x >= lrm)
				{
					m_transform->posicion.x += (jpos.x - lrm);
				}
				if (jpos.y >= rlm_y)
				{
					m_transform->posicion.y += (jpos.y - rlm_y);
				}
				if (jpos.y <= lrm_y)
				{
					m_transform->posicion.y += (jpos.y - lrm_y);
				}
				// si el jugador se da la vuelta y cruza el límite izquierdo
				if (jpos.x <= llm)
				{
					estado = 3; // iniciar transición a la izquierda
				}
				break;

			case 2: // jugador en la zona derecha, cámara sigue hacia la IZQUIERDA
				if (jpos.x <= rlm)
				{
					m_transform->posicion.x += (jpos.x - rlm);
				}
				if (jpos.y >= rlm_y)
				{
					m_transform->posicion.y += (jpos.y - rlm_y);
				}
				if (jpos.y <= lrm_y)
				{
					m_transform->posicion.y += (jpos.y - lrm_y);
				}
				// si el jugador se da la vuelta y cruza el límite derecho
				if (jpos.x >= rrm)
				{
					estado = 4; // Iniciar transición a la derecha
				}
				break;

			case 3:
			{ // TRANSICIÓN HACIA LA IZQUIERDA
				// calculamos dinámicamente el objetivo para que el jugador quede en 'rlm'
				objetivo_x = jpos.x - (m_vdim.x / 5.f);
				objetivo_y = jpos.y;

				Vector2D pos_objetivo_vec = {objetivo_x, objetivo_y};
				// m_transform->posicion = lerp(pos_actual, pos_objetivo_vec, vel * dt);
				m_transform->posicion = lerp2(pos_actual, pos_objetivo_vec, pos_objetivo_vec, vel * dt); // usando lerp2 para una transición más suave

				// si la distancia al objetivo es imperceptible, terminamos el Lerp
				if (std::abs(m_transform->posicion.x - objetivo_x) < 2.0f && std::abs(m_transform->posicion.y - objetivo_y) < 2.0f) {
					m_transform->posicion.x = objetivo_x;
					m_transform->posicion.y = objetivo_y;
					estado = 2; // pasamos al comportamiento de zona derecha
				}
				break;
			}

			case 4:
			{ // TRANSICIÓN HACIA LA DERECHA
				// calculamos dinámicamente el objetivo para que el jugador quede en 'lrm'
				objetivo_x = jpos.x + (m_vdim.x / 5.f);
				objetivo_y = jpos.y;

				Vector2D pos_objetivo_vec = {objetivo_x, objetivo_y};
				// m_transform->posicion = lerp(pos_actual, pos_objetivo_vec, vel * dt);
				m_transform->posicion = lerp2(pos_actual, pos_objetivo_vec, pos_objetivo_vec, vel * dt); // usando lerp2 para una transición más suave

				// si la distancia al objetivo es imperceptible, terminamos el Lerp
				if (std::abs(m_transform->posicion.x - objetivo_x) < 2.0f && std::abs(m_transform->posicion.y - objetivo_y) < 2.0f) {
					m_transform->posicion.x = objetivo_x;
					m_transform->posicion.y = objetivo_y;
					estado = 1; // pasamos al comportamiento de zona izquierda
				}
				break;
			}

			default:
				break;
		}
	}

	CamaraTopDownLock::CamaraTopDownLock(const Vector2D& pos, const Vector2D& dim)
		:Camara{pos,dim}
	{
		nombre = "Camara TopDown #"+std::to_string(Camara::num_camaras);
	}
	void CamaraTopDownLock::onUpdate(float dt)
	{
		Camara::onUpdate(dt);
		if(!m_lockObj.lock()) return;

		auto pos_lock = m_lockObj.lock()->getTransformada()->posicion;

		m_transform->posicion = pos_lock; // seguimiento directo sin interpolación
	}
}
