#define DRIVER_AUTHOR "Magestik"
#define DRIVER_DESC "Stereoscopic Glasses Driver"
#define DRIVER_NAME "glasses3d"

typedef struct {
	int eye;
	int inversed;
} state3d_t;

typedef struct {
	state3d_t state;
	void (*swap_eyes)(state3d_t);
} dev3d_t;

/* GLOBALS */
extern int current_eye;
extern int refresh_rate;

void glasses3d_swap(void);






