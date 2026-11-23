#include <math.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>

PSP_MODULE_INFO("MudRunner PSP", 0, 0, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define BUF_WIDTH  512
#define SCR_WIDTH  480
#define SCR_HEIGHT 272
#define PI 3.14159265358979323846f

typedef struct {
    unsigned int color;
    float x, y, z;
} Vertex;

static unsigned int __attribute__((aligned(16))) list[262144];
static Vertex __attribute__((aligned(16))) cube[36];

static float truck_x = 0.0f;
static float truck_z = 0.0f;
static float truck_yaw = 0.0f;
static float speed = 0.0f;

static void build_cube(void) {
    /* 12 triangles, unit cube centered at origin. */
    const float p[8][3] = {
        {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
        {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}
    };
    const int idx[36] = {
        0,1,2, 0,2,3, 4,6,5, 4,7,6,
        0,4,5, 0,5,1, 3,2,6, 3,6,7,
        1,5,6, 1,6,2, 0,3,7, 0,7,4
    };
    int i;
    for (i = 0; i < 36; ++i) {
        cube[i].x = p[idx[i]][0];
        cube[i].y = p[idx[i]][1];
        cube[i].z = p[idx[i]][2];
        cube[i].color = 0xFF586B45;
    }
}

static void setup_gu(void) {
    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);
    sceGuDepthBuffer((void*)0x110000, BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH / 2), 2048 - (SCR_HEIGHT / 2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_CLIP_PLANES);
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

static void draw_box(float x, float y, float z,
                     float sx, float sy, float sz,
                     unsigned int color) {
    int i;
    for (i = 0; i < 36; ++i) cube[i].color = color;
    sceGumPushMatrix();
    {
        ScePspFVector3 pos = {x, y, z};
        ScePspFVector3 scale = {sx, sy, sz};
        sceGumTranslate(&pos);
        sceGumScale(&scale);
        sceGumDrawArray(GU_TRIANGLES,
            GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
            36, 0, cube);
    }
    sceGumPopMatrix();
}

static void draw_world(void) {
    int x, z;

    /* Low-poly test ground: a tiled green-brown plane. */
    for (x = -8; x < 8; ++x) {
        for (z = -8; z < 8; ++z) {
            unsigned int c = ((x + z) & 1) ? 0xFF514B36 : 0xFF5B543D;
            draw_box((float)x * 3.0f, -1.15f, (float)z * 3.0f,
                     1.5f, 0.12f, 1.5f, c);
        }
    }

    /* Truck chassis and cabin. */
    draw_box(truck_x, -0.05f, truck_z, 0.85f, 0.22f, 1.65f, 0xFF3A783D);
    draw_box(truck_x, 0.48f, truck_z - 0.45f, 0.72f, 0.48f, 0.72f, 0xFF477EAE);

    /* Six simple dark wheel blocks, three axles. */
    for (x = -1; x <= 1; ++x) {
        float wz = truck_z + (float)x * 1.0f;
        draw_box(truck_x - 0.92f, -0.58f, wz, 0.18f, 0.38f, 0.34f, 0xFF202020);
        draw_box(truck_x + 0.92f, -0.58f, wz, 0.18f, 0.38f, 0.34f, 0xFF202020);
    }
}

int main(void) {
    SceCtrlData pad;
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    pspDebugScreenInit();
    build_cube();
    setup_gu();

    for (;;) {
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_START) break;

        float steer = ((float)pad.Lx - 128.0f) / 128.0f;
        float throttle = (128.0f - (float)pad.Ly) / 128.0f;

        speed += throttle * 0.012f;
        speed *= 0.985f;
        if (speed > 0.18f) speed = 0.18f;
        if (speed < -0.10f) speed = -0.10f;
        truck_yaw += steer * speed * 0.035f;
        truck_x += sinf(truck_yaw) * speed;
        truck_z += cosf(truck_yaw) * speed;

        sceGuStart(GU_DIRECT, list);
        sceGuClearColor(0xFF87A9C4);
        sceGuClearDepth(0);
        sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective(55.0f, 16.0f/9.0f, 0.5f, 150.0f);

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();
        {
            ScePspFVector3 cam = {
                truck_x - sinf(truck_yaw) * 5.0f,
                3.2f,
                truck_z - cosf(truck_yaw) * 5.0f
            };
            ScePspFVector3 target = {truck_x, 0.0f, truck_z};
            ScePspFVector3 up = {0.0f, 1.0f, 0.0f};
            sceGumLookAt(&cam, &target, &up);
        }

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        draw_world();

        sceGuFinish();
        sceGuSync(0, 0);
        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    }

    sceGuTerm();
    sceKernelExitGame();
    return 0;
}
