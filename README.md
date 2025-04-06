# L'arc de Noé
Repo du projet : https://github.com/pier-rot/Projet-LEPL1110

Après avoir cloner la repo ou décompresser l'archive :
```bash
cd Projet-LEPL1110
cmake CMakeLists.txt
make
```
Par défaut, les fichiers `data.txt` et `problem.txt` dans le dossier `data` sont utilisés.

(Optionnel) On peut ensuite passer 2 chemins en arguments :
* Le chemin de la géométrie : <mesh_path>
* Le chemin de la description du problème : <problem_path>

Par exemple : `./bow ./data/mesh.txt ./data/problem.txt` (qui revient à faire `./bow` puisqu'il s'agit des chemins par défaut).