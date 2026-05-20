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

Additional notes (mouse & gamepad):
- Scenes commonly compute a world-space mouse direction and pass it to entity FSMs. Example pattern (see `src/Juego/Escenas/Escena_Sprite.cpp`): obtain raw mouse position with `CE::Render::Get().GetMouseGamePos()`, map it to world coords with `Render::Get().GetTextura().mapPixelToCoords(..., GestorCamaras::Get().getCamaraActiva().getView())`, normalize the vector and call `Entidad::setMouseDirection(mouse_nx, mouse_ny)`; `Entidad::inputFSM()` forwards these values to `fsm->onInputs(control, mouse_nx, mouse_ny)` (see `src/Juego/objetos/Entidad.{hpp,cpp}`).
- Gamepad input is mixed (OR) with keyboard state in scenes. `Escena_Sprite` uses `sf::Joystick::getAxisPosition` with a deadzone of 25.f and maps button 10 as an attack button; the same `IControl` flags are combined for unified handling.
- Mouse buttons are polled per-frame (e.g. left mouse button sets `IControl::acc`) so scenes may use both event-driven `Botones` and polling for reliable input.

## Key Game Systems (Sistemas)

Located in `src/Juego/Sistemas/Sistemas.hpp` - namespace IVJ, marked `[[maybe_unused]]`:
- `SistemaControl()` - Map inputs to entity movement
- `SistemaMover()` - Apply velocity to transform
- `SistemaColAABB()` / `SistemaColAABBMid()` - Rectangle collision detection + optional resolution
- `SistemaSpawn()` - Respawn management via Pool
- `SistemaPaths()` - Bezier curve entity movement
- `SistemaZombieSeguir()` - AI pathfinding with line-of-sight
- `checkLOS()` - Line-of-sight check for obstacles
- `SistemaNPCLookTarget()` - NPC face/aim toward target
- `SistemaAtaqueMelee()` - Melee attack hitbox + damage application
- `SistemaMoverBalas()` - Projectile movement
- `SistemaDialogos()` - Manage dialog box display
- `SistemaGirar()` - Rotation/spinning behavior (IGirar component)
- Debug: `pintarLinea()` - Draw debug lines to screen
- `revisarDistanciaInteraccion()` - Check if player within interact range of NPC

Note: FSMs may receive mouse-direction parameters. Several FSM implementations provide an overload `onInputs(const CE::IControl& control, float mouse_nx, float mouse_ny)` so animation/state transitions can depend on the mouse direction (see `src/Juego/Maquinas/FSMZombie.hpp` and concrete FSMs). `Entidad::inputFSM()` performs the dispatching logic.

## Scene System

**File**: `src/Motor/Primitivos/Escena.hpp` - Abstract base `Escena`.

Required methods: `onInit()`, `onFinal()`, `onUpdate(float dt)`, `onRender()`, `onInputs(*)`. Contains `CE::Pool objetos` for entities and button→action mapping. 

Concrete scenes in `src/Juego/Escenas/` (Escena_Menu, Escena_Sprite). Each registers game state and object lifecycle.

## Game-Specific Components (IJComponentes)

Located in `src/Juego/Componentes/IJComponentes.hpp` - extends `CE::IComponentes` for game logic:

**FSM & AI**:
- `IMaquinaEstado`: Holds reference to entity's FSM (Finite State Machine)
- `ITarget`: Stores pointer to current target entity
- `IRangoAggro`: Detection radius with helper `estaDentroRango()` - used for NPC aggression ranges
- `IEntorno`: Reference to environment pool (for pathfinding obstacles, neighbors)

**Interaction & Dialog**:
- `IInteractuable`: Abstract base for interactive objects; `onInteractuar(CE::Objeto&)` called when player interacts
- `IIndicador`: Visual indicator above NPCs (sprite + shader, used with `SistemaDialogos`)
- `IDialogo`: Dialog box text display; auto-wraps lines with `agregarSaltoLinea()`

**Movement & Behavior**:
- `IGirar`: Rotation with `angulo` and `radio` for spinning behavior
- `IArribaAbajo`: Vertical oscillation with `minY`, `maxY`, `velocidad` (Lab 2–3)
- `IWave`: Wave motion parameters: `amp` (amplitude), `freq` (frequency), `phase` (Lab 2–3)
- `ICirculos`: Circular movement with `radio` and `velocidad` (Lab 2–3)
- `ITriangulo`: Triangle shape with `angulo` rotation (Lab 2–3)

**Combat & Effects**:
- `IArmaMelee`: Melee weapon data - `radio` (hitbox radius), `offset_adelante` (distance ahead), `damage`, `atacando` flag, `objs_golpeados` set to prevent multi-hit. Includes debug hitbox shape.
- `IRayo`: Ray/laser projectile - stores start/end points and magnitude
- `IPistola`: Gun component with `cooldown` timer and `puedeDisparar()` / `disparar()` helpers (to implement YET)

**Particle & Temporary Effects**:
- `IEmisor`: Particle emitter - references parent entity and prefab; `crearParticula()` spawns copies with rotation/velocity; pool accessed via `getPool()`
- `ITemporal`: Marker component - set `done = true` when effect finishes (signal scene to remove object from pool)
- `ICTimer`: Frame-based timer (`curr_frame`, `max_frame`) for effect timing

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

