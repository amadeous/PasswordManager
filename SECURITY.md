# Aspects sécuritaires

## Structure
Le programme présente la structure de fichiers suivante :

* masterPassword : programme principal
* data/ : dossier contenant les mdp chiffrés et le mot de passe principal
  * data/masterPassword : Hash du masterPassword et sel en base64
  * data/nomEnBase64 : Le nom du fichier correspond au keyword utilisé par l'utilisateur pour identifier le mot de passe, en base64. Chaque fichier correspond donc à un mot de passe. Dans chaque fichier, on y trouve le nonce utilisé pour chiffrer le mot de passe (en base64) ainsi que le mot de passe chiffré à l'aide de ce nonce.

La structure « un mot de passe / un fichier » a été choisie pour pouvoir facilement créer et récupérer des mots de passe. Pas besoin de parcourir une liste ou de parser un fichier. L'accès à un fichier est direct (dépendant du système d'exploitation également).
De cette manière, il est possible, avec un logiciel de versioning (VCS) comme git, de ne pas indexer (sauvegarder, exporter) tous ses mots de passe. De plus, cela permettrait, par la suite de regrouper les mots de passes dans des dossiers en fonction de l'usage ou d'envisager un partage de mot de passe, basé sur un partage de fichier.

## Description des états
Il y a 3 états dans lequel est le programme :

* **Not Running** : Le logiciel est dans l'état « Not Running » lorsqu'il n'est pas lancé, qu'il ne s'exécute pas.
* **Locked**: Le programme, une fois lancé, est par défaut en « Locked » lorsque l'utilisateur ne s'est pas authentifié (symbolisé dans le logiciel par la variable key étant à NULL).
* **Unlocked**: Une fois que l'utilisateur a saisi son mot de passe, qu'il a été vérifié et qu'une clef a été générée sur la base de son mot de passe et de son sel, il est considéré comme authentifié.

## Choix des algorithmes
La librairie [Libsodium]() a été sélectionnée pour l'implémentation de ses algorithmes cryptographiques.

### Hachage et fonction de dérivation de clef (KDF)
L'algorithme de hachage utilisé est [Argon2](https://github.com/P-H-C/phc-winner-argon2) ([Doc libsodium](https://download.libsodium.org/doc/password_hashing/the_argon2i_function)), vainqueur de la [Password Hashing Competition (PHC)](https://password-hashing.net) car il est récent et a fait ses preuves. Il est possible et aisé de modifier le ratio rapidité/sécurité en gardant une sécurité fiable.

