# L'arc de Noé

## Introduction

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

## Definition du template de problem.txt

Disclaimer : Si le fichier qui définit pas le probleme n'est pas conforme au template le programme ne pourra pas parse correctement le problem et donc correctement fonctionner.
Les sections de lignes avec "//" sont des commentaires ou les alternatives possibles et ne doivent pas être inclues dans le fichier.

```txt
Problem type    : Planar_Stress // Planar_Strain ou Axisym
E               : 2.1100000e+11 // Module de Young du materiau
nu              : 3.0000000e-01 // Coefficient de Poisson entre -1 et 1/2 (exclus)
rho             : 7.8500000e+01 // La densité du matériaux
g               : 9.8100000e+00
T               : 1.0000000e+02 // La tension dans la corde [N/m]
Conditions de Neumann : 2
Type : Neumann \\ ou Dirichlet
Direction : X \\ Y ou XY
Domaine : NomDomaine \\ Nome du domaine de la conditions
Valeur : 1.0000000e+00 \\ Valeur de la condition
```
