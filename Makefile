CPP = clang++
C = clang

PROJECT_FILES = waves

FILES = shader \
				computeShader \
				framebuffer \
				shaderStorageBuffer \
				texture

LIBFILES = glad

FLAGS = -std=c++20 \
				-lopencv_videoio \
				-lopencv_core \
				-lglfw \
				-L./lib/\
				-lglad \
				-lGL \
				-lX11 \
				-lpthread \
				-lXrandr \
				-lXi \
				-ldl \
				-Wunused-command-line-argument \
				-march=native \
				-O3

SRC_DIR = src
BUILD_DIR = build
OPENGL_DIR = opengl
OPENGL_LIB_DIR = $(OPENGL_DIR)/lib
OPENGL_BUILD_DIR = $(OPENGL_DIR)/build
OPENGL_IMGUI_DIR = $(OPENGL_DIR)/imgui-src
OPENGL_IMGUI_BUILD = $(OPENGL_DIR)/imgui-build
OPENGL_OBJECTS_DIR = $(OPENGL_DIR)/opengl-objects

OPENGL_IMGUI = $(shell find $(OPENGL_IMGUI_DIR) -name '*.cpp' | xargs -I {} basename {} | sed 's/.cpp//')
OPENGL_LIB_O = $(addprefix $(OPENGL_LIB_DIR)/, $(addsuffix .o, $(LIBFILES)))
OPENGL_BUILD_O = $(addprefix $(OPENGL_BUILD_DIR)/, $(addsuffix .o, $(FILES)))
PROJECT_O = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(PROJECT_FILES)))
PROJECT_CPP = $(addprefix $(SRC_DIR)/, $(addsuffix .cpp, $(PROJECT_FILES)))
OPENGL_IMGUI_FILES = $(addprefix $(OPENGL_IMGUI_DIR)/, $(addsuffix .cpp, $(OPENGL_IMGUI)))
OPENGL_IMGUI_O = $(addprefix $(OPENGL_IMGUI_BUILD)/, $(addsuffix .o, $(OPENGL_IMGUI)))

all: lib imgui opengl waves

$(OPENGL_BUILD_DIR)/%.o: %.cpp
	$(CPP) -g -c $^ -std=c++20 -o $@

$(PROJECT_O): $(PROJECT_CPP)
	mkdir -p ./build
	$(CPP) -g -c $^ -std=c++20 -o $@

waves: $(PROJECT_O) $(BUILD_O)
	$(CPP) $^ $(OPENGL_IMGUI_O) $(OPENGL_BUILD_O)  $(FLAGS) -o $(BUILD_DIR)/$@

opengl: $(OPENGL_BUILD_O)

imgui: $(OPENGL_IMGUI_O)

$(OPENGL_IMGUI_O): $(OPENGL_IMGUI_BUILD)/%.o: $(OPENGL_IMGUI_DIR)/%.cpp
	mkdir -p ./$(OPENGL_IMGUI_BUILD)
	$(CPP) -g -c $< -std=c++20 -o $@

lib: $(OPENGL_LIB_O)
	$(C) $^ -shared -o /usr/local/lib/lib$(LIBFILES).so

$(OPENGL_LIB_O): $(OPENGL_DIR)/$(LIBFILES).c
	mkdir -p ./$(OPENGL_LIB_DIR)
	$(C) -c -fPIC $^ -o $(OPENGL_LIB_DIR)/$(LIBFILES).o

$(OPENGL_BUILD_O): $(OPENGL_BUILD_DIR)/%.o: $(OPENGL_OBJECTS_DIR)/%.cpp
	mkdir -p ./$(OPENGL_BUILD_DIR)
	$(CPP) -g -c $< -std=c++20 -o $@

clean:
	rm -rf ./$(OPENGL_BUILD_DIR)
	rm -rf ./$(OPENGL_LIB_DIR)
	rm -rf ./$(OPENGL_IMGUI_BUILD)
	rm -rf ./$(BUILD_DIR)
	rm -f /usr/local/lib/libglad.so
