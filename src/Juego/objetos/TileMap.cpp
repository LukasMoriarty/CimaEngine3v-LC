#include "TileMap.hpp"
#include "../objetos/Entidad.hpp"
#include <fstream>
#include <iostream>

namespace IVJ
{
	bool TileMap::loadTileMap(const std::string& atlas_path, CE::Pool& objetos, CE::Pool& spawn_pool)
	{
		std::ifstream archivo {atlas_path};
		std::string info;
		archivo>>info;//tag [info]
		archivo>>atlas_p>>atlas_w>>atlas_h>>
		tile_w>>tile_h>>map_r>>map_c>>info;
		tiles_vertex.setPrimitiveType(sf::PrimitiveType::Triangles);
		tiles_vertex.resize(map_r*map_c*6);
		if(!atlas_texture.loadFromFile(ASSETS+atlas_p))
		{
			std::cerr<<"[TileMap] No se pudo cargar la imagen "<<atlas_p<<"\n";
			return false;
		}

		int id=0;
		int max_col = atlas_w/tile_w;
		char col;

		for (int j = 0; j < map_r; ++j)
		{
			for (int i = 0; i < map_c; ++i)
			{
				if (!(archivo >> id >> col)) break;

				// If it's a spawn tile, create a spawn zone object
				if (col == 's')
				{
					// Use an Entidad (concrete subclass of CE::Objeto) for spawn zones
					auto spawn_zone = std::make_shared<Entidad>();
					spawn_zone->setPosicion(i * static_cast<float>(tile_w) + tile_w / 2.f,
											 j * static_cast<float>(tile_h) + tile_h / 2.f);

					// IRespawn constructors available:
					//  IRespawn(std::vector<std::shared_ptr<IComponentes>>& lista_comp, int max);
					//  IRespawn(int max,int w, int h, IVJ::Entidad *pre );
					// The previous code attempted to pass a Vector2D + timers which does not match
					// any constructor. Create the respawn component with the (max,w,h,prefab)
					// constructor and then set timer values explicitly.
					auto resp = std::make_shared<CE::IRespawn>(
							5,            // max_spawn
							tile_w,      // width (pixels)
							tile_h,      // height (pixels)
							static_cast<IVJ::Entidad*>(nullptr) // no prefab
					);

					// Configure timers (previously passed as parameters)
					resp->timer_maximo = 120; // frames between spawns
					resp->timer_actual = 0;

					spawn_zone->addComponente(resp);
					spawn_pool.agregarPool(spawn_zone);
				}

				if(id != -1)
				{
					sf::Vertex *trian = &tiles_vertex[(i + j * map_c) * 6];
					int tU = id % max_col;
					int tV = id / max_col;

					if(col == 'c')
					{
						float hw = tile_w/2.f;
						float hh = tile_h/2.f;
						auto bloque = std::make_shared<Entidad>();
						bloque->getStats()->hp = 100;
						bloque->setPosicion(i*tile_w+hw, j*tile_h+hh);
						bloque->addComponente(std::make_shared<CE::IBoundingBox>(
									CE::Vector2D{(float)tile_w, (float)tile_h}));
						objetos.agregarPool(bloque);
					}

					trian[0].position = sf::Vector2f(i * static_cast<float>(tile_w), j * static_cast<float>(tile_h));
					trian[1].position = sf::Vector2f((i + 1) * static_cast<float>(tile_w), j * static_cast<float>(tile_h));
					trian[2].position = sf::Vector2f(i * static_cast<float>(tile_w), (j + 1) * static_cast<float>(tile_h));
					trian[3].position = sf::Vector2f(i * static_cast<float>(tile_w), (j + 1) * static_cast<float>(tile_h));
					trian[4].position = sf::Vector2f((i + 1) * static_cast<float>(tile_w), j * static_cast<float>(tile_h));
					trian[5].position = sf::Vector2f((i + 1) * static_cast<float>(tile_w), (j + 1) * static_cast<float>(tile_h));

					trian[0].texCoords = sf::Vector2f(tU * static_cast<float>(tile_w), tV * static_cast<float>(tile_h));
					trian[1].texCoords = sf::Vector2f((tU + 1) * static_cast<float>(tile_w), tV * static_cast<float>(tile_h));
					trian[2].texCoords = sf::Vector2f(tU * static_cast<float>(tile_w), (tV + 1) * static_cast<float>(tile_h));
					trian[3].texCoords = sf::Vector2f(tU * static_cast<float>(tile_w), (tV + 1) * static_cast<float>(tile_h));
					trian[4].texCoords = sf::Vector2f((tU + 1) * static_cast<float>(tile_w), tV * static_cast<float>(tile_h));
					trian[5].texCoords = sf::Vector2f((tU + 1) * static_cast<float>(tile_w), (tV + 1) * static_cast<float>(tile_h));
				}
			}
		}
		return true;
	}

