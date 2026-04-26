#pragma once
#include <string>
#include <map>

namespace CE
{
	/**
	 * @struct AnimClip
	 * @brief Rango de frames de una animación en un atlas.
	 */
	struct AnimClip
	{
		int first_frame{0};  ///< Índice absoluto del primer frame en el atlas
		int frame_count{0};  ///< Cantidad de frames de la animación
	};

	/**
	 * @class AnimLoader
	 * @brief Parsea los JSON generados por generar_metadatos.py y devuelve
	 *        clips de animación agrupados por nombre.
	 *
	 * El nombre de cada clip se extrae del nombre de archivo eliminando el
	 * prefijo "survivor-" y el sufijo "_NN.png".
	 *
	 * Ejemplos para el atlas de pies:
	 *   "idle"         -> {0,  1}
	 *   "run"          -> {1,  20}
	 *   "strafe_left"  -> {21, 20}
	 *   "strafe_right" -> {41, 20}
	 *   "walk"         -> {61, 20}
	 *
	 * Ejemplos para el atlas de knife:
	 *   "idle_knife"        -> {0,  20}
	 *   "meleeattack_knife" -> {20, 15}
	 *   "move_knife"        -> {35, 20}
	 */
	class AnimLoader
	{
	public:
		/**
		 * @brief Carga y parsea un archivo JSON de metadata de animación.
		 * @param json_path Ruta al archivo indices_animacion.json
		 * @return Mapa nombre_animacion -> AnimClip
		 */
		static std::map<std::string, AnimClip> cargar(const std::string& json_path);

		/**
		 * @brief Obtiene un clip del mapa; devuelve clip vacío si no existe.
		 */
		static AnimClip getClip(const std::map<std::string, AnimClip>& clips,
								 const std::string& nombre);
	};
}