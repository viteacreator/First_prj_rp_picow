# Contributing Guide / Ghid de Contribuție

## English

Thank you for contributing to this project! This guide will help you get started.

### Branch Protection Rules

This repository has branch protection enabled on the `main` branch. This means:

- ✅ You CAN create and push to your own branches
- ✅ You CAN create Pull Requests
- ❌ You CANNOT push directly to `main`
- ❌ You CANNOT merge your own Pull Requests (maintainer approval required)

### Workflow

1. **Clone the repository** (first time only):
   ```bash
   git clone https://github.com/viteacreator/First_prj_rp_picow.git
   cd First_prj_rp_picow
   ```

2. **Create a new branch**:
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes and commit**:
   ```bash
   # Make your changes
   git add .
   git commit -m "Description of your changes"
   ```

4. **Push your branch**:
   ```bash
   git push origin feature/your-feature-name
   ```

5. **Create a Pull Request**:
   - Go to the repository on GitHub
   - Click "Compare & pull request"
   - Fill in the description
   - Submit the Pull Request

6. **Wait for review**:
   - The maintainer will review your changes
   - Address any feedback
   - Once approved, the maintainer will merge your PR

### Branch Naming Convention

Use descriptive names with prefixes:
- `feature/` - New features (e.g., `feature/add-wifi-support`)
- `bugfix/` - Bug fixes (e.g., `bugfix/fix-memory-leak`)
- `docs/` - Documentation updates (e.g., `docs/update-readme`)
- `refactor/` - Code refactoring (e.g., `refactor/simplify-pid-logic`)

### Commit Messages

- Use clear, descriptive messages
- Start with a verb in present tense (e.g., "Add", "Fix", "Update")
- Keep the first line under 50 characters
- Add detailed description if needed

Example:
```
Add WiFi configuration validation

- Validate SSID length
- Check password requirements
- Add error messages for invalid configs
```

### Code Style

This is a C project for Raspberry Pi Pico W. Please follow these guidelines:

- Use consistent indentation (spaces or tabs as per existing code)
- Add comments for complex logic
- Keep functions small and focused
- Test your changes before submitting

### Testing

Before submitting a PR:
- Ensure your code compiles without errors
- Test the functionality you're adding/changing
- Check that existing features still work

### Questions?

If you have questions or need help, feel free to:
- Open an issue on GitHub
- Ask in the Pull Request comments
- Contact the repository owner

---

## Română

Mulțumim pentru contribuția ta la acest proiect! Acest ghid te va ajuta să începi.

### Reguli de Protecție Branch

Acest repository are protecție activată pe branch-ul `main`. Asta înseamnă:

- ✅ POȚI crea și face push în branch-urile tale
- ✅ POȚI crea Pull Requests
- ❌ NU POȚI face push direct în `main`
- ❌ NU POȚI face merge la propriile Pull Requests (necesită aprobare de la maintainer)

### Workflow

1. **Clonează repository-ul** (doar prima dată):
   ```bash
   git clone https://github.com/viteacreator/First_prj_rp_picow.git
   cd First_prj_rp_picow
   ```

2. **Creează un branch nou**:
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feature/nume-feature
   ```

3. **Fă modificările și commit**:
   ```bash
   # Fă modificările
   git add .
   git commit -m "Descrierea modificărilor"
   ```

4. **Push branch-ul**:
   ```bash
   git push origin feature/nume-feature
   ```

5. **Creează un Pull Request**:
   - Mergi la repository pe GitHub
   - Click "Compare & pull request"
   - Completează descrierea
   - Trimite Pull Request-ul

6. **Așteaptă review**:
   - Maintainer-ul va revizui schimbările
   - Adresează feedback-ul primit
   - După aprobare, maintainer-ul va face merge

### Convenție Nume Branch-uri

Folosește nume descriptive cu prefixe:
- `feature/` - Funcționalități noi (ex: `feature/add-wifi-support`)
- `bugfix/` - Rezolvări bug-uri (ex: `bugfix/fix-memory-leak`)
- `docs/` - Actualizări documentație (ex: `docs/update-readme`)
- `refactor/` - Refactorizare cod (ex: `refactor/simplify-pid-logic`)

### Mesaje Commit

- Folosește mesaje clare și descriptive
- Începe cu un verb la prezent (ex: "Adaugă", "Rezolvă", "Actualizează")
- Păstrează prima linie sub 50 caractere
- Adaugă descriere detaliată dacă e necesar

Exemplu:
```
Adaugă validare configurație WiFi

- Validează lungimea SSID
- Verifică cerințele pentru parolă
- Adaugă mesaje de eroare pentru configurații invalide
```

### Stil Cod

Acesta este un proiect C pentru Raspberry Pi Pico W. Te rugăm să urmezi aceste linii directoare:

- Folosește indentare consistentă (spații sau tab-uri ca în codul existent)
- Adaugă comentarii pentru logica complexă
- Păstrează funcțiile mici și focusate
- Testează modificările înainte de a le trimite

### Testare

Înainte de a trimite un PR:
- Asigură-te că codul compilează fără erori
- Testează funcționalitatea pe care o adaugi/modifici
- Verifică că funcționalitățile existente încă funcționează

### Întrebări?

Dacă ai întrebări sau ai nevoie de ajutor:
- Deschide un issue pe GitHub
- Întreabă în comentariile Pull Request-ului
- Contactează owner-ul repository-ului