De plus, il permet la dérivation de manière sécurisée, d'un mot de passe pour obtenir une clef de taille désirée pour faire du chiffrement (plus d'informations [ici](https://download.libsodium.org/doc/key_derivation#deriving-a-key-from-a-password)).

### Chiffrement
Le chiffrement a été réalisé à l'aide de [ChaCha20-Poly1305](https://download.libsodium.org/doc/secret-key_cryptography/aead/chacha20-poly1305) car cet algorithme propose du chiffrement authentifié et l'ajout de données additionnelle. Même si les données additionnelles ne sont pas utilisés dans la version actuelle, un usage futur pourrait en être fait (stockage d'un pseudo, d'une URL,... voir la [Roadmap](ROADMAP.md)).

La construction choisie est [XChaCha20-Poly1305](https://download.libsodium.org/doc/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction) car il s'agit de celle suggérée par la librairie ainsi que la construction la plus sûre pour l'utilisation de nonce random. Cette construction peut chiffrer des messages de tailles non limitées. Le mode combiné a été utilisé dans ce logiciel afin d'attacher le tag (pour l'intégrité) au message chiffré.

## Gestion de la mémoire
La majorité des allocations mémoires, pour des objets qui ne sont pas « sensibles » à savoir sel, nonce, mots de passe chiffrés ou hachés (qui sont stockés dans des fichiers et accessible en dehors de l'état « Unlocked ») sont stockés en mémoire dans des tableaux classiques ou des allocations mémoires classiques, réalisées avec des calloc (afin d'initialiser la mémoire à 0). La raison à cela est que les allocations mémoires protégées (voir le paragraphe ci-après) sont coûteuses et limitées pour chaque processus. Il faut donc les utiliser uniquement lorsque cela est nécessaire.

Les informations sensibles, tel que la clef de chiffrement/déchiffrement et les mots de passe (masterPassword et mots de passe à stocker) qui doivent être présents « en clair » dans le programme à un moment donné sont stockés dans une mémoire sécurisée par Libsodium grâce à deux méchanisme « [Guarded Heap allocations](https://download.libsodium.org/doc/memory_management#guarded-heap-allocations) » et « [Memory Locking](https://download.libsodium.org/doc/memory_management#locking-memory) ». Le logiciel présente donc des méthodes telles que __sodium_allocarray__ et __sodium_free__ pour l'allocation et la libération de mémoire sécurisée ainsi que les méthodes __sodium_mlock__ et __sodium_munlock__ pour protéger les pages des swaps et dump mémoire.

__Il est à noter que d'autres précautions peuvent être prises au niveau du système d'exploitation pour améliorer la protection contre les attaques sur la mémoire. Pour cela, consulter la documentation de libsodium, ci-après.__

Pour plus d'information, consulter la page de documentation sur le site de libsodium relative à la mémoire sécurisée ([lien](https://download.libsodium.org/doc/memory_management)). 

## Gestion de l'authentification

### Premier lancement
Le premier lancement est lorsque le fichier _data/masterPassword_ est absent.

L'utilisateur est invité à saisir un mot de passe maître, puis le même, une seconde fois, pour confirmation. Ce mot de passe est haché à l'aide d'[Argon2](#hachage-et-fonction-de-dérivation-de-clef-kdf).

Un sel est ensuite généré aléatoirement et utilisé avec le mot de passe maître pour générer une clef avec [Argon2](#hachage-et-fonction-de-dérivation-de-clef-kdf) (toujours). La clef a la taille requise pour le chiffrement avec [ChaCha-Poly1305](#chiffrement).

Le mot de passe haché et le sel, transformé en base64) sont stockés dans le fichier masterPassword. La clef est gardée en mémoire sécurisée alors que les mots de passe clair et haché ainsi que le sel sont supprimés de la mémoire. 

### Lancements ultérieurs

Lorsque l'utilisateur lance le programme une nouvelle fois ou se logue lorsque le programme a été verrouillé, il est invité à entrer son mot de passe. Une fois que cela est fait sans encombre, la hash du mot de passe stocké dans masterPassword est récupéré et le logiciel vérifie que le mot de passe entré correspond.

Si c'est le cas, il utilise le sel stocké dans le fichier masterPassword pour dériver la clef de la même manière que pour le premier lancement.

La clef est gardée en mémoire sécurisée alors que le mot de passe entré, le mot de passe haché ainsi que le sel sont supprimés de la mémoire.

### Changement de mot de passe maître

Lors d'un changement de mot de passe maître, on suppose l'utilisateur authentifié. On ne lui redemande donc pas son mot de passe (bien que cela puisse être fait). Il est invité à effectuer la même procédure que lors de la [première connexion](#premier-lancement) sauf que la clé précédente est conservée durant ce processus (car authentifié).

En effet, pour que l'utilisateur puisse récupérer ses mots de passe s'il change de mot de passe maître, il faut re-chiffrer chacun d'eux. Donc une fois que le logiciel est en possession de la vieille et la nouvelle clef, il va déchiffrer chaque mot de passe avec l'ancienne clef et le rechiffrer par après à l'aide de la nouvelle clef dans un fichier au nom similaire. Chaque mot de passe stocké en mémoire sécurisée est supprimé de la mémoire sécurisée au fur et à mesure.

L'ancienne clef est ensuite supprimée de la mémoire sécurisée.

En cas de problème lors du re-chiffrement d'un mot de passe (lecture du ficher existant, ré-écriture du fichier, déchiffrement ou re-chiffrement,...) le mot de passe est supprimé et il est indiqué à la fin du processus, le nombre de mot de passe perdus (mais cela ne devrait pas arriver). Une autre manière de faire pourrait être envisagée dans une version ultérieure : copier les mots de passes dans un endroit séparé et si tout a pu être récupéré, effectuer le changement de mot de passe et supprimer les mots de passe chiffrés avec l'ancienne clef.

## Contraintes
Les contraintes de hachage et chiffrement ont été choisies pour que le temps maximum pour se loguer soit ~5 secondes et celui pour ajouter ou récupérer un mot de passe soit presque instantané.

## Divers
### Choix de fgets pour la lecture des mots de passe
[fgets](https://linux.die.net/man/3/fgets) permet la lecture d'un nombre de char donné ou jusqu'à un retour à la ligne (y compris). Cela est parfait pour notre usage car il permet de stocker les mots clefs et mots de passe contenant des espaces. De plus, il gère les débordement de buffer et nous libère donc de cette contrainte supplémentaire.

### Choix de la librairie Base64

Après recherche et la lecture de [ce très intéressant article](https://github.com/gaspardpetit/base64/) de [gaspardpetit](https://github.com/gaspardpetit), j'ai opté pour la librairie d'Apple basée sur la librairie d'Apache pour sa simplicité d'utilisation.

Afin de limiter l'usage de librairie externe, l'utilisation du [helper Base64](https://download.libsodium.org/doc/helpers#base64-encoding-decoding) de la librairie Libsodium aurait été possible.