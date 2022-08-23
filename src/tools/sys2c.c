#include <stdio.h>

int size;
unsigned char* buf;

int
main(int argc,char** argv) {
	FILE* f;
	unsigned char c;
	int i, j, w;

	if (argc != 3) {
		printf ("Usuage: sys2c [file.sys] [identifier]\n");
		return 1;
	}

	f = fopen (argv[1], "rb");
	if (f == NULL) {
		perror ("unable to open file");
		return 1;
	}
	fseek (f, 0, SEEK_END); size = ftell (f); rewind (f);

	if ((buf = (unsigned char*)malloc (size)) == NULL) {
		fclose (f);
		printf ("out of memory\n");
		return 1;
	}

	if (!fread (buf, size, 1, f)) {
		free (buf); fclose (f);
		perror ("unable to read file");
		return 1;
	}

	/* calc width */
	w = (80 - 25 - strlen (argv[2])) / 5;

	/* off we go */
	printf ("#include <sys/types.h>\n\n", argv[2]);
	printf ("unsigned char %s[] = { ", argv[2]);
	for (i = 0; i < size; i++) {
		printf ("0x%02x", buf[i]);
		if ((i) && (i % w == (w - 1))) {
			printf (",\n");
			for (j = 0; j < (15 + strlen (argv[2])); j++)
				printf (" ");
		} else
			if (i != (size - 1)) printf (", ");
	}
	printf (" };\n\n");
	printf ("#define %s_size %u\n", argv[2], size);
	free (buf); fclose (f);
	return 0;
}

/* vim:set ts=2: */
