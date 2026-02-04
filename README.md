# First_prj_rp_picow

Proiect de control PID cu WiFi pentru Raspberry Pi Pico W / PID Control Project with WiFi for Raspberry Pi Pico W

## Despre Proiect / About Project

Acest proiect implementează un sistem de control PID cu server web pentru Raspberry Pi Pico W. Include:
- Control PID pentru simularea unui sistem fizic (plantă)
- Server web pentru interfață utilizator
- Manager WiFi pentru conectivitate
- mDNS pentru descoperire în rețea

This project implements a PID control system with web server for Raspberry Pi Pico W. It includes:
- PID controller for simulating a physical system (plant)
- Web server for user interface
- WiFi manager for connectivity
- mDNS for network discovery

## Colaborare / Collaboration

### Pentru Colaboratori / For Collaborators

Acest repository folosește protecție de branch pentru `main`. Te rugăm să citești:
- 📖 [Ghidul de Colaborare (Română)](COLABORARE.md) - Instrucțiuni detaliate în română
- 📖 [Contributing Guide (English)](CONTRIBUTING.md) - Detailed instructions in English

**Important:**
- ❌ NU poți face push direct în `main`
- ✅ POȚI crea branch-uri proprii și Pull Requests
- 🔍 Toate schimbările necesită review înainte de merge

**Important:**
- ❌ You CANNOT push directly to `main`
- ✅ You CAN create your own branches and Pull Requests
- 🔍 All changes require review before merging

### Workflow Rapid / Quick Workflow

```bash
# Clonează / Clone
git clone https://github.com/viteacreator/First_prj_rp_picow.git
cd First_prj_rp_picow

# Creează branch nou / Create new branch
git checkout -b feature/my-feature

# Fă modificări și commit / Make changes and commit
git add .
git commit -m "Descriere modificări"

# Push / Push
git push origin feature/my-feature

# Apoi creează Pull Request pe GitHub / Then create Pull Request on GitHub
```

## Structură Proiect / Project Structure

```
├── First_prj.c          # Main application file
├── pid.c/h              # PID controller implementation
├── plant.c/h            # Plant (system) simulation
├── sim_state.c/h        # Simulation state management
├── sim_worker.c/h       # Simulation worker thread
├── wifi_manager.c/h     # WiFi connection management
├── web_server.c/h       # Web server implementation
├── mdns_manager.c/h     # mDNS service discovery
├── ui_test.html         # Web UI for testing
├── CMakeLists.txt       # CMake build configuration
├── COLABORARE.md        # Ghid colaborare în română
├── CONTRIBUTING.md      # Contributing guide in English
└── .github/
    ├── CODEOWNERS       # Code ownership definitions
    └── pull_request_template.md
```

## Build

Acest proiect folosește Pico SDK. Pentru a compila:

```bash
mkdir build
cd build
cmake ..
make
```

## Configurare Hardware / Hardware Setup

- Raspberry Pi Pico W
- Conexiune WiFi necesară / WiFi connection required
- LED pe GPIO 25 pentru indicare status / LED on GPIO 25 for status indication

## Licență / License

[Specificați licența aici / Specify license here]

## Contact

Pentru întrebări sau probleme, deschide un issue pe GitHub.

For questions or issues, open an issue on GitHub.
