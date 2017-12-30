// Object Triangle Vertices: X, Y, Z (Clockwise Winding)
static float CubeTri[108] = {
  // Cube Front Face
  -10.0,  10.0, -10.0, // Triangle 1 Top Left
   10.0,  10.0, -10.0, // Triangle 1 Top Right
  -10.0, -10.0, -10.0, // Triangle 1 Bottom Left
   10.0,  10.0, -10.0, // Triangle 2 Top Right
   10.0, -10.0, -10.0, // Triangle 2 Bottom Right
  -10.0, -10.0, -10.0, // Triangle 2 Bottom Left

  // Cube Back Face
   10.0,  10.0,  10.0, // Triangle 3 Top Right
  -10.0,  10.0,  10.0, // Triangle 3 Top Left
   10.0, -10.0,  10.0, // Triangle 3 Bottom Right
  -10.0,  10.0,  10.0, // Triangle 4 Top Left
  -10.0, -10.0,  10.0, // Triangle 4 Bottom Left
   10.0, -10.0,  10.0, // Triangle 4 Bottom Right

  // Cube Left Face
  -10.0,  10.0,  10.0, // Triangle 5 Top Left
  -10.0,  10.0, -10.0, // Triangle 5 Top Right
  -10.0, -10.0,  10.0, // Triangle 5 Bottom Left
  -10.0,  10.0, -10.0, // Triangle 6 Top Right
  -10.0, -10.0, -10.0, // Triangle 6 Bottom Right
  -10.0, -10.0,  10.0, // Triangle 6 Bottom Left

  // Cube Right Face
   10.0,  10.0, -10.0, // Triangle 7 Top Left
   10.0,  10.0,  10.0, // Triangle 7 Top Right
   10.0, -10.0, -10.0, // Triangle 7 Bottom Left
   10.0,  10.0,  10.0, // Triangle 8 Top Right
   10.0, -10.0,  10.0, // Triangle 8 Bottom Right
   10.0, -10.0, -10.0, // Triangle 8 Bottom Left

  // Cube Top Face
   10.0,  10.0, -10.0, // Triangle 9 Top Right
  -10.0,  10.0, -10.0, // Triangle 9 Top Left
  -10.0,  10.0,  10.0, // Triangle 9 Bottom Left
   10.0,  10.0, -10.0, // Triangle 10 Top Right
  -10.0,  10.0,  10.0, // Triangle 10 Bottom Left
   10.0,  10.0,  10.0, // Triangle 10 Bottom Right

  // Bottom Face
  -10.0, -10.0, -10.0, // Triangle 11 Top Left
   10.0, -10.0, -10.0, // Triangle 11 Top Right
   10.0, -10.0,  10.0, // Triangle 11 Bottom Right
  -10.0, -10.0, -10.0, // Triangle 12 Top Left
   10.0, -10.0,  10.0, // Triangle 12 Bottom Right
  -10.0, -10.0,  10.0, // Triangle 12 Bottom Left
};

// Object Triangle Colors: R, G, B, A
static uint8_t CubeRedCol[48] = {
  // Cube Front Face
  255,0,0,255, // Triangle 1 Color
  255,0,0,255, // Triangle 2 Color

  // Cube Back Face
  120,0,0,255, // Triangle 3 Color
  120,0,0,255, // Triangle 4 Color

  // Cube Left Face
  140,0,0,255, // Triangle 5 Color
  140,0,0,255, // Triangle 6 Color

  // Cube Right Face
  160,0,0,255, // Triangle 7 Color
  160,0,0,255, // Triangle 8 Color

  // Cube Top Face
  180,0,0,255, // Triangle 9 Color
  180,0,0,255, // Triangle 10 Color

  // Cube Bottom Face
  100,0,0,255, // Triangle 11 Color
  100,0,0,255, // Triangle 12 Color
};

// Object Triangle Colors: R, G, B, A
static uint8_t CubeGreenCol[48] = {
  // Cube Front Face
  0,255,0,255, // Triangle 1 Color
  0,255,0,255, // Triangle 2 Color

  // Cube Back Face
  0,120,0,255, // Triangle 3 Color
  0,120,0,255, // Triangle 4 Color

  // Cube Left Face
  0,140,0,255, // Triangle 5 Color
  0,140,0,255, // Triangle 6 Color

  // Cube Right Face
  0,160,0,255, // Triangle 7 Color
  0,160,0,255, // Triangle 8 Color

  // Cube Top Face
  0,180,0,255, // Triangle 9 Color
  0,180,0,255, // Triangle 10 Color

  // Cube Bottom Face
  0,100,0,255, // Triangle 11 Color
  0,100,0,255, // Triangle 12 Color
};

