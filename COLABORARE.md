# Ghid de Colaborare și Protecție Branch

Acest ghid explică cum să adaugi un coleg pentru a lucra împreună la acest repository, asigurându-te că pot lucra doar în branch-uri separate, nu direct în `main`.

## Pasul 1: Adăugarea unui Colaborator

### Pentru repository-uri private:
1. Mergi la repository-ul pe GitHub: `https://github.com/viteacreator/First_prj_rp_picow`
2. Click pe tab-ul **Settings** (în partea de sus a paginii)
3. În meniul din stânga, click pe **Collaborators and teams**
4. Click pe butonul verde **Add people**
5. Introduceți username-ul GitHub al colegului sau adresa de email
6. Selectați nivelul de acces:
   - **Write** - permite colaboratorului să facă push în branch-uri, dar NU în branch-uri protejate
   - **Admin** - acces complet (nu recomandabil pentru colaboratori normali)
7. Click **Add [username] to this repository**

### Pentru repository-uri publice:
- Colaboratorii pot face fork la repository și crea Pull Requests fără a avea acces direct
- Poți folosi și metoda de mai sus pentru a da acces direct

## Pasul 2: Protejarea Branch-ului Main

După ce ai adăugat colaboratorul, trebuie să protejezi branch-ul `main`:

1. În **Settings** al repository-ului, mergi la secțiunea **Branches** (în meniul din stânga)
2. Sub **Branch protection rules**, click pe **Add rule** sau **Add branch protection rule**
3. În câmpul **Branch name pattern**, introduceți: `main`
4. Activează următoarele setări recomandate:

   ### Setări Obligatorii:
   - ✅ **Require a pull request before merging**
     - Acest lucru forțează toți colaboratorii (inclusiv tu) să creeze Pull Requests
     - Opțional: activează **Require approvals** (necesită review-uri înainte de merge)
   
   - ✅ **Require status checks to pass before merging** (dacă ai CI/CD)
     - Asigură că testele trec înainte de merge
   
   - ✅ **Do not allow bypassing the above settings**
     - Previne bypass-ul regulilor de către administratori

   ### Setări Opționale dar Recomandate:
   - ✅ **Require conversation resolution before merging**
     - Toate comentariile din review trebuie rezolvate
   
   - ✅ **Include administrators**
     - Aplică aceste reguli și pentru tine (foarte recomandat)
   
   - ✅ **Restrict who can push to matching branches**
     - Poți specifica doar anumite persoane/echipe care pot face push direct
     - Pentru a bloca complet push-urile directe, lasă această listă goală după activare

5. Click **Create** sau **Save changes**

## Pasul 3: Workflow de Lucru pentru Colaboratori

După configurarea protecției, colaboratorii tăi vor trebui să urmeze acest workflow:

### Pentru Colaborator:

1. **Clone repository-ul** (prima dată):
   ```bash
   git clone https://github.com/viteacreator/First_prj_rp_picow.git
   cd First_prj_rp_picow
   ```

2. **Creează un branch nou** pentru fiecare feature/task:
   ```bash
   git checkout -b feature/nume-descriptiv
   ```

3. **Lucrează și commit-ează schimbările**:
   ```bash
   # Faceți modificările necesare
   git add .
   git commit -m "Descriere clară a modificărilor"
   ```

4. **Push branch-ul pe GitHub**:
   ```bash
   git push origin feature/nume-descriptiv
   ```

5. **Creează un Pull Request**:
   - Mergi pe GitHub la repository
   - Vei vedea un banner galben care sugerează să creezi un Pull Request
   - Click pe **Compare & pull request**
   - Adaugă o descriere detaliată a schimbărilor
   - Click **Create pull request**

6. **Așteaptă review și merge**:
   - Colaboratorul NU poate face merge direct în `main`
   - Tu (ca owner) trebuie să revizuiești și să aprobi Pull Request-ul
   - După aprobare, poți face merge în `main`

### Pentru Tine (Owner):

1. **Revizuiește Pull Request-urile**:
   - Mergi la tab-ul **Pull requests** din repository
   - Click pe PR-ul pe care vrei să-l revizuiești
   - Verifică schimbările în tab-ul **Files changed**
   - Adaugă comentarii sau sugestii dacă este necesar

2. **Aprobă și Merge**:
   - Click pe **Review changes** → **Approve**
   - Click pe **Merge pull request**
   - Confirmă merge-ul

## Pasul 4: Sincronizarea Branch-ului Main

Colaboratorii trebuie să-și actualizeze periodic branch-ul main local:

```bash
# Schimbă pe branch-ul main
git checkout main

# Trage ultimele schimbări
git pull origin main

# Schimbă înapoi pe branch-ul de lucru
git checkout feature/nume-branch

# Opțional: integrează schimbările din main
git merge main
# SAU
git rebase main
```

## Verificarea Setărilor

Pentru a verifica că totul funcționează corect:

1. **Testează protecția branch-ului**:
   - Încearcă să faci push direct în `main`:
     ```bash
     git checkout main
     echo "test" >> test.txt
     git add test.txt
     git commit -m "test"
     git push origin main
     ```
   - Ar trebui să primești o eroare de tipul: `protected branch hook declined`

2. **Verifică accesul colaboratorului**:
   - Colaboratorul ar trebui să poată:
     ✅ Clona repository-ul
     ✅ Crea branch-uri noi
     ✅ Push în branch-uri proprii
     ✅ Crea Pull Requests
   - Colaboratorul NU ar trebui să poată:
     ❌ Push direct în `main`
     ❌ Merge Pull Requests (dacă nu are permisiuni de admin)
     ❌ Șterge branch-ul `main`

## Rezolvarea Problemelor

### "Permission denied" când colaboratorul încearcă să push-eze
- Verifică că ai adăugat colaboratorul corect în Settings → Collaborators
- Verifică că colaboratorul a acceptat invitația (primește un email)

### "Protected branch" error
- Acesta este comportamentul așteptat pentru push-uri directe în `main`
- Colaboratorul trebuie să creeze un branch și un Pull Request

### Colaboratorul nu poate crea branch-uri
- Verifică că are cel puțin permisiunea "Write" în repository
- Verifică că folosește comanda corectă: `git checkout -b nume-branch`

## Best Practices

1. **Nume descriptive pentru branch-uri**:
   - `feature/add-wifi-support`
   - `bugfix/fix-memory-leak`
   - `docs/update-readme`

2. **Commit messages clare**:
   - Scurte și descriptive
   - În limba engleză sau română (consistent în tot repository-ul)

3. **Pull Requests mici și focusate**:
   - Un PR ar trebui să rezolve o singură problemă
   - Mai ușor de revizuit și de testat

4. **Code Review**:
   - Revizuiește prompt Pull Request-urile
   - Oferă feedback constructiv
   - Testează schimbările locale înainte de merge

## Resurse Suplimentare

- [GitHub Docs: Managing access to repositories](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/managing-repository-settings/managing-teams-and-people-with-access-to-your-repository)
- [GitHub Docs: About protected branches](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches)
- [Git Branching Workflow](https://git-scm.com/book/en/v2/Git-Branching-Branching-Workflows)
