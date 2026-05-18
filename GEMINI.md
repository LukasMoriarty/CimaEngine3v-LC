# MODO DE OPERACIÓN CRÍTICO
- Actúa estrictamente en modo CONSULTA (Ask).
- NO generes scripts automáticos ni intentes modificar el sistema de archivos.
- Explica detalladamente qué hacer, en qué archivo (especificando si pertenece a /src/Motor o /src/Juego) y qué líneas modificar en CLion.

# CimaEngine3v-LC: AI Agent Guide

## Architecture Overview

**CimaEngine3v** is a C++17 game engine built on SFML (3.0.x) + ImGui with a **component-based entity system (ECS)** and layered architecture. Two main namespaces:
- **CE (Cima Engine)**: Core engine systems (Motor, Render, Assets, Input, Scenes, Components)
- **IVJ (Juego)**: Game-specific implementation (Entities, Systems, Scenes, Machines/FSM)

**Key Structure**: `src/Motor/` (engine) and `src/Juego/` (game) are intentionally separated. The Motor provides abstractions that Juego extends.

## Build System: CMake + FetchContent

Commands must run from project root:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug  # or Release
cmake --build . --target CimaEngine3v -- -j 8
# Binary: ../bin/Debug/CimaEngine3v or ../bin/Release/CimaEngine3v
```

**Key details**: Assets path is defined at compile-time via `ASSETS` macro pointing to `assets/`. Copy `compile_commands.json` from `build/` to root after CMake for IDE intellisense.

## Entry Point & Game Loop Flow

**File**: `src/Main/main.cpp` - Initializes `Motor` with `MotorConfig` and game layer `Juego`.

**Motor::OnRun()** cycle (`src/Motor/Motor.cpp:25-50`):
1. `OnInit()` - Initialize game layer and render system
2. Loop:
   - `OnEventFrame()` - Poll SFML window events → InputBuffer
   - `OnUpdateFrame(dt)` - Update game logic with delta time
   - `OnRenderFrame()` - Clear screen, render all drawables, display

**Critical**: `Motor` owns the main thread loop and calls methods on abstract `GameLayer`. `Juego` extends `GameLayer` and calls `GestorEscenas` to manage active scene.

## Component System (ECS Pattern)

**Base component**: `CE::IComponentes` (`src/Motor/Componentes/IComponentes.hpp`) - all game components inherit this.

**Core components**:
- `ITransform`: posición, velocidad, angulo
- IControl, IStats, ISprite, ITexto, IShader
- IBoundingBox, IRespawn, IPaths, etc.

**Objects**: `CE::Objeto` (`src/Motor/Primitivos/Objetos.hpp`) extends `sf::Drawable` + `sf::Transformable`. Stores components with `addComponente<T>()` / `getComponente<T>()` using RTTI (dynamic_cast). `IVJ::Entidad extends CE::Objeto` for game-specific behavior.

**Key pattern**: Attach components at runtime, query by type. Enables composition-based design without inheritance chains.

## Singleton Managers

All follow the same pattern - static `Get()` returns mutable instance, private constructor.

- **`CE::Render`**: RenderWindow, RenderTexture (post-process), drawable queue. Call `Render::Get().GetTextura()` to draw to game surface. `DrawGameTexture()` stretches game output to window.
- **`CE::GestorAssets`**: Cache for `sf::Font`, `sf::Texture`, `sf::Sound`, `sf::Music`. Load with `agregarFont(key, path)`, retrieve with `getFont(key)`.
- **`CE::GestorEscenas`**: Scene registry + activation. Register with `registrarEscena(key, scene)`, switch with `cambiarEscena(key)`.
- **`CE::CamarasGestor`**: Camera/viewport management (referenced in Motor setup).

## Input Handling

`CE::InputBuffer` (`src/Motor/Inputs/InputBuffer.hpp`) - circular queue of up to 32 SFML events. Motor collects events → buffer, Juego consumes via `OnInputs(InputBuffer&)`.

`IControl` component tracks 8 directions + 4 action buttons (boolean flags). Game code calls `SistemaControl()` to update based on inputs.

## Key Game Systems (Sistemas)

Located in `src/Juego/Sistemas/Sistemas.hpp` - namespace IVJ, marked `[[maybe_unused]]`:
- `SistemaControl()` - Map inputs to entity movement
- `SistemaMover()` - Apply velocity to transform
- `SistemaColAABB()` - Rectangle collision detection + optional resolution
- `SistemaSpawn()` - Respawn management via Pool
- `SistemaPaths()` - Bezier curve entity movement
- `SistemaZombieSeguir()` - AI pathfinding with line-of-sight
- Debug: `pintarLinea()` - Draw debug lines to screen

## Scene System

**File**: `src/Motor/Primitivos/Escena.hpp` - Abstract base `Escena`.

Required methods: `onInit()`, `onFinal()`, `onUpdate(float dt)`, `onRender()`, `onInputs(*)`. Contains `CE::Pool objetos` for entities and button→action mapping. 

Concrete scenes in `src/Juego/Escenas/` (Escena_Menu, Escena_Prueba, Escena_SS, Escena_Sprite, etc.). Each registers game state and object lifecycle.

## Rendering & GUI

Objects must inherit `sf::Drawable` and implement `draw()`. Motor calls `Render::Get().AddToDraw(obj)` to queue renderables. GUI layers via `CE::GLayer` (ImGui windows) added to Motor's `gui_layers` vector.

In Debug mode: `GLogger`, `GListaObjetos` (object inspector), `GPropiedades`, `GViewport` debug panels auto-loaded. Set `DEBUG` flag in CMakeLists.txt line 13.

## Asset Loading Patterns

Typical scene init (e.g., `Escena_Sprite.cpp:60+`):
```cpp
CE::GestorAssets::Get().agregarTextura("pies", 
    ASSETS "/sprites/Top_Down_Survivor/feet/survivor-feet-atlas.png",
    CE::Vector2D{0.f, 0.f}, CE::Vector2D{3456.f, 3456.f});