// Object Triangle Colors: R, G, B, A
static uint8_t CubeBlueCol[48] = {
  // Cube Front Face
  0,0,255,255, // Triangle 1 Color
  0,0,255,255, // Triangle 2 Color

  // Cube Back Face
  0,0,120,255, // Triangle 3 Color
  0,0,120,255, // Triangle 4 Color

  // Cube Left Face
  0,0,140,255, // Triangle 5 Color
  0,0,140,255, // Triangle 6 Color

  // Cube Right Face
  0,0,160,255, // Triangle 7 Color
  0,0,160,255, // Triangle 8 Color

  // Cube Top Face
  0,0,180,255, // Triangle 9 Color
  0,0,180,255, // Triangle 10 Color

  // Cube Bottom Face
  0,0,100,255, // Triangle 11 Color
  0,0,100,255, // Triangle 12 Color
};

// Object Triangle Colors: R, G, B, A
static uint8_t CubeYellowCol[48] = {
  // Cube Front Face
  255,255,0,255, // Triangle 1 Color
  255,255,0,255, // Triangle 2 Color

  // Cube Back Face
  120,120,0,255, // Triangle 3 Color
  120,120,0,255, // Triangle 4 Color

  // Cube Left Face
  140,140,0,255, // Triangle 5 Color
  140,140,0,255, // Triangle 6 Color

  // Cube Right Face
  160,160,0,255, // Triangle 7 Color
  160,160,0,255, // Triangle 8 Color

  // Cube Top Face
  180,180,0,255, // Triangle 9 Color
  180,180,0,255, // Triangle 10 Color

  // Cube Bottom Face
  100,100,0,255, // Triangle 11 Color
  100,100,0,255, // Triangle 12 Color
};

// Object Triangle Colors: R, G, B, A
static uint8_t CubePurpleCol[48] = {
  // Cube Front Face
  255,0,255,255, // Triangle 1 Color
  255,0,255,255, // Triangle 2 Color

  // Cube Back Face
  120,0,120,255, // Triangle 3 Color
  120,0,120,255, // Triangle 4 Color

  // Cube Left Face
  140,0,140,255, // Triangle 5 Color
  140,0,140,255, // Triangle 6 Color

  // Cube Right Face
  160,0,160,255, // Triangle 7 Color
  160,0,160,255, // Triangle 8 Color

  // Cube Top Face
  180,0,180,255, // Triangle 9 Color
  180,0,180,255, // Triangle 10 Color

  // Cube Bottom Face
  100,0,100,255, // Triangle 11 Color
  100,0,100,255, // Triangle 12 Color
};

// Object Triangle Colors: R, G, B, A
static uint8_t CubeCyanCol[48] = {
  // Cube Front Face
  0,255,255,255, // Triangle 1 Color
  0,255,255,255, // Triangle 2 Color

  // Cube Back Face
  0,120,120,255, // Triangle 3 Color
  0,120,120,255, // Triangle 4 Color

  // Cube Left Face
  0,140,140,255, // Triangle 5 Color
  0,140,140,255, // Triangle 6 Color

  // Cube Right Face
  0,160,160,255, // Triangle 7 Color
  0,160,160,255, // Triangle 8 Color

  // Cube Top Face
  0,180,180,255, // Triangle 9 Color
  0,180,180,255, // Triangle 10 Color

  // Cube Bottom Face
  0,100,100,255, // Triangle 11 Color
  0,100,100,255, // Triangle 12 Color
};

// Scene Object Position Data: Translation X, Y, Z
static float CubeRedPos[3] = { -35.0, 20.0, 90.0 }; // Object Position: X, Y, Z
static float CubeGreenPos[3] = { 0.0, 20.0, 90.0 }; // Object Position: X, Y, Z
static float CubeBluePos[3] = { 35.0, 20.0, 90.0 }; // Object Position: X, Y, Z
static float CubeYellowPos[3] = {-35.0, -20.0, 90.0 }; // Object Position: X, Y, Z
static float CubePurplePos[3] = { 0.0, -20.0, 90.0 }; // Object Position: X, Y, Z
static float CubeCyanPos[3] = { 35.0, -20.0, 90.0 }; // Object Position: X, Y, Z