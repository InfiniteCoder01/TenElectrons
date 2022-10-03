CXX := em++

Mova := D:/Dev/C++/LIB/Mova/src
Mova_OBJ := $(or $(word 1,$(wildcard build/Mova/*.o)), mova)

GLM := D:/Dev/C++/LIB/glm

Include := -I $(Mova) -I $(GLM)
Flags :=
Linker := -s ASYNCIFY -s 'ASYNCIFY_IMPORTS=["emscripten_asm_const_int", "emscripten_asm_const_ptr"]' --use-preload-plugins --bind
Settings := -o html/index.html --shell-file html/basic_template.html --preload-file Assets

html/index.html: $(ImGUI_OBJ) $(Mova_OBJ) src/*
	@echo Building Application...
	@$(CXX) $(wildcard src/*.cpp build/*/*.o) $(Flags) $(Linker) $(Include) $(Settings)

$(Mova_OBJ): $(Mova)/*.cpp $(Mova)/*/*.cpp $(Mova)/*.h
	@echo Building Mova...
	@for %%f in ($(wildcard $(Mova)/*.cpp $(Mova)/*/*.cpp)) do @$(CXX) -c %%f $(Flags) $(Include) -o build/Mova/%%~nxf.o
