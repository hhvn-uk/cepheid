#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <raylib.h>
#include "main.h"

#define LOADING_FPS 60
#define LOADING_W 500
#define LOADING_H 50
#define LOADING_FONT 30
#define LOADING_PAD ((LOADING_H - LOADING_FONT) / 2)

struct shmdata {
	int step;
	char str[LOAD_STR_MAX];
};

static int recv_winch = 0;

static void
loading_sighandler(int signal) {
	fflush(stdout);
	if (signal == SIGWINCH)
		recv_winch = 1;
}

Loader *
loading_open(int steps, char *initstr) {
	Loader *ret;
	int fd;
	struct shmdata *shmdata;
	/* child only */
	struct shmdata *data;
	char curstr[LOAD_STR_MAX];
	struct sigaction sa;

	ret = malloc(sizeof(Loader));
	if (!ret) return NULL;

	snprintf(ret->path, sizeof(ret->path), "/%ld-loading", (long)getpid());
	if ((fd = shm_open(ret->path, O_CREAT|O_RDWR, 0600)) == -1)
		goto fail;

	if (ftruncate(fd, sizeof(struct shmdata)) == -1)
		goto fail;

	if ((shmdata = mmap(NULL, sizeof(struct shmdata), PROT_READ|PROT_WRITE,
					MAP_SHARED, fd, 0)) == MAP_FAILED)
		goto fail;

	shmdata->step = 0;
	ret->step = &shmdata->step;
	ret->data = shmdata->str;

	if ((ret->pid = fork()) == -1)
		goto unmap;

	if (ret->pid == 0) {
		/* we're the child */
		sa.sa_handler = loading_sighandler;
		sa.sa_flags = SA_RESTART;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGWINCH, &sa, NULL);

		fd = shm_open(ret->path, O_RDONLY, 0);
		data = mmap(NULL, sizeof(struct shmdata), PROT_READ,
				MAP_SHARED, fd, 0);
		if (fd == -1 || data == MAP_FAILED)
			exit(1);

		SetWindowState(FLAG_WINDOW_TOPMOST);
		SetTargetFPS(LOADING_FPS);
		SetExitKey(KEY_NULL);
		InitWindow(LOADING_W, LOADING_H, "Loading...");
		snprintf(curstr, sizeof(curstr), "%s...", initstr);

		while (!WindowShouldClose()) {
			if (recv_winch) {
				recv_winch = 0;
				memcpy(curstr, data->str, LOAD_STR_MAX);
			}

			BeginDrawing();

			ClearBackground(col_bg);
			DrawText(curstr, LOADING_PAD, LOADING_PAD, LOADING_FONT, col_fg);

			BeginScissorMode(0, 0, data->step * LOADING_W / steps, LOADING_H);
			DrawRectangle(0, 0, data->step * LOADING_W / steps, LOADING_H, col_fg);
			DrawText(curstr, LOADING_PAD, LOADING_PAD, LOADING_FONT, col_bg);
			EndScissorMode();

			EndDrawing();
		}
		exit(0);
	}

	return ret;

unmap:
	munmap(ret->data, sizeof(struct shmdata));
fail:
	free(ret);
	return NULL;
}

void
loading_update(Loader *hand, char *str) {
	(*hand->step)++;
	snprintf(hand->data, LOAD_STR_MAX, "%s...", str);
	kill(hand->pid, SIGWINCH);
}

void
loading(Loader **hand, int steps, char *str) {
	if (!*hand)
		*hand = loading_open(steps, str);
	else
		loading_update(*hand, str);
}

void
loading_close(Loader *hand) {
	kill(hand->pid, SIGTERM);
	shm_unlink(hand->path);
	munmap(hand->data, sizeof(struct shmdata));
	free(hand);
}