CE::GestorAssets::Get().agregarFont("test", ASSETS "/fonts/Eater.ttf");
auto& tex = CE::GestorAssets::Get().getTextura("pies");
```

`ASSETS` macro = compile-time path to `assets/` folder. Spritesheet anatomy: texture file, (x,y) offset, (w,h) region extraction.

## Code Organization

```
Motor/
  ├─ App/ALayer.hpp         # GameLayer abstract base
  ├─ Componentes/           # Component definitions
  ├─ Render/Render.hpp      # Singleton renderer
  ├─ Primitivos/            # ECS: Objeto, Pool, Escena, GestorEscenas
  ├─ GUI/                   # ImGui integration layers
  ├─ Inputs/                # InputBuffer, input handling
  ├─ Camaras/               # Camera system
  ├─ Utils/                 # Vector2D, MotorConfig, helpers
  └─ Motor.cpp/hpp          # Main loop orchestrator

Juego/
  ├─ objetos/               # Entidad (game entity), Persona, TileMap, Texto
  ├─ Componentes/           # Game-specific components (IDialogo, etc.)
  ├─ Escenas/               # Scene implementations
  ├─ Sistemas/              # Game systems (movement, collision, AI, etc.)
  ├─ Maquinas/              # FSM (finite state machines)
  └─ Juego.cpp/hpp          # Ce::GameLayer implementation
```

## C++ Standard & Compiler Flags

- **Standard**: C++17 (line 81, CMakeLists.txt)
- **Debug**: `-ggdb -Wall -Wextra -Werror` (strict + debug symbols)
- **Release**: `-O2` (optimization)
- **RTTI required**: extensive use of `dynamic_cast<T*>()` in component system

## Common Editing Patterns

1. **Add component to entity**: `entity->addComponente(std::make_shared<ISprite>(...))`, then retrieve with `entity->getComponente<ISprite>()`
2. **Attach system logic to scene**: Add function calls in `Escena::onUpdate()`
3. **Map inputs to actions**: Register button→string in scene's `HashBotones` map
4. **Create new scene**: Extend `CE::Escena`, implement virtuals, register in `Juego::OnInit()`
5. **Debug visualization**: Use `pintarLinea()` or ImGui panels (enabled in Debug build)

## Critical Dependencies

- **SFML 3.0.x**: FetchContent downloads at CMake time. Graphics, Audio, Window, System modules used.
- **ImGui v1.91.9b-docking**: ImGui-SFML bridge (v3.0) manages window integration
- **Standard library**: `<memory>` (unique_ptr, shared_ptr), `<map>`, `<vector>`, `<optional>`

## Quick Debugging

- **Rebuild**: `cd build && cmake --build . --target CimaEngine3v -- -j 8`
- **Recompile after header edits**: Clear build folder or use `touch src/file.hpp` to re-trigger CMake
- **Check symbols**: `nm bin/Debug/CimaEngine3v | grep Symbol` for linking issues
- **Enable debug UI**: Ensure `DEBUG` compile flag in CMakeLists.txt, look for GLogger/GViewport windows
- **Input issues**: Check `InputBuffer` state in `Motor::OnEventFrame()` and `Juego::OnInputs(buffer)`

