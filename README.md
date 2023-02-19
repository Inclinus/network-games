# Network-Games 

[![stars](https://custom-icon-badges.demolab.com/github/stars/Inclinus/network-games?logo=star)]()
[![size](https://custom-icon-badges.demolab.com/github/languages/code-size/Inclinus/network-games?logo=file-code&logoColor=white)]()
[![issues](https://custom-icon-badges.demolab.com/github/issues-raw/Inclinus/network-games?logo=issue)]()
[![last-commit](https://custom-icon-badges.demolab.com/github/last-commit/Inclinus/network-games?logo=history&logoColor=white)]()


Network-Games est un projet effectué en 2ème année d'études à l'ESGI Paris par Alexandre COMET, Noam DE MASURE et Thibaut LULINSKI.

Le but ? Jouer à des jeux en ligne en 1c1, comme le morpion ou encore le puissance 4.\
Nous avons ajouté l'utilisation de la librairie SDL2 pour nous permettre de rendre les jeux plus visuels que si tout était fait en ligne de commande.

## 📥 Installation et exécution

### <img src="https://camo.githubusercontent.com/b462a7f57c3b8941b927b4100f1d5c6ebfc95d21a3715e588321f7282d9398e2/68747470733a2f2f63646e2e7261776769742e636f6d2f63726f636f64696c656a732f666f6e742d617765736f6d652d6173736574732f6d61737465722f6d656469612f7261696e626f772f74656c65766973696f6e2e737667" alt= "" width="18" height="18"> Client

Pas d'inquiétude un script start.sh est mis a disposition et va vérifier et/ou installé les dépendances pour vous !

#### Librairie utilisée

- Librairie SDL2
- Librairie SDL2 Image
- Librairie SDL2 Ttf

### Prérequis

- Git

### <img src="https://camo.githubusercontent.com/0ce367415651b8e93f7c81caf2fd447bb5633ed2b5e58b82fd032a15539009fa/68747470733a2f2f63646e2e7261776769742e636f6d2f63726f636f64696c656a732f666f6e742d617765736f6d652d6173736574732f6d61737465722f6d656469612f7261696e626f772f6c696e75782e737667" alt= "" width="18" height="18"> Execution

```bash

# Clone le depot git
$ git clone https://github.com/Inclinus/network-games/

# Rendre le script éxécutable
$ chmod +x start.sh

# Lancer le script de démarrage
$ ./start.sh

```

### <img src="https://camo.githubusercontent.com/90064d1c3c59aff29f5cb6d6b092659abae88a05f43f46a480691010a8c65841/68747470733a2f2f63646e2e7261776769742e636f6d2f63726f636f64696c656a732f666f6e742d617765736f6d652d6173736574732f6d61737465722f6d656469612f7261696e626f772f7461736b732e737667" alt= "" width="18" height="18"> Serveur

Pas d'inquiétude un docker à été réalisé pour vous et tout est inclus dedans !

#### Librairie utilisée

- Librairie Mysql-Client

### Prérequis

- Docker

### <img src="https://camo.githubusercontent.com/0ce367415651b8e93f7c81caf2fd447bb5633ed2b5e58b82fd032a15539009fa/68747470733a2f2f63646e2e7261776769742e636f6d2f63726f636f64696c656a732f666f6e742d617765736f6d652d6173736574732f6d61737465722f6d656469612f7261696e626f772f6c696e75782e737667" alt= "" width="18" height="18"> Execution

```bash

# Pull l'image Docker
$ docker pull ghcr.io/kyatons/srv-network-games:latest

# Lancer le docker
$ docker run -p 4444:4444 -it ghcr.io/kyatons/srv-network-games:latest
```
