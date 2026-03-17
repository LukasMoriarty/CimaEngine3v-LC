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
				while (!vacia())
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

			int size() const
			{
				int count = 0;
				Nodo* temp = inicio;
				while (temp != nullptr)
				{
					count++;
					temp = temp->siguiente;
				}
				return count;
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

	// Forward declaration para usar Figuras
	class Figuras;

	// Componente de timer específico para partes de la víbora
	class ICTimer : public CE::IComponentes
	{
		public:
			explicit ICTimer(int max_frames = 60);
			~ICTimer() override = default;
		public:
			int curr_frame;
			int max_frame;
	};

	// Componente que representa una parte del cuerpo de la víbora
	class ICParte : public CE::IComponentes
	{
		public:
			explicit ICParte(std::shared_ptr<Figuras> figura, int timer_max = 15);
			~ICParte() override = default;
		public:
			Cola<CE::Vector2D> cola_posiciones;
			std::shared_ptr<Figuras> parte;
			std::shared_ptr<CE::ITransform> pos;
			std::shared_ptr<ICTimer> timer;
			bool hacerAccion;
	};

	// Componente que gestiona todas las partes del cuerpo de la víbora
	class ICPartesCuerpo : public CE::IComponentes
	{
		public:
			explicit ICPartesCuerpo(int w, int h);
			~ICPartesCuerpo() override = default;

			void agregarParte(std::shared_ptr<ICParte> nueva_parte);

		public:
			std::vector<std::shared_ptr<ICParte>> partes;
			int width;
			int height;
	};

	/**
	 * @class IEstadoEpidemia
	 * @brief Componente que gestiona el estado epidémico de una persona.
	 *
	 * Maneja los estados: Normal, Enfermo, Recuperacion, Inmune, Muerto,
	 * junto con sus tiempos de transición y probabilidades individuales.
	 *
	 * Integración en la cuadrícula: cada persona conoce su (fila, columna)
	 * para que el sistema de contagio pueda localizar sus vecinos.
	 */
	class IEstadoEpidemia : public CE::IComponentes
	{
	public:
		/**
		 * @enum Estado
		 * @brief Estados posibles de la persona en la simulación.
		 */
		enum class Estado
		{
			Normal,       /// Sana, susceptible a infección (sin inmunidad)
			Enfermo,      /// Infectada, contagia vecinos cada segundo durante 3s
			Recuperacion, /// En recuperación (2s), no puede ser infectada
			Inmune,       /// Inmunidad permanente, nunca podrá ser infectada
			Muerto        /// Fallecida, sin interacción
		};

		/**
		 * @brief Constructor.
		 * @param fila    Fila en la cuadrícula (eje Y)
		 * @param columna Columna en la cuadrícula (eje X)
		 */
		explicit IEstadoEpidemia(int fila, int columna);
		~IEstadoEpidemia() override = default;

		/** @brief Devuelve true si la persona puede ser infectada ahora. */
		bool puedeSerInfectado() const
		{
			return estado_actual == Estado::Normal && !tiene_inmunidad_parcial;
		}

		/** @brief Infecta a la persona si está en condiciones de ser infectada. */
		void infectar()
		{
			if (!puedeSerInfectado()) return;
			estado_actual       = Estado::Enfermo;
			timer_estado        = 0.f;
			timer_tick          = 0.f;
			ticks_realizados    = 0;
			pendiente_contagiar = false;
		}

	public:
		Estado estado_actual{Estado::Normal};

		float timer_estado{0.f};        ///< Tiempo acumulado en el estado actual
		float timer_tick{0.f};          ///< Contador para el siguiente tick de contagio (~1 s)
		int   ticks_realizados{0};      ///< Ticks de contagio realizados (máx 3)

		float prob_contagio{0.f};             ///< Prob. de ser infectado por vecino (0.15–0.30)
		float prob_inmunidad_permanente{0.f}; ///< Prob. de inmunidad permanente al recuperarse (0.01–0.10)

		bool  tiene_inmunidad_parcial{false};
		float timer_inmunidad_parcial{0.f};

		// Posicion
		int fila{0};
		int columna{0};

		// Flag
		bool pendiente_contagiar{false}; ///< Activado cuando un tick de contagio se dispara

		static constexpr float DUR_ENFERMO           = 3.f;  ///< Duración del estado enfermo (s)
		static constexpr float DUR_RECUPERACION      = 2.f;  ///< Duración de la recuperación (s)
		static constexpr float DUR_INMUNIDAD_PARCIAL = 2.f;  ///< Duración de inmunidad parcial (s)
		static constexpr float PROB_MUERTE           = 0.10f; ///< Probabilidad de morir al recuperarse
		static constexpr float TICK_INTERVALO        = 1.f;  ///< Intervalo entre ticks de contagio (s)
	};

	class ITriangulo : public CE::IComponentes {
		public:
			explicit ITriangulo(float r, float ang);
			~ITriangulo() override{};
		public:
			sf::CircleShape tri_shape;
			float angulo;
	};
} // namespace IVJ