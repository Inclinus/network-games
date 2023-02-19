#!/bin/bash

# Code Couleurs
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

if [ -x "$(command -v apt-get)" ]; then
  # Installer les paquets avec APT
  sudo apt-get update
  sudo apt-get install gcc libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev -y
elif [ -x "$(command -v yum)" ]; then
  # Installer les paquets avec YUM
  sudo yum update
  sudo yum install gcc libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev -y
elif [ -x "$(command -v pacman)" ]; then
  # Installer les paquets avec Pacman
  sudo pacman -Sy
  sudo pacman -S gcc libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev --noconfirm
elif [ -x "$(command -v zypper)" ]; then
  # Installer les paquets avec Zypper
  sudo zypper refresh
  sudo zypper install gcc libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev -y
else
  echo -e "${RED}[FAIL]${NC} Aucun gestionnaire de paquets compatible n'a été trouvé sur ce système."
fi
echo -e "${GREEN}[OK]${NC} Les dépendances sont bien installé ou présente sur ce système !"

echo "Compilation du Client ..."
gcc events/EventManager.c client/client.c client/tictactoe/tictactoe.c client/connect4/connect4.c sdl-utils/SDLUtils.c $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -lm -pthread  -o target/client.out

echo "Lancement ..."
./target/client.out