Additional asset patterns seen in the project:
- Audio: scenes load SFX and music via `CE::GestorAssets::Get().agregarSonido("key", ASSETS "/sfx/...")` and `agregarMusica("key", ASSETS "/music/...")`. Use `CE::GestorAssets::Get().getMusica("key").play()` and `.setVolume()` to control playback (example: `src/Juego/Escenas/Escena_Sprite.cpp` lines ~59-71).
- Animation metadata: animations are loaded from JSON using `CE::AnimLoader::cargar(path)` and then retrieved with `CE::AnimLoader::getClip(...)` (see `Escena_Sprite.cpp` lines ~84-87). This project separates texture atlases from clip/indices JSON files.

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
  ├─ Componentes/           # Game-specific components (IDialogo, IMaquinaEstado, etc.)
  ├─ Escenas/               # Scene implementations
  ├─ Sistemas/              # Game systems (movement, collision, AI, attacks, epidemics)
  ├─ Figuras/               # Shape classes (Rectangulo, Triangulo, Circulo) for lab exercises
  ├─ Maquinas/              # FSM (finite state machines)
  │  ├─ Pies/               # Player feet animation FSM
  │  ├─ Cuerpo/             # Player body/weapon animation FSM
  │  ├─ Zombie/             # Zombie AI behavior FSM
  │  ├─ Balas/              # Projectile/bullet FSMs
  │  ├─ Naves/              # Ship/enemy FSMs
  │  ├─ Bosses/             # Boss behavior FSMs
  │  ├─ FX/                 # Visual effect FSMs (e.g., BloodSplash)
  │  ├─ FSM.hpp             # Base FSM abstract class
  │  └─ FSMBoss.hpp         # Boss-specific FSM base
  └─ Juego.cpp/hpp          # GameLayer implementation
```

## C++ Standard & Compiler Flags

- **Standard**: C++17 (line 81, CMakeLists.txt)
- **Debug**: `-ggdb -Wall -Wextra -Werror` (strict + debug symbols)
- **Release**: `-O2` (optimization)
- **RTTI required**: extensive use of `dynamic_cast<T*>()` in component system

## Advanced Patterns & Systems

### Particle Effects & Temporary Objects

**Pattern**: Use `IEmisor` component to spawn particles, mark them with `ITemporal` to auto-remove when done.

Example structure:
1. Create a prefab Entidad with an animation (ISprite) and ITemporal marker
2. Parent entity adds `IEmisor(parent, prefab)` component
3. Call `IEmisor::crearParticula()` from FSM or system to spawn copies
4. Particle's behavior FSM (e.g., BloodSplash in `src/Juego/Maquinas/FX/BloodSplash.cpp`) sets `ITemporal::done = true` when animation ends
5. Scene's `onUpdate()` removes objects where `getComponente<ITemporal>()->done == true`

**Key files**: `src/Juego/Componentes/IJComponentes.hpp` (IEmisor, ITemporal), `src/Juego/Maquinas/FX/` (effect FSMs)

### Melee Combat System

**Pattern**: Player has `IArmaMelee` component; when attacking FSM state is active, scene calls `SistemaAtaqueMelee()` each frame.

Workflow:
1. Player entity equipped with `IArmaMelee(radius, offset_adelante, damage)`
2. Player FSM enters attack state (sets `IArmaMelee::atacando = true`)
3. Scene's `onUpdate()` calls `SistemaAtaqueMelee(player, game_pool)` while `atacando == true`
4. System center hitbox at `player_pos + offset_adelante * mouse_direction`, collision-check all enemies in pool
5. Track hits in `IArmaMelee::objs_golpeados` to prevent multi-hit per swing
6. Play SFX once per swing (check `swing_sfx_played` flag)
7. FSM exits attack state → reset flags (`atacando = false`, clear `objs_golpeados`, `swing_sfx_played = false`)

**Key files**: `src/Juego/Escenas/Escena_Sprite.cpp` (attack handling), `src/Juego/Sistemas/Sistemas.cpp` (SistemaAtaqueMelee)

## Common Editing Patterns

1. **Add component to entity**: `entity->addComponente(std::make_shared<ISprite>(...))`, then retrieve with `entity->getComponente<ISprite>()`
2. **Attach system logic to scene**: Add function calls in `Escena::onUpdate()`
3. **Map inputs to actions**: Register button→string in scene's `HashBotones` map
4. **Create new scene**: Extend `CE::Escena`, implement virtuals, register in `Juego::OnInit()`
5. **Debug visualization**: Use `pintarLinea()` or ImGui panels (enabled in Debug build)
6. **Add interaction NPC**: Give Entidad `IInteractuable`, `IIndicador`, `IDialogo` components; scene checks `revisarDistanciaInteraccion()` and calls `SistemaDialogos()`
7. **Spawn particles**: Attach `IEmisor(parent, prefab)` to parent; call `crearParticula()` from FSM; mark particles with `ITemporal`
8. **Melee attack**: equip entity with `IArmaMelee`; add attack state to FSM; scene calls `SistemaAtaqueMelee()` in loop

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

