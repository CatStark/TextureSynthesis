# TextureSynthesis
Texture Synthesis for  Procedurally Generating  Realistic Terrain Surfaces of Asteroids

Compile wth 

```

g++ `pkg-config --cflags opencv` -o textureSynthesis -g -lgomp -fopenmp main.cpp FinalImage.cpp Patch.cpp Grid.cpp graph.cpp maxflow.cpp `pkg-config --libs opencv`

