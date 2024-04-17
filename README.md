# Gestion automatique de la porte des poules

Ce projet consiste en un programme C++ qui gère automatiquement l'ouverture et la fermeture de la porte du poulailler en fonction de l'heure de la journée. Le programme récupère les horaires de début de jour et de début de nuit à partir d'une base de données et contrôle un système de verrin pour actionner la porte.

## Schéma du système

```
+------------------+
|  Raspberry Pi    |
|                  |
|  +------------+  |
|  | Programme  |  |
|  | C++        |  |
|  +------------+  |
|                  |
|  +------------+  |
|  | WiringPi   |  |
|  +------------+  |
|                  |
+------------------+
         |
         |
         |
         v
+------------------+
|     Relais       |
+------------------+
         |
         |
         |
         v
+------------------+
|     Verrin       |
+------------------+
         |
         |
         |
         v
+------------------+
|  Porte poulailler|
+------------------+
```

Le programme C++ s'exécute sur un Raspberry Pi et utilise la bibliothèque WiringPi pour contrôler les broches GPIO. Il est connecté à un système de relais qui permet d'actionner le verrin. Le verrin est relié à la porte du poulailler et permet de l'ouvrir ou de la fermer.

## Fonctionnement du programme

1. Le programme se connecte à une base de données pour récupérer les horaires de début de jour et de début de nuit.
2. Il récupère l'heure actuelle du système.
3. En fonction de l'heure actuelle et des horaires de début de jour et de nuit, il détermine si c'est le jour ou la nuit.
4. Si c'est le jour et que la porte est fermée, il actionne le verrin pour ouvrir la porte.
5. Si c'est la nuit et que la porte est ouverte, il actionne le verrin pour fermer la porte.
6. Le programme met à jour l'état de la porte dans la base de données (ouvert ou fermé).
7. Le programme attend 10 secondes avant de recommencer le cycle.

## Configuration de la base de données

Le programme nécessite une base de données pour stocker les horaires de début de jour et de début de nuit, ainsi que l'état de la porte (ouvert ou fermé). Voici la structure de la table `PORTES` utilisée dans le programme :

```
+------------+-------------+------+-----+---------+----------------+
| Field      | Type        | Null | Key | Default | Extra          |
+------------+-------------+------+-----+---------+----------------+
| id         | int         | NO   | PRI | NULL    | auto_increment |
| debut_jour | varchar(50) | YES  |     | NULL    |                |
| debut_nuit | varchar(50) | YES  |     | NULL    |                |
| ouvert     | tinyint     | YES  |     | NULL    |                |
| ferme      | tinyint     | YES  |     | NULL    |                |
+------------+-------------+------+-----+---------+----------------+
```

## Configuration du matériel

Le programme utilise les broches GPIO suivantes pour contrôler le système de relais :
- `PIN_RELAI_1` (broche 4)
- `PIN_RELAI_2` (broche 5)

Il est nécessaire de connecter correctement les relais aux broches correspondantes du Raspberry Pi et de relier les relais au verrin pour actionner la porte.

## Installation et utilisation

1. Clonez ce dépôt sur votre Raspberry Pi.
2. Installez les dépendances nécessaires (`wiringPi`, `cppconn`, etc.).
3. Configurez les paramètres de connexion à la base de données dans le code source.
4. Compilez le programme à l'aide d'un compilateur C++ compatible. (compil.sh)
5. Exécutez le programme compilé sur votre Raspberry Pi. (start.sh)

Le programme s'exécutera en continu, gérant automatiquement l'ouverture et la fermeture de la porte du poulailler en fonction de l'heure de la journée.

N'hésitez pas à adapter le code source et la configuration en fonction de vos besoins spécifiques.

## Licence

Ce projet est sous licence MIT.
