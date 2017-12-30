//
// rdpdemo/src/main.c: RDP demo (entry point).
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <os/fbtext.h>
#include <libgfx/init.h>
#include <libgfx/rdp.h>
#include <libgfx/rspbuf.h>
#include <rcp/vi.h>
#include <stdint.h>

// These pre-defined values are suitable for NTSC.
// TODO: Add support for PAL and PAL-M televisions.
static vi_state_t vi_state = {
  0x0000324E, // status
  0x00200000, // origin
  0x00000140, // width
  0x00000002, // intr
  0x00000000, // current
  0x03E52239, // burst
  0x0000020D, // v_sync
  0x00000C15, // h_sync
  0x0C150C15, // leap
  0x006C02EC, // h_start
  0x002501FF, // v_start
  0x000E0204, // v_burst
  0x00000200, // x_scale
  0x00000400, // y_scale
};

static uint32_t fb_origin;

// krom's 3d library.
static unsigned next_free_vert;
#include "3d.c"
#include "3dscene.c"

#define SAMPLE_TYPE 0x00200000000000ULL // Set_Other_Modes b: Determines How Textures Are Sampled: 0=1x1 (Point Sample), 1=2x2. Note That Copy (Point Sample 4 Horizontally Adjacent Texels) Mode Is Indicated By CYCLE_TYPE (Bit 45)
#define BI_LERP_0 0x00080000000000ULL // Set_Other_Modes Z: 1=BI_LERP, 0=Color Convert Operation In Texture Filter. Used In Cycle 0 (Bit 43)
#define RGB_DITHER_SEL_NO_DITHER 0x0000C000000000ULL             // Set_Other_Modes V2: RGB Dither Selection No Dither (Bit 38..39)
#define ALPHA_DITHER_SEL_NO_DITHER 0x00003000000000ULL // Set_Other_Modes V1: Alpha Dither Selection No Dither (Bit 36..37)
#define B_M1A_0_2 0x00000080000000ULL // Set_Other_Modes V: Blend Modeword, Multiply 1a Input Select 2, Cycle 0 (Bit 30..31)

// Set Combine Mode
static inline void rdp_set_combine_mode( struct libgfx_rspbuf *rspbuf, uint8_t sub_a_r0, uint8_t mul_r0, uint8_t sub_a_a0, uint8_t mul_a0, uint8_t sub_a_r1, uint8_t mul_r1, uint8_t sub_b_r0, uint8_t sub_b_r1, uint8_t sub_a_a1, uint8_t mul_a1, uint8_t add_r0, uint8_t sub_b_a0, uint8_t add_a0, uint8_t add_r1, uint8_t sub_b_a1, uint8_t add_a1 )
{
    libgfx_rspbuf_append( rspbuf, 0x3C000000 | sub_a_r0 << 20 | mul_r0 << 15 | sub_a_a0 << 12 | mul_a0 << 9 | sub_a_r1 << 5 | mul_r1 );
    libgfx_rspbuf_append( rspbuf, sub_b_r0 << 28 | sub_b_r1 << 24 | sub_a_a1 << 21 | mul_a1 << 18 | add_r0 << 15 | sub_b_a0 << 12 | add_a0 << 9 | add_r1 << 6 | sub_b_a1 << 3 | add_a1);
}

