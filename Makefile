NAME       := bone-run
CC_PC      := gcc

# OpenOrbis toolchain. Путь задаётся переменной окружения OO_PS4_TOOLCHAIN.
PS4_BIN    := $(OO_PS4_TOOLCHAIN)/bin/linux
PS4_CC     := $(PS4_BIN)/orbis-clang
PS4_LD     := $(PS4_BIN)/orbis-ld
FSELF      := $(PS4_BIN)/create-fself

.PHONY: test ps4 package clean

# Тесты игровой логики на ПК (работают без PS4 SDK)
test:
	mkdir -p build
	$(CC_PC) -std=c11 -Wall -Wextra -Werror -Isrc src/game.c tests/test_game.c -o build/test_game
	./build/test_game

# Сборка eboot.bin для PS4.
# ВНИМАНИЕ: флаги линковки и список библиотек скопируй из официальных
# примеров OpenOrbis для своей версии toolchain. Здесь шаблон.
ps4:
	@test -n "$(OO_PS4_TOOLCHAIN)" || (echo "Переменная OO_PS4_TOOLCHAIN не задана" && exit 1)
	mkdir -p build/ps4
	$(PS4_CC) -c -O2 -Wall -fpie -Isrc src/game.c -o build/ps4/game.o
	$(PS4_CC) -c -O2 -Wall -fpie -Isrc src/main_ps4.c -o build/ps4/main_ps4.o
	$(PS4_LD) -o build/ps4/$(NAME).elf build/ps4/game.o build/ps4/main_ps4.o
	$(FSELF) build/ps4/$(NAME).elf build/ps4/eboot.bin

# Папка для пакетирования и zip
package: ps4
	rm -rf dist && mkdir -p dist/$(NAME)/sce_sys
	cp build/ps4/eboot.bin dist/$(NAME)/
	cp -r pkg/sce_sys/. dist/$(NAME)/sce_sys/
	cp -r assets dist/$(NAME)/
	cd dist && zip -r $(NAME)-ps4.zip $(NAME)

clean:
	rm -rf build dist
