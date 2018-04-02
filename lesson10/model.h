typedef struct tagVERTEX
{
	float x, y, z;
	float u, v;
} VERTEX;


typedef struct tagTRIANGLE
{
	VERTEX vertex[3];
} TRIANGLE;

typedef struct tagSECTOR
{
	int numtriangles;
	TRIANGLE* triangle;
} SECTOR;