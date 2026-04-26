#include "AnimLoader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iostream>

namespace CE
{
    std::map<std::string, AnimClip> AnimLoader::cargar(const std::string& json_path)
    {
        std::ifstream file(json_path);
        if (!file.is_open())
        {
            std::cerr << "[AnimLoader] No se pudo abrir: " << json_path << "\n";
            return {};
        }

        // Mapa auxiliar: nombre_anim -> {min_frame, max_frame}
        std::map<std::string, std::pair<int,int>> ranges;

        std::string line;
        bool in_frames = false;

        while (std::getline(file, line))
        {
            // Detectar sección "frames"
            if (!in_frames)
            {
                if (line.find("\"frames\"") != std::string::npos)
                    in_frames = true;
                continue;
            }

            // Fin del objeto frames
            if (line.find('}') != std::string::npos)
                break;

            // Buscar par "filename.png": N
            auto q1 = line.find('"');
            if (q1 == std::string::npos) continue;
            auto q2 = line.find('"', q1 + 1);
            if (q2 == std::string::npos) continue;

            std::string filename = line.substr(q1 + 1, q2 - q1 - 1);
            if (filename.empty()) continue;

            auto colon = line.find(':', q2);
            if (colon == std::string::npos) continue;

            int index = 0;
            try { index = std::stoi(line.substr(colon + 1)); }
            catch (...) { continue; }

            // --- Extraer nombre de animación ---
            // Ejemplo: "survivor-meleeattack_knife_14.png"
            //  1. Quitar prefijo hasta el primer '-' inclusive
            std::string name = filename;
            auto dashPos = name.find('-');
            if (dashPos != std::string::npos)
                name = name.substr(dashPos + 1);

            // Quitar .png si quedó
            auto dotPng = name.find(".png");
            if (dotPng != std::string::npos)
                name = name.substr(0, dotPng);

            //  2. Quitar sufijo "_NN" (último guion bajo + dígitos)
            auto lastUnderscore = name.rfind('_');
            if (lastUnderscore != std::string::npos)
            {
                // Verificar que lo que sigue sean solo dígitos
                std::string suffix = name.substr(lastUnderscore + 1);
                bool all_digits = !suffix.empty() &&
                    std::all_of(suffix.begin(), suffix.end(), ::isdigit);
                if (all_digits)
                    name = name.substr(0, lastUnderscore);
            }

            // Acumular rango
            if (ranges.find(name) == ranges.end())
                ranges[name] = {index, index};
            else
            {
                ranges[name].first  = std::min(ranges[name].first,  index);
                ranges[name].second = std::max(ranges[name].second, index);
            }
        }

        // Convertir rangos a AnimClip
        std::map<std::string, AnimClip> clips;
        for (auto& [name, rng] : ranges)
            clips[name] = AnimClip{rng.first, rng.second - rng.first + 1};

        return clips;
    }

    AnimClip AnimLoader::getClip(const std::map<std::string, AnimClip>& clips,
                                  const std::string& nombre)
    {
        auto it = clips.find(nombre);
        if (it != clips.end())
            return it->second;
        std::cerr << "[AnimLoader] Clip no encontrado: " << nombre << "\n";
        return AnimClip{0, 1};
    }
}