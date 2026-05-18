#include "IJComponentes.hpp"
#include "../../Motor/Render/Render.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../Figuras/Figuras.hpp"
#include <cmath>
#include <string>

namespace IVJ
{

	IMaquinaEstado::IMaquinaEstado()
	{
		fsm = nullptr;
	}

	IRayo::IRayo(CE::Vector2D& pos,float& sig,const float magnitud)
	:magnitud{magnitud},dir{sig},lp1{pos},lp2{0.f}
	{}

	CE::Vector2D& IRayo::getP1() const
	{
		return lp1;
	}

	CE::Vector2D& IRayo::getP2()
	{
		int direction = (dir > 0) ? 1 : -1;
		lp2.x = lp1.x + direction * magnitud;
		lp2.y = lp1.y ;
		return lp2;
	}

	//Interactuables
	IDialogo::IDialogo()
		:IInteractuable(),texto{L""},id_texto{0}
	{
		texto= L"Texto de Prueba ABC ABC, se debe cargar por el arbol, al precionar una tecla"
			L"se avanza al siguiente texto del dialogo, esto es solo una demostración."
			L" El texto debe tener saltos de linea automáticos para no salirse de la caja"
			L"  de dialogo que se renderiza en pantalla."
			L"Espero que funcione correctamente y se vea bien en pantalla.";

	}
	void IDialogo::onInteractuar(CE::Objeto& obj)
	{
		(void)obj;
		if(!activo) return;
	}

	std::wstring IDialogo::agregarSaltoLinea(const std::wstring& str, size_t max_len)
	{
		std::wstring resultado;
		size_t pos = 0;
		while (pos < str.size())
		{
			size_t chunk_size = std::min(max_len, str.size() - pos);
			resultado += str.substr(pos, chunk_size);
			pos += chunk_size;
			if (pos < str.size())
				resultado += L"\n";
		}
		return resultado;
	}
	void IDialogo::onRender()
	{
		if(!activo) return;
		auto dim = CE::Render::Get().GetVentana().getSize();
		sf::RectangleShape fondo;
		fondo.setSize({dim.x*0.7f,150.f});
		fondo.setFillColor({0,0,0,150});
		fondo.setPosition({(dim.x/3.f)-200.f,120.f});
		sf::Font font{CE::GestorAssets::Get().getFont("nova")};
		std::wstring texto_con_saltos = agregarSaltoLinea(texto,76);
		sf::Text texto_render{font,texto_con_saltos,20};
		texto_render.setPosition({(dim.x/3.f)-180.f,130.f});
		CE::Render::Get().AddToDraw(fondo);
		CE::Render::Get().AddToDraw(texto_render);
	}

	IIndicador::IIndicador(const sf::Texture &ref, float escala,Entidad *p,const sf::Color& c)
		: IInteractuable(),sprite{ref},color{c},escala{escala}
	{
		sprite.setScale({escala,escala});
		auto dim = sprite.getTextureRect().size;
		sprite.setOrigin({dim.x/2.f,dim.y/2.f});
		parent = p;
		if(!shader.loadFromFile(ASSETS "/shaders/prueba.frag",sf::Shader::Type::Fragment)) exit(1);
		shader.setUniform("blob",
		CE::GestorAssets::Get().getTextura("blob"));
		shader.setUniform("mask_color",sf::Glsl::Vec4(color));
	}
	void IIndicador::onInteractuar(CE::Objeto &obj)
	{
		(void)obj;
	}
	void IIndicador::onRender()
	{
		if(!activo) return;
		auto pos = parent->getTransformada()->posicion;
		auto sd = parent->getComponente<CE::ISprite>()->m_sprite.getGlobalBounds().size;
		pos = pos + CE::Vector2D{0,-sd.y};
		sprite.setPosition({pos.x,pos.y});
		sf::RenderStates estados;
		estados.shader = &shader;
		CE::Render::Get().AddToDraw(sprite,estados);
	}

	IGirar::IGirar(const float ang, const float r)
		:CE::IComponentes{}, angulo{ang},radio{r}
	{}

	IArribaAbajo::IArribaAbajo(const float minY, const float maxY, const float vel)
		:CE::IComponentes{}, minY{minY}, maxY{maxY}, velocidad{vel}
	{}

	IWave::IWave(float amp, float freq, float phase)
		:CE::IComponentes{}, amp{amp}, freq{freq}, phase{phase}
	{}

	ICirculos::ICirculos(float radio, float velocidad)
		:CE::IComponentes{}, radio{radio}, velocidad{velocidad}
	{}

	// Implementación de ICTimer
	ICTimer::ICTimer(int max_frames)
		: CE::IComponentes{}, curr_frame{0}, max_frame{max_frames}
	{}

	// Implementación de ICParte
	ICParte::ICParte(std::shared_ptr<Figuras> figura, int timer_max)
		: CE::IComponentes{},
		  cola_posiciones{},
		  parte{figura},
		  pos{std::make_shared<CE::ITransform>()},
		  timer{std::make_shared<ICTimer>(timer_max)},
		  hacerAccion{false}
	{}

	// Implementación de ICPartesCuerpo
	ICPartesCuerpo::ICPartesCuerpo(int w, int h)
		: CE::IComponentes{}, partes{}, width{w}, height{h}
	{}

	IEstadoEpidemia::IEstadoEpidemia(int f, int c)
		: CE::IComponentes{}, fila{f}, columna{c}
	{}

	ITriangulo::ITriangulo(float r, float ang)
		:CE::IComponentes{},
		tri_shape{r, 3},
		angulo{ang}
	{
		tri_shape.setOrigin({r/2.f,r/2.f});
	}

	ITarget::ITarget(CE::Objeto *target)
		: CE::IComponentes{}, pos{nullptr}
	{
		if (target)
			pos = &target->getTransformada()->posicion;
	}

	void ITarget::setTarget(CE::Objeto &t)
	{
		pos = &(t.getTransformada()->posicion);
	}

	IRangoAggro::IRangoAggro(float radio)
		: CE::IComponentes{}, radio{radio},
		marcador{sf::CircleShape{radio}}
	{
		marcador.setOrigin({radio,radio});
		marcador.setFillColor(sf::Color{255,0,0,64});	// mayor alpha = menos transparente
	}

	bool IRangoAggro::estaDentroRango(CE::Objeto& parent, CE::Vector2D& target)
	{
		auto pos = parent.getTransformada()->posicion;

		/*
		 * Asumiendo la formula del ciruclo x^2 + y^2 = r^2
		 * si quiero saber si un punto esta dentro del circulo entonces
		 * el centro en (h,k) podemos calcular la distancia del punto (x,y)
		 * con respecto al centro y el radio
		 * (x-h)^2 + (y-k)^2 <= r^2
		 * punto es el target y centro la pos del ente
		 */
		float r2 = radio * radio;
		float diX = (target.x - pos.x);
		float diY = (target.y - pos.y);
		float dist = diX * diX + diY * diY;
		return dist <= r2;
	}
} // namespace IVJ