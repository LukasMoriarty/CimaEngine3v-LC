#pragma once
#include <unordered_set>


#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/Objetos.hpp"
#include "../Maquinas/FSM.hpp"
#include "Motor/Primitivos/CEPool.hpp"

namespace IVJ
{
	class FSM; //referencia circular
	//Maquina estado componente
	class IMaquinaEstado : public CE::IComponentes
	{
		public:
			explicit IMaquinaEstado();
			~IMaquinaEstado() override{};
			// funcion de copiar
			std::shared_ptr<IComponentes> clonar() const override
			{
				return std::make_shared<IMaquinaEstado>(*this);
			}
		public:
			std::shared_ptr<FSM> fsm;
			bool congelar{false};
	};

	//componente QUEST con un arreglo de int de 8 bit

	class IRayo : public CE::IComponentes
	{
		public:
			explicit IRayo(CE::Vector2D& pos,float& sig,const float magnitud=1.f );
			~IRayo() override{};
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IRayo>(*this); }
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
			// IIndicador contains an sf::Shader which is non-copyable; cloning is not supported
			std::shared_ptr<CE::IComponentes> clonar() const override { return nullptr; }
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
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IDialogo>(*this); }
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
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IGirar>(*this); }
		public:
			float angulo;
			float radio;
	};

	// lab 2 y 3
	class IArribaAbajo : public CE::IComponentes
	{
		public:
			explicit IArribaAbajo(const float minY, const float maxY, const float vel);
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IArribaAbajo>(*this); }
		public:
			float minY;
			float maxY;
			float velocidad;
	};

	class IWave : public CE::IComponentes
	{
		public:
			explicit IWave(float amp, float freq, float phase);
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IWave>(*this); }
		public:
			float amp;	  // amplitud
			float freq;	 // frecuencia
			float phase;	// fase
	};

	class ICirculos : public CE::IComponentes
	{
		public:
			explicit ICirculos(float radio, float velocidad);
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<ICirculos>(*this); }
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
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<ICTimer>(*this); }
		public:
			int curr_frame;
			int max_frame;
	};

	// Simple marker component for short-lived visual effects (e.g. blood splashes).
	// When 'done' is set to true the scene can remove the object from its pool.
	class ITemporal : public CE::IComponentes {
		public:
			explicit ITemporal() : done(false) {}
			~ITemporal() override = default;
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<ITemporal>(*this); }
		public:
			bool done;
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
		std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IEstadoEpidemia>(*this); }

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
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<ITriangulo>(*this); }
		public:
			sf::CircleShape tri_shape;
			float angulo;
	};

	class ITarget : public CE::IComponentes
	{
		public:
			explicit ITarget(CE::Objeto * target);
			void setTarget(CE::Objeto& t);
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<ITarget>(*this); }
		public:
			CE::Vector2D* pos;
	};
	class IRangoAggro : public CE::IComponentes
	{
		public:
			explicit IRangoAggro(float radio);
			virtual ~IRangoAggro(){};
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IRangoAggro>(*this); }
		bool estaDentroRango(CE::Objeto& parent, CE::Vector2D& target);
		public:
			float radio;
			sf::CircleShape marcador;
	};

	class IEntorno : public CE::IComponentes
	{
		public:
			explicit IEntorno(const std::vector<std::shared_ptr<CE::Objeto>>* pool) : pool(pool) {}
			std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IEntorno>(*this); }
		const std::vector<std::shared_ptr<CE::Objeto>>* pool;
	};

	class IArmaMelee : public CE::IComponentes {
	public:
		float radio;
		float offset_adelante; // Distancia frente al personaje
		int damage;
		bool atacando{false};  // Se activa desde la FSM (AtacarCuerpoKnife)
		bool swing_played{false};	// true mientras el swing SFX ya se disparó este ataque

		// Guarda a quién ya golpeó en este swing para no repetir daño
		std::unordered_set<CE::Objeto *> objs_golpeados;

		// Para debug (se dibuja si atacando == true)
		sf::CircleShape hitbox_debug;

		explicit IArmaMelee(float radio = 48.f, float offset = 48.f, int damage = 15)
			: CE::IComponentes{}, radio(radio), offset_adelante(offset), damage(damage)
		{
			hitbox_debug.setRadius(radio);
			hitbox_debug.setOrigin({radio, radio});
			hitbox_debug.setFillColor(sf::Color(255, 0, 0, 100)); // Rojo semi-transparente
		}
		std::shared_ptr<CE::IComponentes> clonar() const override { return std::make_shared<IArmaMelee>(*this); }
	};
	class IEmisor : public CE::IComponentes {
		public:
			explicit IEmisor(Entidad &parent, Entidad &prefab);
			~IEmisor(void) {};
			std::shared_ptr<IComponentes> clonar() const override
			{
				return std::make_shared<IEmisor>(*this);
			}
			void onUpdate(float dt);
			void crearParticula(void);
			CE::Pool &getPool(void) {
				return pool;
			}
		private:
			CE::Pool pool;
			Entidad &parent;	// quien tiene el emisor
			Entidad &prefab;	// el objeto que se va a copiar
	};
	class IPistola : public CE::IComponentes {
		public:
			explicit IPistola(float cooldown);
			~IPistola(void) {};
			std::shared_ptr<IComponentes> clonar() const override
			{
				return std::make_shared<IPistola>(*this);
			}
			bool puedeDisparar() const;
			void disparar();
		private:
			float cooldown;		// tiempo mínimo entre disparos (s)
			float timer;		// tiempo acumulado desde el último disparo (s)
	};
} // namespace IVJ