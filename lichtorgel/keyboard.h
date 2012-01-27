typedef struct {
	unsigned char type, x, y;
} KeyboardEvent;

int		keyboard_init();
void	keyboard_kill();
int		keyboard_poll(KeyboardEvent* e);

