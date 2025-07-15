# Nom du compilateur
CC = gcc
CFLAGS = -O2 -MD -MP

# Options d'inclusion et de liaison
INCLUDE = `sdl2-config --cflags` -I/usr/local/include
LIBS   = `sdl2-config --libs` -lSDL2_image -ltmx -lz `xml2-config --libs` -lm

# Fichiers sources
SRC = main.c \
      framework/map.c \
      game/game.c \
      game/entity.c \
      game/player.c \
      game/npc.c

# Fichiers objets & dépendances
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

# Nom de l'exécutable
EXEC = PokemonV2

# Cible par défaut
all: $(EXEC)

# Inclure automatiquement les fichiers de dépendances
-include $(DEP)

# Édition des liens
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

# Compilation des .c en .o avec génération des dépendances
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJ) $(DEP) $(EXEC)

# Exécution
run: $(EXEC)
	./$(EXEC)
