#pragma once
#include "../Maquinas/FSM.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/Objetos.hpp"
namespace IVJ
{

	template <typename T>
	class Cola
	{
		private:
			struct Nodo
			{
				T data;
				Nodo *siguiente;
				Nodo(T d) : data(d), siguiente(nullptr) {}
			};
			Nodo *inicio;
			Nodo *fin;
		public:
			Cola() : inicio(nullptr), fin(nullptr) {}

			~Cola()
			{
				while (!vacia)
				{
					pop();
				}
			}

			void push(T d)
			{
				Nodo *nuevo = new Nodo(d);
				if (fin == nullptr)
				{
					inicio = fin = nuevo;
					return;
				}
				fin->siguiente = nuevo;
				fin = nuevo;
			}
			void pop()
			{
				if (inicio == nullptr)
					return;
				Nodo *temp = inicio;
				inicio = inicio->siguiente;
				if (inicio == nullptr)
					fin = nullptr;
				delete temp;
			}

			T front()
			{
				if (inicio != nullptr)
					return inicio->data;
				return T();
			}

			bool vacia()
			{
				return inicio == nullptr;
			}
	};

	class FSM; //referencia circular
	//Maquina estado componente
	class IMaquinaEstado : public CE::IComponentes
	{
		public:
			explicit IMaquinaEstado();
			~IMaquinaEstado() override{};
		public:
			std::shared_ptr<FSM> fsm;
	};

	//componente QUEST con un arreglo de int de 8 bit

	class IRayo : public CE::IComponentes
	{
		public:
			explicit IRayo(CE::Vector2D& pos,float& sig,const float magnitud=1.f );
			~IRayo() override{};
		private:
			float magnitud;
			float& dir;
			CE::Vector2D& lp1;
			CE::Vector2D lp2;
		public:
			CE::Vector2D& getP1() const;
			CE::Vector2D& getP2();
			float getMagnitud() const {return magnitud;}
	};

	class IInteractuable : public CE::IComponentes
	{
		public:
			IInteractuable()=default;
			virtual ~IInteractuable() =default;
			virtual void onInteractuar(CE::Objeto& obj)=0;
		public:
			bool interactuado{false};
	};

	class IIndicador : public IInteractuable
	{
		public:
			explicit IIndicador(const sf::Texture &ref,float escala,Entidad *p, const sf::Color& c);
			virtual ~IIndicador() =default;
			void onInteractuar(CE::Objeto& obj) override;
			void onRender();
		public:
			sf::Sprite sprite;
			sf::Shader shader;
			sf::Color color;
			float escala;
			int frame_activo{0};
			bool activo{false};
		private:
			Entidad* parent;
	};

	class IDialogo : public IInteractuable
	{
		public:
			IDialogo();
			virtual ~IDialogo() = default;
			void onInteractuar(CE::Objeto& obj) override;
			void onRender();
		private:
			std::wstring agregarSaltoLinea(const std::wstring& str, size_t max_len);
		public:
			std::wstring texto;
			int id_texto;
			bool activo{false};
	};

	class IGirar : public CE::IComponentes
	{
		public:
			explicit IGirar(const float ang, const float r);
		public:
			float angulo;
			float radio;
	};

	// lab 2 y 3
	class IArribaAbajo : public CE::IComponentes
	{
		public:
			explicit IArribaAbajo(const float minY, const float maxY, const float vel);
		public:
			float minY;
			float maxY;
			float velocidad;
	};

	class IWave : public CE::IComponentes
	{
		public:
			explicit IWave(float amp, float freq, float phase);
		public:
			float amp;	  // amplitud
			float freq;	 // frecuencia
			float phase;	// fase
	};

	class ICirculos : public CE::IComponentes
	{
		public:
			explicit ICirculos(float radio, float velocidad);
		public:
			float radio;
			float velocidad;
	 };

	// componente nuevo ICTimer
	// ojo que en IComponentes ya hay un ITimer, pero este es diferente, el ITimer del motor es para medir el tiempo en frames, mientras que este ICTimer es para medir el tiempo de cada parte de la vibora, por lo que se deben modificar los atributos para que sean mas especificos a su uso
	class ICTimer {};

	// componente nuevo ICParte
	// para este componente se deben inicializar los otros componentes que tiene de atributo al momento de crear el objeto
	class ICParte {};

	// componente nuevo ICPartesCuerpo
	class ICPartesCuerpo {};
}