	bool TileMap::loadTileMap(const std::string& atlas_path)
	{
		std::ifstream archivo {atlas_path};
		std::string info;
		archivo>>info;//tag [info]
		archivo>>atlas_p>>atlas_w>>atlas_h>>
		tile_w>>tile_h>>map_r>>map_c>>info;
		tiles_vertex.setPrimitiveType(sf::PrimitiveType::Triangles);
		tiles_vertex.resize(map_r*map_c*6);
		if(!atlas_texture.loadFromFile(ASSETS+atlas_p))
		{
			std::cerr<<"[TileMap] No se pudo cargar la imagen "<<atlas_p<<"\n";
			return false;
		}
		int id=0;
		int max_col = atlas_w/tile_w;

		// Forzar a leer SOLO el tamaño exacto de la cuadrícula
		for (int j = 0; j < map_r; ++j)
		{
			for (int i = 0; i < map_c; ++i)
			{
				if (!(archivo >> id)) break; // Evita error si el archivo es más corto

													if(id != -1) // no ignorar tile
													{
													 // Referencia al triangulo a dibujar
													 sf::Vertex *trian = &tiles_vertex[(i + j * map_c) * 6];

													 // Posición del tile en el atlas
													 int tU = id % max_col;
													 int tV = id / max_col;

													 // Vertices del triángulo
													 trian[0].position = sf::Vector2f(i * static_cast<float>(tile_w), j * static_cast<float>(tile_h));
													 trian[1].position = sf::Vector2f((i + 1) * static_cast<float>(tile_w), j * static_cast<float>(tile_h));
													 trian[2].position = sf::Vector2f(i * static_cast<float>(tile_w), (j + 1) * static_cast<float>(tile_h));
													 trian[3].position = sf::Vector2f(i * static_cast<float>(tile_w), (j + 1) * static_cast<float>(tile_h));
													 trian[4].position = sf::Vector2f((i + 1) * static_cast<float>(tile_w), j * static_cast<float>(tile_h));
													 trian[5].position = sf::Vector2f((i + 1) * static_cast<float>(tile_w), (j + 1) * static_cast<float>(tile_h));

													 // Textura del triángulo
													 trian[0].texCoords = sf::Vector2f(tU * static_cast<float>(tile_w), tV * static_cast<float>(tile_h));
													 trian[1].texCoords = sf::Vector2f((tU + 1) * static_cast<float>(tile_w), tV * static_cast<float>(tile_h));
													 trian[2].texCoords = sf::Vector2f(tU * static_cast<float>(tile_w), (tV + 1) * static_cast<float>(tile_h));
													 trian[3].texCoords = sf::Vector2f(tU * static_cast<float>(tile_w), (tV + 1) * static_cast<float>(tile_h));
													 trian[4].texCoords = sf::Vector2f((tU + 1) * static_cast<float>(tile_w), tV * static_cast<float>(tile_h));
													 trian[5].texCoords = sf::Vector2f((tU + 1) * static_cast<float>(tile_w), (tV + 1) * static_cast<float>(tile_h));
													}
			}
		}
		return true;
	}

	void TileMap::draw(sf::RenderTarget& target, sf::RenderStates state) const
	{
		state.transform*=getTransform();
		state.texture = &atlas_texture;
		//state.shader = &tilemap_shader->m_shader;
		if(tilemap_shader)
			target.draw(tiles_vertex,&tilemap_shader->m_shader);
		else
			target.draw(tiles_vertex,state);
	}

	bool TileMap::loadTileMapShaders(const std::string& vert, const std::string& frag)
	{
		tilemap_shader = std::make_unique<CE::IShader>(vert,frag);
		if(!tilemap_shader) return false;
		return true;
	}
}
