# L'arc de Noé

## Introduction

Repo du projet : https://github.com/pier-rot/Projet-LEPL1110

Pour visualiser la géométrie et ses paramètres : https://www.desmos.com/calculator/urnvelt10o

Après avoir cloner la repo ou décompresser l'archive :

Sous linux :
```bash
cd Projet-LEPL1110
cmake CMakeLists.txt
make run
```
Par défaut, les fichiers `data.txt` et `problem.txt` dans le dossier `data` sont utilisés.

(Optionnel) On peut ensuite passer 2 chemins en arguments :

* Le chemin de la géométrie : <mesh_path>
* Le chemin de la description du problème : <problem_path>

Par exemple : `./bow ./data/mesh.txt ./data/problem.txt` (qui revient à faire `./bow` puisqu'il s'agit des chemins par défaut).

## Definition du template de problem.txt

Disclaimer : Si le fichier qui définit pas le probleme n'est pas conforme au template le programme ne pourra pas parse correctement le problem et donc correctement fonctionner.
Les sections de lignes avec "//" sont des commentaires ou les alternatives possibles et ne doivent pas être inclues dans le fichier.

```txt
Problem type    : 0
E               : 1.7200000e+10
nu              : 3.0000000e-01
rho             : 7.8500000e+03
g               : 9.8100000e+00
T               : 0.0000000e+00
Conditions : 3
Type : DIRICHLET_X
Domaine : HandleRight
Valeur : 0.0000000e+00
Type : DIRICHLET_Y
Domaine : Bottom
Valeur : 0.0000000e+00
Type : NEUMANN_Y
Domaine : AttachPoint
Valeur : -1.0000000e+07
```
