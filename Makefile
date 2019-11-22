OPENCV ?= 0
OPENMP ?= 0
DEBUG  ?= 0

OBJ= main.o panorama.o phash.o matrix.o image.o utils.o draw.o filter.o hough.o canny.o blob.o harris.o flow.o # insert objectfiles here
EXECOBJA= panorama_images.o rotate.o compare_images.o resize.o grayscale.o binarize.o apply_filter.o find_lines.o find_blobs.o find_corners.o webcam.o flow_cam.o # add executables here

VPATH=./src/:./examples
EXEC=boomercv
OBJDIR=./obj/

CC=gcc
CPP=g++
OPTS=-Ofast
LDFLAGS= -lm -pthread
COMMON= -Iinclude/ -Isrc/
CFLAGS=-Wall -Wno-unknown-pragmas -Wfatal-errors -fPIC

ifeq ($(OPENMP), 1)
CFLAGS+= -fopenmp
endif

ifeq ($(DEBUG), 1)
OPTS=-O0 -g
endif

CFLAGS+=$(OPTS)

ifeq ($(OPENCV), 1)
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV
LDFLAGS+= `pkg-config --libs opencv` -lstdc++
COMMON+= `pkg-config --cflags opencv`
OBJ+= image_opencv.o
endif

EXECOBJS = $(addprefix $(OBJDIR), $(EXECOBJA))
OBJS   = $(addprefix $(OBJDIR), $(OBJ))
DEPS   = $(wildcard include/*.h) Makefile

all: obj $(EXEC)

$(EXEC): $(OBJS) $(EXECOBJS)
	$(CC) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.c $(DEPS)
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

ifeq ($(OPENCV), 1)
$(OBJDIR)%.o: %.cpp $(DEPS)
	$(CPP) $(COMMON) $(CFLAGS) -c $< -o $@
endif

obj:
	mkdir -p obj

.PHONY: clean
clean:
	rm -rf $(OBJS) $(ALIB) $(EXEC) $(EXECOBJS) $(OBJDIR)/* $(OBJDIR)