void main(void *unused __attribute__((unused))) {
  struct libgfx_rspbuf *rspbuf = libgfx_rspbuf_create();
  struct libn64_fbtext_context context;
  
  // Setup the OS's private/hidden text rendering engine.
  // The 0 and ~0 are just fill colors (black and white).
  libn64_fbtext_init(&context, 0x80000000 | vi_state.origin,
      ~0, 0, 320, LIBN64_FBTEXT_16BPP);
	  
  libgfx_init();

  // Register DP/VI interrupts on this thread. When registering a thread
  // w/ interrupts, it causes the threads message queue to get populated
  // w/ a message each time an interrupt fires.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_DP);

  // Initialize the RDP by building and issuing a display list.
  rdp_set_color_image(rspbuf, FORMAT_RGBA, COLOR_ELEMENT_16B,
      vi_state.width - 1, vi_state.origin);

  rdp_set_scissor(rspbuf, 0, 0, 320, 240,
      SCISSOR_NO_INTERLACED, SCISSOR_DONTCARE);

  rdp_set_other_modes(rspbuf, CYCLE_TYPE_FILL);
  rdp_set_fill_color(rspbuf, rdp_rgba16(24, 128, 212, 255));
  rdp_fill_rectangle(rspbuf, 0, 0, 319, 239);
  rdp_sync_pipe(rspbuf);
  rdp_sync_full(rspbuf);

  rsp_finalize(rspbuf);
  libgfx_rspbuf_flush(rspbuf);
  libgfx_run();
  libn64_recvt_message();
  vi_flush_state(&vi_state);

  //libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_VI);
  //libn64_thread_unreg_intr(libn64_thread_self(), LIBN64_INTERRUPT_DP);

  // For each frame...
  uint16_t fps_tick=0;
  uint16_t fps_sec=0;
  uint32_t last_sec=0;

  uint16_t XRot = 0; // X Rotation Value (0..1023)
  uint16_t YRot = 0; // Y Rotation Value (0..1023)
  uint16_t ZRot = 0; // Z Rotation Value (0..1023)

  while (1) {
    rdp_set_other_modes(rspbuf, CYCLE_TYPE_FILL);
    rdp_set_fill_color(rspbuf, rdp_rgba16(24, 128, 212, 255));
    rdp_fill_rectangle(rspbuf, 0, 0, 319, 239);
    rdp_sync_pipe(rspbuf);

    rdp_set_other_modes(rspbuf, SAMPLE_TYPE|BI_LERP_0|RGB_DITHER_SEL_NO_DITHER|ALPHA_DITHER_SEL_NO_DITHER|B_M1A_0_2); // Set Other Modes
    rdp_set_combine_mode(rspbuf, 0x0,0x00, 0,0, 0x6,0x01, 0x0,0xF, 1,0, 0,0,0, 7,7,7); // Set Combine Mode: SubA RGB0,MulRGB0, SubA Alpha0,MulAlpha0, SubA RGB1,MulRGB1, SubB RGB0,SubB RGB1, SubA Alpha1,MulAlpha1, AddRGB0,SubB Alpha0,AddAlpha0, AddRGB1,SubB Alpha1,AddAlpha1
    matrix_identity(Matrix3D); // Reset Matrix To Identity
    translate_xyz(Matrix3D, CubeRedPos[0], CubeRedPos[1], CubeRedPos[2]); // Translate: Matrix, X, Y, Z
    rotate_x(Matrix3D, Sin1024, XRot); // Rotate: Matrix, Precalc Table, X
    fill_triangle_array(rspbuf, CubeTri, CubeRedCol, CULL_BACK, 0, 108); // Fill Triangle Array: Vert Array, Color Array, RDP Buffer, Culling, Base, Length

    matrix_identity(Matrix3D); // Reset Matrix To Identity
    translate_xyz(Matrix3D, CubeGreenPos[0], CubeGreenPos[1], CubeGreenPos[2]); // Translate: Matrix, X, Y, Z
    rotate_y(Matrix3D, Sin1024, YRot); // Rotate: Matrix, Precalc Table, Y
    fill_triangle_array(rspbuf, CubeTri, CubeGreenCol, CULL_BACK, 0, 108); // Fill Triangle Array: Vert Array, Color Array, RDP Buffer, Culling, Base, Length

    matrix_identity(Matrix3D); // Reset Matrix To Identity
    translate_xyz(Matrix3D, CubeBluePos[0], CubeBluePos[1], CubeBluePos[2]); // Translate: Matrix, X, Y, Z
    rotate_z(Matrix3D, Sin1024, ZRot); // Rotate: Matrix, Precalc Table, Z
    fill_triangle_array(rspbuf, CubeTri, CubeBlueCol, CULL_BACK, 0, 108); // Fill Triangle Array: Vert Array, Color Array, RDP Buffer, Culling, Base, Length

    rdp_sync_full(rspbuf);
    rsp_finalize(rspbuf);
    libgfx_rspbuf_flush(rspbuf);
    libgfx_rspbuf_flush_vertices(rspbuf);
    libgfx_run();

    // We blow out the vertex cache if we do this without flushing...
    next_free_vert = 0;

    matrix_identity(Matrix3D); // Reset Matrix To Identity
    translate_xyz(Matrix3D, CubeYellowPos[0], CubeYellowPos[1], CubeYellowPos[2]); // Translate: Matrix, X, Y, Z
    rotate_xy(Matrix3D, Sin1024, XRot, YRot); // Rotate: Matrix, Precalc Table, X, Y
    fill_triangle_array(rspbuf, CubeTri, CubeYellowCol, CULL_BACK, 0, 108); // Fill Triangle Array: Vert Array, Color Array, RDP Buffer, Culling, Base, Length

    matrix_identity(Matrix3D); // Reset Matrix To Identity
    translate_xyz(Matrix3D, CubePurplePos[0], CubePurplePos[1], CubePurplePos[2]); // Translate: Matrix, X, Y, Z
    rotate_xz(Matrix3D, Sin1024, XRot, ZRot); // Rotate: Matrix, Precalc Table, X, Z
    fill_triangle_array(rspbuf, CubeTri, CubePurpleCol, CULL_BACK, 0, 108); // Fill Triangle Array: Vert Array, Color Array, RDP Buffer, Culling, Base, Length

    matrix_identity(Matrix3D); // Reset Matrix To Identity
    translate_xyz(Matrix3D, CubeCyanPos[0], CubeCyanPos[1], CubeCyanPos[2]); // Translate: Matrix, X, Y, Z
    rotate_xyz(Matrix3D, Sin1024, XRot, YRot, ZRot); // Rotate: Matrix, Precalc Table, X, Y, Z
    fill_triangle_array(rspbuf, CubeTri, CubeCyanCol, CULL_BACK, 0, 108); // Fill Triangle Array: Vert Array, Color Array, RDP Buffer, Culling, Base, Length

    rdp_sync_full(rspbuf);
    rsp_finalize(rspbuf);

    // Wait for the last RDP DL to finish before issuing anew.
    libn64_recvt_message();
    libgfx_rspbuf_flush(rspbuf);
    libgfx_rspbuf_flush_vertices(rspbuf);
    libgfx_run();

#if 0
    // Point to VI to the last fb, swap the front and back fbs.
    vi_flush_state(&vi_state);
    vi_state.origin ^= 0x100000; // 1MB

    // Wipe the back-buffer to black.
    for (i = 0; i < 320 * 240 * 2; i += 16) {
      __asm__ __volatile__(
        ".set gp=64\n\t"
        "cache 0xD, 0x0(%0)\n\t"
        "sd $zero, 0x0(%0)\n\t"
        "sd $zero, 0x8(%0)\n\t"
        "cache 0x19, 0x0(%0)\n\t"
        ".set gp=default\n\t"

        :: "r" (0x80000000 | (vi_state.origin + i))
        : "memory"
      );
    }

#endif
    // All vertices were flushed and can be reused.
    next_free_vert = 0;

    // Update triangle rotation variables
    XRot = (XRot + 1) & 1023;
    YRot = (YRot + 1) & 1023;
    ZRot = (ZRot + 1) & 1023;

    // Set the position of the cursor. If your string
    // contains \n, it will automatically advance it.
    // The text also wraps around, but it won't scroll.
    context.x = 1;
    context.y = 1;

	  struct timeval timer = libn64_time();
	
    if(timer.tv_sec!=last_sec)
    {
        fps_sec=fps_tick;
        fps_tick=0;
        last_sec=timer.tv_sec;
    }	
    fps_tick++;

    libn64_fbtext_putu32(&context, fps_sec);	
	
    // Block until the next VI interrupt comes in.
    libn64_recvt_message();
  }
}

