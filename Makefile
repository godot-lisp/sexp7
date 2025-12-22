.PHONY: test

test: bin/s7 demo/.godot
	$(info 🧪 Running tests)
	bin/s7 test/test-main.sx7

bin/s7: s7/s7.c
	$(info 🛠️ Building scheme interpreter)
	gcc s7/s7.c -o bin/s7 -DWITH_MAIN -DWITH_SYSTEM_EXTRAS -DWITH_C_LOADER=0 -I. -O2 -g -ldl -lm

SRC_FILES := $(shell find src -type f ! -name "*.os")
DEMO_FILES := $(shell find demo -type f -name "*.tscn" -or -name "*.sx7")

demo/.godot: $(SRC_FILES) $(DEMO_FILES)
	$(info 🛠️ Building extension)
	@scons
	$(info 📦 Importing test scene)
	godot --path demo --headless --import

.PHONY: run

run: demo/.godot
	godot -e --path demo main.tscn

.PHONY: demo

demo: demo/.godot
	godot --path demo

.PHONY: android

android:
	scons platform=android target=template_debug

.PHONY: pack

pack:
	rsync -rup demo/addons bin/pack
	rsync -rup bin/android bin/ios bin/linux bin/macos bin/web bin/windows demo/bin/sexp7.gdextension bin/pack/bin

.PHONY: test-watch

test-watch:
	find demo/addons/s7 test | entr make test
