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

**File**: [main.cpp](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Main/main.cpp) - Initializes `Motor` with `MotorConfig` and game layer `Juego`.

**[Motor::OnRun()](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Motor.cpp#L25-L50)** cycle:
1. `OnInit()` - Initialize game layer and render system
2. Loop:
   - `OnEventFrame()` - Poll SFML window events → InputBuffer
   - `OnUpdateFrame(dt)` - Update game logic with delta time
   - `OnRenderFrame()` - Clear screen, render all drawables, display

**Critical**: `Motor` owns the main thread loop and calls methods on abstract `GameLayer`. `Juego` extends `GameLayer` and calls `GestorEscenas` to manage active scene.

## Component System (ECS Pattern)

**Base component**: [IComponentes](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L25) - all game components inherit this.

**Core components** (in [IComponentes.hpp](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp)):
- [ITransform](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L69): posicion, velocidad, angulo, pos_prev
- [IControl](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L382): arr, abj, izq, der, run, jmp, acc, sacc (8 booleans)
- [IStats](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L148): hp, hp_max, str, def, agi, congelar, timer_damage_shader
- [ISprite](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L215): m_sprite (sf::Sprite), width, height, escala
- [IShader](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L269): m_shader, uniform map with type erasure (void*)
- [IBoundingBox](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L440): tam (Vector2D), mitad (Vector2D)
- [IRespawn](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L468): timer_actual/maximo, curr_spawn, max_spawn, width, height, prefab
- [IPaths](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L527): puntos (Bezier), addCurva(), id_curva, frame counters
- [ITimer](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L108): frame_actual, frac_actual, frame_maximo
- [ITexto](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L180): m_texto (sf::Text)
- [INombre](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Componentes/IComponentes.hpp#L35): nombre (string)

**Objects**: [Objeto](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Primitivos/Objetos.hpp) extends `sf::Drawable` + `sf::Transformable`. Stores components with `addComponente<T>()` / `getComponente<T>()` using RTTI (dynamic_cast). [Entidad](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/objetos/Entidad.hpp) extends `CE::Objeto` for game-specific behavior.

**Key pattern**: Attach components at runtime, query by type. Enables composition-based design without inheritance chains.

## Singleton Managers

All follow the same pattern - static `Get()` returns mutable instance, private constructor.

- **[Render](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Render/Render.hpp)**: RenderWindow, RenderTexture (post-process), drawable queue. Call `Render::Get().GetTextura()` to draw to game surface. `DrawGameTexture()` stretches game output to window maintaining 16:9 aspect ratio with letterbox/pillarbox. `GetMouseGamePos()` returns mouse in game-space coordinates (1920×1080).
- **[GestorAssets](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Primitivos/GestorAssets.hpp)**: Cache for `sf::Font`, `sf::Texture`, `sf::Sound`, `sf::Music`. Load with `agregarFont(key, path)`, retrieve with `getFont(key)`.
- **[GestorEscenas](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Primitivos/GestorEscenas.hpp)**: Scene registry + activation. Register with `registrarEscena(key, scene)`, switch with `cambiarEscena(key)`.
- **[GestorCamaras](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Camaras/CamarasGestor.hpp)**: Camera/viewport management. `setCamaraActiva(id)`, `getCamaraActiva()`, `agregarCamara(shared_ptr<Camara>)`.

## Input Handling

[InputBuffer](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Inputs/InputBuffer.hpp) - circular queue of up to 32 SFML events. Motor collects events → buffer, Juego consumes via `OnInputs(InputBuffer&)`.

`IControl` component tracks 8 directions + 4 action buttons (boolean flags). Game code calls `SistemaControl()` to update based on inputs.

Input patterns in `Escena_Sprite`:
- Keyboard polling each frame with `sf::Keyboard::isKeyPressed` (avoids sticky keys)
- Gamepad mixed (OR) with keyboard state; uses `sf::Joystick::getAxisPosition` with 25.f deadzone, button 10 = attack
- Mouse: `sf::Mouse::isButtonPressed(Left)` sets `ctrl->acc`
- Mouse direction computed each frame: `CE::Render::Get().GetMouseGamePos()` → `mapPixelToCoords()` → normalize → `setMouseDirection(nx, ny)` on entities

## Key Game Systems (Sistemas)

Located in [Sistemas.hpp](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Sistemas/Sistemas.hpp) — namespace IVJ, marked `[[maybe_unused]]`:

**Movement & Physics**:
- `SistemaControl(obj, dt)` — Map IControl flags to ITransform velocity.
- `SistemaMover(vector<Objeto>, dt)` — Apply velocity to position (simple).
- `SistemaMover(shared_ptr<Objeto>, dt)` — Full mover: normalizes diagonal input, applies velocity, computes rotation from mouse position, handles run multiplier.

**Collision**:
- `SistemaColAABB(A, B, resolve)` — AABB overlap check. `resolve=true` resets `A.posicion = pos_prev`.
- `SistemaColAABBMid(A, B, resolve)` — Center-to-center overlap using half-extents.
- `checkRayoHit(npc, p1, p2)` — Ray vs AABB hit.

**AI & Navigation**:
- `SistemaZombieSeguir(zombie, target_pos, obstacles, dt)` — Full zombie AI: range check, LOS verification, obstacle avoidance (±45° probes), rotation toward target/movement, velocity application.
- `checkLOS(p1, p2, obstacles, ignore)` — Slab-method ray vs AABB; returns false if any obstacle blocks the ray.
- `getBlockingObstacle(p1, p2, obstacles, ignore)` — Returns the blocking obstacle if checkLOS fails.
- `SistemaNPCLookTarget(obj, target)` — Rotate NPC to face target if in aggro range.

**Spawning & Paths**:
- `SistemaSpawn(game_pool, spawn_zones, zombie_clips, player_target, zombies_por_generar)` — Reads IRespawn from spawn_zones pool, creates Entidad zombies with full component setup (ISprite, FSM, IBoundingBox, IRangoAggro, IEntorno, ITarget, ITexto, IShader, and IZombieSpawnerRef linking back to the spawning zone).
- `SistemaPaths(vector<Objeto>)` — Bezier curve movement; changes shader when curve segment changes.

**Combat & Scoring**:
- `SistemaAtaqueMelee(atacante, game_pool, puntaje, zombies_matados)` — Computes hitbox center ahead of player using `sin/cos(angulo)`, circle overlap vs all ITarget entities, tracks hits in `objs_golpeados`, spawns BloodSplash FX, plays swing/slash SFX. Resets `swing_played` and `objs_golpeados` when `atacando == false`. Award 100 points and increments zombie kill counter on zombie death. Spawns health pickup with 50% probability upon knife kill.
- `SistemaDisparar(tirador, cuerpo, arma, game_pool, bala_tex)` — Spawns IBala bullet entity in direction of cuerpo's angle; calls `arma.iniciarDisparo()`.
- `SistemaActualizarBalas(game_pool, dt)` — Moves all IBala entities, tracks `distancia_viajada`, marks hp=0 when `rango_max` exceeded.
- `SistemaColBalas(game_pool, puntaje, zombies_matados)` — Detects IBala vs ITarget collisions, applies damage, spawns BloodSplash FX, deactivates bullet. Awards 50 points and increments zombie kill counter on zombie death. Spawns ammo pickup with 25% probability upon bullet kill.

**Dialog & Interaction**:
- `SistemaDialogos(dialogo, obj)` — Activates IDialogo component.
- `revisarDistanciaInteraccion(jugador, npc, dist_max)` — Distance check for NPC interaction range.

**Debug**:
- `pintarLinea(p1, p2, color)` — Draws lerp-sampled line using tiny CircleShapes.

**Lab systems**:
- `SistemaGirar`, `SistemaRectanguloMoverUpDown`, `SistemaTrianguloWave`, `SistemaCirculoMoverEnCirculos`.

## Scene System

**File**: [Escena.hpp](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Motor/Primitivos/Escena.hpp) — Abstract base `Escena`.

Required methods: `onInit()`, `onFinal()`, `onUpdate(float dt)`, `onRender()`, `onInputs(*)`. Contains `CE::Pool objetos` for entities and button→action mapping via `registrarBotones(scancode, "name")`.

Concrete scenes in `src/Juego/Escenas/`:
- [Escena_Menu](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Escenas/Escena_Menu.hpp) — Main menu with music, option selection (Iniciar / Salir), scene transitions.
- [Escena_Prueba](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Escenas/Escena_Prueba.hpp) — Simple test scene with basic objects.
- [Escena_Sprite](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Escenas/Escena_Sprite.hpp) — Main gameplay: round systems, player (feet+body split), zombies, tilemap, fog shader, melee/gun combat, spawn zones, drops, HUD, pause overlay, and retry screens.
- [Escena_Camara](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Escenas/Escena_Camara.hpp) — Camera demo scene showing different follow modes.

### Escena_Sprite architecture
The player is split into two `Entidad` objects rendered in order (feet below, body above):
- `jugador_ref` — feet sprite + IControl + IBoundingBox + IEmisor (bullet emitter) + FSM (IdlePies/MoverPies)
- `cuerpo` — knife/gun sprite + IControl (synced from jugador_ref each frame) + IArmaMelee + IPistola + IInventario + FSM (IdleCuerpoKnife/MoverCuerpoKnife/AtacarCuerpoKnife or pistol equivalents)

Tilemap layers: `floors.txt` (visual floor) + `walls_colisiones.txt` (visual walls + collision boxes + spawn zones).
Fog shader (`fog.frag`) uses lamp position, radius, softness, noise params to create darkness effect around player.

## Game-Specific Components (IJComponentes)

Located in [IJComponentes.hpp](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Componentes/IJComponentes.hpp) — extends `CE::IComponentes` for game logic:

**FSM & AI**:
- `IMaquinaEstado`: holds `shared_ptr<FSM>`, `congelar` flag (freeze prevents state transitions).
- `ITarget`: pointer to target `CE::Vector2D* pos` and `CE::Objeto* target_obj`. `setTarget(CE::Objeto&)`.
- `IRangoAggro`: detection radius, `sf::CircleShape marcador` (debug). `estaDentroRango(parent, target)` using Pythagoras.
- `IEntorno`: const reference to a pool vector (for LOS obstacle checking).

**Interaction & Dialog**:
- `IInteractuable`: abstract base; `onInteractuar(CE::Objeto&)`, `interactuado` flag.
- `IIndicador`: visual indicator above NPCs (sprite + prueba.frag shader with blob texture and mask color). `IIndicador::clonar()` returns nullptr (sf::Shader is non-copyable).
- `IDialogo`: dialog box with `wstring texto`, auto line-wrap via `agregarSaltoLinea(str, 76)`, renders black rect + sf::Text overlay.

**Movement & Behavior**:
- `IGirar`: angle + radius for orbital spinning.
- `IArribaAbajo`: minY, maxY, velocidad (bounces between limits).
- `IWave`: amp, freq, phase (sinusoidal XY motion).
- `ICirculos`: radio, velocidad (circular orbit).
- `ITriangulo`: triangle visual helper component.

**Combat**:
- `IArmaMelee`: radio (hitbox radius), offset_adelante, damage, `atacando` flag, `swing_played` flag, `objs_golpeados` (unordered_set to prevent multi-hit), `hitbox_debug` (CircleShape).
- `IRayo`: start/end points with magnitude, direction reference.
- `IBala`: damage, rango_max, distancia_viajada, `CE::Objeto* origen` (to avoid self-hit), `activa` flag.
- `IPistola`: balas_cargador/max_cargador/reserva, cooldown_disparo, timer_disparo, tiempo_recarga, timer_recarga, `disparando`, `recargando` flags. Methods: `puedeDisparar()`, `iniciarDisparo()`, `iniciarRecarga()`, `onUpdate(dt)`.

**Inventory**:
- `IItem`: tipo (Cuchillo/Pistola/Rifle/Escopeta), nombre, key_textura, json_anims, sprite dimensions, escala — data needed to hot-swap the body FSM/sprite when equipping.
- `IInventario`: `vector<shared_ptr<IItem>>` items, idx_activo, `agregarItem()`, `getItemActivo()`, `ciclarSiguiente()`, `ciclarAnterior()`.

**Particle & Temporary Effects**:
- `IEmisor`: particle emitter — references parent entity and prefab; `crearParticula()` spawns copies with rotation/velocity matching parent angle (8-direction detection using rounded atan2 values); internal Pool tracks living particles.
- `ITemporal`: marker component — set `done = true` when effect finishes (signals scene to remove object from pool).
- `ICTimer`: frame-based timer (curr_frame, max_frame).
- `IEstadoEpidemia`: epidemic simulation states (Normal/Enfermo/Recuperacion/Inmune/Muerto), timers, probabilities, grid position (fila, columna), `infectar()`, `puedeSerInfectado()`.

**Drops, HUD, and Links**:
- `IDropItem`: Represents health or ammo drops picked up by collision.
- `IDelayedSpawn`: Delays drop spawning (e.g., 2.0s delay before drop becomes visible and collectible).
- `IFloatingText`: Fades out text (e.g., "+50 HP", "+16 Municion") drifting upwards.
- `IZombieSpawnerRef`: Links zombie entities back to their spawn zone so that spawner cooldown triggers upon zombie death.

## FSM Architecture

**Base class**: [FSM.hpp](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Maquinas/FSM.hpp)
- `virtual FSM* onInputs(const CE::IControl&)` — returns new FSM state or nullptr
- `virtual FSM* onInputs(const CE::IControl&, float mouse_nx, float mouse_ny)` — default delegates to above
- `virtual void onEntrar/onSalir/onUpdate(const Entidad&, float dt)`
- `std::string nombre` — used by `Entidad::inputFSM()` to dispatch to correct overload

**Specialized bases**:
- [FSMBoss](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Maquinas/FSMBoss.hpp) — `onInputs(Entidad& parent, CE::Vector2D& target)` for boss AI
- [FSMZombie](file:///Users/lukas/CLionProjects/CimaEngine3v-LC/src/Juego/Maquinas/FSMZombie.hpp) — `onInputs(Entidad& self, CE::Vector2D& target)` for zombie AI; `inputFSM()` detects "Zombie" in nombre. Contains the `ZombieConst` parameters:
  - `RANGO_AGGRO` = 10000.f
  - `VELOCIDAD` = 250.f
  - `DISTANCIA_ATAQUE` = 35.f
  - `DISTANCIA_IDLE` = 250.f
  - `FRAME_RATE_ATTACK` = 0.06f
  - `FRAME_RATE_MOVE` = 0.03f

**Dispatch in `Entidad::inputFSM()`**:
1. nombre contains "Boss" → static_cast to FSMBoss, pass `*ITarget->pos`
2. nombre contains "Zombie" → static_cast to FSMZombie, pass `*ITarget->pos`
3. Otherwise → pass IControl + mouse_dir_x/y

**Player feet FSMs** (`src/Juego/Maquinas/Pies/`):
- `IdlePies(clip, rate)` — Shows frame 0. Transitions to MoverPies based on input + mouse direction (computes dot/perp products to choose walk vs strafe).
- `MoverPies(clip, rate)` — Animates walk/strafe/run. Uses mouse direction to pick: walk {61,20}, strafe_left {21,20}, strafe_right {41,20}, or faster frame rate for run.

**Player body FSMs** (`src/Juego/Maquinas/Cuerpo/`):
- `IdleCuerpoKnife` → AtacarCuerpoKnife (acc) or MoverCuerpoKnife (movement).
- `MoverCuerpoKnife` → IdleCuerpoKnife (no input) or AtacarCuerpoKnife (acc).
- `AtacarCuerpoKnife` — One-shot attack animation {20,15}; sets `IArmaMelee::atacando=true`; exposes `shouldDealDamage()` / `markDamageDealt()` API for damage window timing.

**Zombie FSMs** (`src/Juego/Maquinas/Zombie/`):
- `IdleZombie` → MoverZombie if target in RANGO_AGGRO (250) AND checkLOS passes.
- `MoverZombie` → AtacarZombie if dist < DISTANCIA_ATAQUE (45), IdleZombie if dist > DISTANCIA_IDLE (250).
- `AtacarZombie` — One-shot attack animation, forces velocity=0; returns to Idle/Mover after completion.

**Pistol body FSMs** (`src/Juego/Maquinas/Pistola/`):
- `IdleCuerpoPistola`, `MoverCuerpoPistola`, `DispararCuerpoPistola`, `RecargarCuerpoPistola`.
- Set/clear `IPistola::disparando` and `IPistola::recargando` in onEntrar/onSalir.

**FX FSMs** (`src/Juego/Maquinas/FX/`):
- `BloodSplash(clip, rate)` — One-shot animation; sets `ITemporal::done = true` when finished; used for melee hit particles.

**Boss FSMs** (`src/Juego/Maquinas/Bosses/`):
- `IdleBoss` ↔ `AttackBoss` based on distance < 200.

## Rendering & GUI

Objects must inherit `sf::Drawable` and implement `draw()`. Motor calls `Render::Get().AddToDraw(obj)` to queue renderables. Also `AddToDraw(obj, sf::RenderStates)` for shader-equipped objects.

`Render` uses a `sf::RenderTexture` (1920×1080) as intermediate target. `DrawGameTexture()` renders that texture to the window with a `window_view` that maintains 16:9 aspect ratio with letterboxing/pillarboxing.

GUI layers via `CE::GLayer` (ImGui windows) added to Motor's `gui_layers` vector. In Debug mode: `GLogger`, `GListaObjetos` (object inspector with scene/camera selectors), `GPropiedades` (component editor), `GViewport` debug panels auto-loaded.

## Asset Loading Patterns

Typical scene init (e.g., `Escena_Sprite.cpp`):
```cpp
CE::GestorAssets::Get().agregarTextura("pies",
    ASSETS "/sprites/Top_Down_Survivor/feet/survivor-feet-atlas.png",
    CE::Vector2D{0.f, 0.f}, CE::Vector2D{3456.f, 3456.f});
CE::GestorAssets::Get().agregarFont("test", ASSETS "/fonts/Eater.ttf");
auto& tex = CE::GestorAssets::Get().getTextura("pies");
```

Audio:
```cpp
CE::GestorAssets::Get().agregarSonido("slash01", ASSETS "/sfx/...wav");
CE::GestorAssets::Get().getSonido("slash01").play();
CE::GestorAssets::Get().agregarMusica("bgm", ASSETS "/music/...mp3");
CE::GestorAssets::Get().getMusica("bgm").play();
CE::GestorAssets::Get().getMusica("bgm").setVolume(20.f);
CE::GestorAssets::Get().getMusica("bgm").setLooping(true);
```

Animation metadata (JSON → clip map):
```cpp
auto clips = CE::AnimLoader::cargar(ASSETS "/sprites/.../indices_animacion.json");
CE::AnimClip clip = CE::AnimLoader::getClip(clips, "idle");
// AnimClip has {first_frame, frame_count}
```

`ASSETS` macro = compile-time path to `assets/` folder.

## Tilemap System (`src/Juego/objetos/TileMap`)

Two load overloads:
- `loadTileMap(path)` — Visual-only, reads `[Atlas_info]` header + `[ID_Matrix]`
- `loadTileMap(path, objetos_pool, spawn_pool)` — Extended format: each cell has `id col` pairs where:
   - `col == 'n'` → no collision, visual only
   - `col == 'c'` → collision: creates Entidad with IBoundingBox in objetos_pool
   - `col == 's'` → spawn zone: creates Entidad with IRespawn in spawn_pool

Tilemap header format:
```
[Atlas_info]
/atlas/texture.png
<atlas_width> <atlas_height>
<tile_width> <tile_height>
<map_rows> <map_cols>
[ID_Matrix]
id col id col ...
```

Rendered as sf::VertexArray (triangles). Supports optional shader via `loadTileMapShaders(vert, frag)`.

## Camera System (`src/Motor/Camaras/`)

Four camera types (register with `GestorCamaras::Get().agregarCamara()`):
- `CamaraTopDownLock` — Direct position lock, no smoothing (id=0 in Juego)
- `CamaraLERP(pos, dim)` — Smooth follow with lerp factor (id=1 in Juego)
- `CamaraSnapVentana(pos, dim, vdim)` — Dead-zone window: camera snaps when player exits inner window (id=2)
- `CamaraSnapLR(pos, dim, vdim)` — Mario World style: state machine with smooth lerp transitions on direction change (id=3)

All cameras support `lockEnObjeto(shared_ptr<Objeto>)` and `setViewSize(x, y)`.

## Code Organization

```
Motor/
  ├─ App/ALayer.hpp         # GameLayer abstract base
  ├─ Componentes/           # Component definitions (IComponentes.hpp/.cpp)
  ├─ Render/Render.hpp      # Singleton renderer (16:9 letterbox, game texture)
  ├─ Primitivos/            # ECS: Objeto, Pool, Escena, GestorEscenas, GestorAssets
  ├─ GUI/                   # ImGui integration layers (GLogger, GListaObjetos, etc.)
  ├─ Inputs/                # InputBuffer, Botones
  ├─ Camaras/               # Camera variants + GestorCamaras
  ├─ Utils/                 # Vector2D, AnimLoader, Lerp, Lista<T>, MotorConfig
  └─ Motor.cpp/hpp          # Main loop orchestrator

Juego/
  ├─ objetos/               # Entidad (inputFSM dispatch, copy ctor), TileMap, Texto
  ├─ Componentes/           # IJComponentes.hpp/.cpp (all IVJ-specific components)
  ├─ Escenas/               # Scene implementations
  ├─ Sistemas/              # Sistemas.hpp/.cpp (all game systems)
  ├─ Figuras/               # Shape classes for lab exercises
  ├─ Maquinas/              # FSM (finite state machines)
  │  ├─ Pies/               # IdlePies, MoverPies (mouse-aware strafe selection)
  │  ├─ Cuerpo/             # IdleCuerpoKnife, MoverCuerpoKnife, AtacarCuerpoKnife
  │  ├─ Zombie/             # IdleZombie, MoverZombie, AtacarZombie
  │  ├─ Pistola/            # IdleCuerpoPistola, MoverCuerpoPistola, etc.
  │  ├─ Balas/              # BalasIdle, BalasBoom
  │  ├─ Naves/              # IdleJugadores, MoverJugadores
  │  ├─ Bosses/             # IdleBoss, AttackBoss
  │  ├─ FX/                 # BloodSplash (sets ITemporal::done when done)
  │  ├─ FSM.hpp             # Base FSM (onInputs + mouse overload)
  │  ├─ FSMBoss.hpp         # Boss-specific FSM base
  │  ├─ FSMZombie.hpp       # Zombie-specific FSM + ZombieConst (speeds/ranges)
  └─ Juego.cpp/hpp          # GameLayer implementation
```

## C++ Standard & Compiler Flags

- **Standard**: C++17
- **Debug**: `-ggdb -Wall -Wextra -Werror` (strict + debug symbols)
- **Release**: `-O2`
- **RTTI required**: extensive use of `dynamic_cast<T*>()` in component system

## Advanced Patterns & Systems

### Particle Effects & Temporary Objects

Pattern: `IEmisor` spawns particles, `ITemporal::done` signals removal.

1. Create prefab Entidad with ISprite + ITimer + IMaquinaEstado.
2. Parent entity gets `IEmisor(parent, prefab)` component.
3. `IEmisor::crearParticula()` copies prefab, sets direction from parent angle (8-direction via rounded atan2), assigns BalasIdle FSM, adds to internal pool.
4. BloodSplash FSM plays one-shot animation, sets `ITemporal::done = true`.
5. Scene's `onUpdate()` `std::erase_if` removes objects where `ITemporal::done == true`.

### Melee Combat & Score/Drops

Scene calls `SistemaAtaqueMelee(*cuerpo, objetos, puntaje, zombies_matados)` every frame in `onUpdate()`.

1. Check `IArmaMelee::atacando`; if false: clear `objs_golpeados` and `swing_played`, return.
2. Compute hitbox center: `{pos.x + sin(angle)*offset, pos.y - cos(angle)*offset}`.
3. Circle-circle overlap against all ITarget entities in pool (radius: arma.radio + 20).
4. On hit: apply damage, insert to `objs_golpeados` (prevent re-hit), spawn BloodSplash entity in game_pool, update ITexto HP label.
5. Play slash SFX if hit, swing SFX if miss (gated by `swing_played`).
6. If hit reduces HP to 0: increments `zombies_matados`, awards `puntaje += 100`, and has a 50% probability of dropping a `IDelayedSpawn` Health item.

### Bullet Collision, Damage, and Ammo Drops

Scene calls `SistemaColBalas(game_pool, puntaje, zombies_matados)` every frame in `onUpdate()`.

1. Collects active bullets. For each bullet vs ITarget entities:
2. Checks collision using `SistemaColAABBMid()`.
3. On hit: deactivates bullet, applies damage, plays hurt sound effects, spawns `BloodSplash` FX.
4. If target HP drops to 0: increments `zombies_matados`, awards `puntaje += 50`, and has a 25% probability of spawning a `IDelayedSpawn` Ammo item.

### Drops & Floating Texts

When a drop item is triggered (via `IDelayedSpawn` countdown ending):
1. Spawn `IDropItem` (either `Salud` or `Municion`) and display `[ + HP + ]` or `[ + AMMO + ]` on the map.
2. When the player collides with it: item is marked `ITemporal::done = true` to purge.
3. A floating text entity is created with `IFloatingText`, drifting upwards and fading out using delta time, displaying `+50 HP` or `+16 Municion`.

### Zombie Spawners and Timers

Each zombie spawned carries an `IZombieSpawnerRef`. When a zombie dies:
1. The scene locates the linked spawner zone entity.
2. Resets the spawner's `IRespawn` component timers (`timer_maximo = 120` (2s), `timer_actual = 0`) to trigger the next spawn under the round cap limits.

### Pause Menu and Game Over Reset States

`Escena_Sprite` manages game flow control:
1. **Pause state**: Pressing `Escape` toggles `en_pausa`. This pauses `musica_fondo`, plays `musica_pausa`, and draws a semi-transparent black rectangle with option prompts (Continuar / Salir). Selection is triggered by Enter (`confirmar`).
2. **Game Over / Reset**: If player HP is 0, the game enters `jugador_muerto`. Pressing Enter triggers `resetJuego()`, which:
   - Restores player HP (200) and moves them back to (1750, 1750).
   - Resets active weapon clips (8/32 ammo for pistols).
   - Resets round status, kills, and score.
   - Clears all spawned targets, drops, float texts, and bullets from the pool while preserving static walls.
   - Resets spawn zone cooldowns.

## Common Editing Patterns

1. **Add component to entity**: `entity->addComponente(std::make_shared<ISprite>(...))`, retrieve with `entity->getComponente<ISprite>()`
2. **Attach system logic to scene**: Add function calls in `Escena::onUpdate()`
3. **Map inputs to actions**: `registrarBotones(sf::Keyboard::Scancode::W, "arriba")` in `onInit()`
4. **Create new scene**: Extend `CE::Escena`, implement virtuals, register in `Juego::OnInit()`
5. **Debug visualization**: Use `pintarLinea()` or ImGui panels (enabled in Debug build)
6. **Add interaction NPC**: Give Entidad `IInteractuable`, `IIndicador`, `IDialogo`; scene checks `revisarDistanciaInteraccion()` and calls `SistemaDialogos()`
7. **Spawn particles**: Attach `IEmisor(parent, prefab)` to parent; call `crearParticula()` from FSM; mark particles with `ITemporal`
8. **Melee attack**: Equip entity with `IArmaMelee`; add attack state to FSM setting `atacando=true`; scene calls `SistemaAtaqueMelee()` each frame
9. **Gun attack**: Equip entity with `IPistola`; call `SistemaDisparar()` on trigger; call `arma.onUpdate(dt)` + `SistemaActualizarBalas()` + `SistemaColBalas()` each frame
10. **Add inventory**: Attach `IInventario` + multiple `IItem` components; `ciclarSiguiente/Anterior()` to switch; swap cuerpo FSM/sprite on item change

## Critical Dependencies

- **SFML 3.0.x**: FetchContent downloads at CMake time. Graphics, Audio, Window, System modules.
- **ImGui v1.91.9b-docking**: ImGui-SFML bridge (v3.0) manages window integration
- **Standard library**: `<memory>` (unique_ptr, shared_ptr), `<map>`, `<vector>`, `<optional>`, `<algorithm>`, `<cmath>`

## Quick Debugging

- **Rebuild**: `cd build && cmake --build . --target CimaEngine3v -- -j 8`
- **Recompile after header edits**: Clear build folder or `touch src/file.hpp`
- **Check symbols**: `nm bin/Debug/CimaEngine3v | grep Symbol`
- **Enable debug UI**: Ensure `DEBUG` compile flag in CMakeLists.txt line 13
- **Input issues**: Check `InputBuffer` state in `Motor::OnEventFrame()` and `Juego::OnInputs(buffer)`
- **FSM dispatch**: Check `Entidad::inputFSM()` — nombre must contain "Boss", "Zombie", or neither for correct dispatch
- **Shader compile fail**: IShader constructor calls `exit(EXIT_FAILURE)` on load failure