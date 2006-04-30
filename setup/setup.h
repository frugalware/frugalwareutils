typedef struct
{
	char *name;
	char *desc;
	int (*run)(int argc, char **argv);
	void *handle;
} plugin_t;
