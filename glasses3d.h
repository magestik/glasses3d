#define DRIVER_AUTHOR "Magestik"
#define DRIVER_DESC "3D Glasses driver"
#define DRIVER_NAME "glasses3d"

#define MAX_DEVICES 16

/* GLOBALS */
extern int current_eye;
extern int inversed;

typedef struct {
	int eye;
	int inversed;
} state3d_t;

typedef struct {
	state3d_t state;
	void (*swap_eyes)(state3d_t);
} dev3d_t;

extern dev3d_t devices_list[MAX_DEVICES];
extern int devices_count;






