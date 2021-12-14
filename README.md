# Gestionnaire de mot de passe interactif

_Mai 2019_

Ce projet a été développé dans le cadre du cours optionnel Cryptographie Appliquée Avancée, dispensé à la HEIG-VD. L'énoncé est disponible ici : [lab_02](lab_02.pdf).

## Implémentation

Cette implémentation peut faire les choses suivantes :

- Créer un mot de passe maître lors de la première utilisation
- Être reconnu lors d'une future connexion avec ce mot de passe
- Changer de mot de passe maître
- Stocker des mots de passe de manières sécurisées
- Récupérer un mot de passe précédemment stocké
- Supprimer un mot de passe précédemment stocké

Un diagramme d'activité du logiciel est disponible ici : [Diagramme d'activité](activityDiagram/)   
Les questions de sécurité sont traitées ici : [SECURITY.md](SECURITY.md)

## Build

Pour construire le logiciel depuis les sources, exécuter les commandes suivantes :

```bash
git clone https://github.com/amadeous/PasswordManager.git
cd passwordManager/src && make
```

Il est possible de modifier la taille des mots de passe stockés en changeant la constante PW_SIZE dans le fichier _src/const.h_.   
Vous pouvez également modifier les constantes de [hachage](https://download.libsodium.org/doc/password_hashing/the_argon2i_function#constants) et [chiffrement](https://download.libsodium.org/doc/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction#constants) pour modifier le ratio temps/sécurité dans le fichier _src/const.h_ sur la base des valeurs indiquées dans la documentation de libsodium.

## Bugs connus
* Il y a parfois des comportements indéterminés lorsqu'un mot-clef contient des caractères spéciaux (mais les espaces sont gérés).

## Licence

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.

This programm includes software (Base64 library) developed by the Apache Group for use in the Apache HTTP server project (http://www.apache.org/).