/*
 * Subor: proj3.c
 * Datum: 5.12.2010 
 * Autor: Peter Lacko, xlacko06@stud.fit.vutbr.cz
 * Projekt: Maticove operacie: Proj c. 3 pre predmet IZP
 * Popis: Program prevazda rozne operacie s vektormi a maticami.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>


enum tecodes
{
  EOK,		// vsetko ok
  EPARAMS,	// chybny vstup na prikazovam riadku
  EOFILE,	// subor sa nepodarilo otvorit
  ERFILE,	// nespravny format dat suboru
  ECOMPUTE,	// nie je mozne vykonat operaciu
  EMALLOC,	// nedostatok pamate
  EUNKNOWN,	// neznama chyba
};

enum tstate
{
  HELP,		// napoveda
  TEST,		// test suboru
  VADD,		// sucet dvoch vektorov
  VSCAL,	// skalarny sucin dvoch vektorov
  MMULT,	// sucin dvoch matic
  MEXPR,	// vypocita maticovy vyraz A*B*A
  EIGHT,	// vyhlada vektor v matici
  BUBBL,	// spocita "bubliny" v matici
};

const char *ECODEMSG[] =
{
  "Zda sa ze je vsetko ok\n",			// EOK
  "Chybne parametre prikazoveho riadku\n",	// EPARAMS
  "Vstupný súbor neexistuje ...\n",		// EOFILE
  "Chybny format vstupneho suboru/suborov\n",		// ERFILE
  "Nie je mozne vykonat operaciu\n",		// ECOMPUTE
  "Nie je možné alokovať pamat\n",			// EMALLOC
  "Toto by sa nemalo nikdy vypisat\n",		// EUNKNOWN
};

const char *HELPMSG = 
{
  "\nProgram: Maticove operacie\n\
   Autor: Peter Lacko, xlacko06, december 2010\n\
   Program sluzi na vypocet suctu dvoch vektorov,\n\
   skalarneho sucinu vektorov, sucin dvoch matic,\n\
   vyhladanie vektora v matici (osemsmerovka),\n\
   a zistenie poctu \"bublin\" v matici.\n\n\
   Pouzitie:\n\
   proj3 PREPINAC [SUBOR1] [SUBOR2]\n\
   -h\n\
   	vypise tuto napovedu\n\
   --test SUBOR\n\
   	otestuje spravnost formatu suboru, a vylaci ho na stdout\n\
   --vadd VECTOR1 VECTOR2\n\
   	vykona sucet vektorov v zadanych suboroch, vysledok\n\
	sa zobrazi na stdout\n\
   --vscal VECTOR1 VECTOR2\n\
   	vykona skalarny sucin vektorov, vysledok sa zobrazi na stdout\n\
   --mmult MATRIX1 MATRIX2\n\
   	vykona sucin dvoch matic, vysledok sa ozbrazi na stdout\n\
   --mexpr MATRIX1 MATRIX2\n\
   	vykona sucin matic A*B*A, vysledok sa zobrazi na stdout\n\
   --eight VECTOR1 MATRIX1\n\
        zisti, ci sa VECTOR1 nachadza v MATRIX1\n\
   --bubbles MATRIX1\n\
   	zisti pocet \"bublin\" v matici\n",
};

typedef struct params
{
  int ecode;	// chybovy stav
  int state;	// stav programu
}TParams;

/*
 * struktura na uchovanie udajov o subore
 */
typedef struct file_info
{
  int type;	// "typ" suboru
  int nom;	// pocet matic
  int row;	// pocet riadkov
  int col;	// pocet stlpcov
  int *vec;
  int **matrix;
  int is_less;
}FInfo;

/*
 * struktura na pohyb v matici
 */
typedef struct position
{
  int **matrix;
  int y;
  int x;
  int rows;
  int cols;
}TPos;

void go_right(TPos ball);
void go_left(TPos ball);
void go_down(TPos ball);
void go_up(TPos ball);



/*
 * otestuje existenciu suboru
 */
int file_exist(char *file)
{
  FILE *fr;
  if((fr = fopen(file, "r")) != NULL)
  {
    fclose(fr);
    return 0;
  }
  return 1;
}


/*
 * spracuje parametre prikazoveho riadku
 */
TParams getParams(int argc, char *argv[])
{
  //FILE *fa;
  //FILE *fb;
  TParams result = 
  {
    .ecode = EOK,
    .state = HELP,
    //.file_a = 1,
    //.file_b = 1,
  };
  if (argc == 2)
  {
    if (strcmp(argv[1], "-h") == 0)
      return result;
    else {
      result.ecode = EPARAMS;
      return result;
    }
  }
  else if (argc == 3)
  {
    if (strcmp(argv[1], "--test") == 0)
    {
      if(file_exist(argv[2]) == 1)
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = TEST;
      return result;
    }
    else if(strcmp(argv[1], "--bubbles") == 0)
    {
      if(file_exist(argv[2]))
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = BUBBL;
      return result;
    }
    else 
    {
      result.ecode = EPARAMS;
      return result;
    }
  }

  else if(argc == 4)
  {
    if (strcmp(argv[1], "--vadd") == 0)
    {
      if(file_exist(argv[2]) || file_exist(argv[3]))
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = VADD;
      return result;
    }
    else if(strcmp(argv[1], "--vscal") == 0)
    {
      if(file_exist(argv[2]) || file_exist(argv[3]))
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = VSCAL;
      return result;
    }
    else if(strcmp(argv[1], "--mmult") == 0)
    {
      if(file_exist(argv[2]) || file_exist(argv[3]))
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = MMULT;
      return result;
    }
    else if(strcmp(argv[1], "--mexpr") == 0)
    {
      if(file_exist(argv[2]) || file_exist(argv[3]))
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = MEXPR;
      return result;
    }
    else if(strcmp(argv[1], "--eight") == 0)
    {
      if(file_exist(argv[2]) || file_exist(argv[3]))
      {
	result.ecode = EOFILE;
	return result;
      }
      result.state = EIGHT;
      return result;
    }
    else 
    {
      result.ecode = EPARAMS;
      return result;
    }
  }
  else 
    result.ecode = EPARAMS;
  return result;
}

/*
 * vytlaci chybovu hlasku
 */
void printECode(int ecode)
{
  if (ecode < EOK || ecode > EUNKNOWN)
  { ecode = EUNKNOWN; }

  fprintf(stderr, "%s", ECODEMSG[ecode]);
}


/*
 * uvolni maticu
 */
void free_matrix(FInfo *matrix)
{
  for(int i = 0; i < matrix->row; i++)
    free(matrix->matrix[i]);
  free(matrix->matrix);
}


/*
 * vytlaci vektor vector
 */
void print_vector(FInfo *vec)
{
  printf("1\n");
  printf("%d\n", vec->col);
  for(int i = 0; i < vec->col; i++)
    printf("%d ", vec->vec[i]);
  putchar('\n');
}


/*
 * vytlaci maticu matrix
 */
void print_matrix(FInfo *matrix)
{
  printf("2\n");
  printf("%d %d\n", matrix->row, matrix->col);
  for(int i = 0; i < matrix->row; i++)
  {
    for(int j = 0; j < matrix->col; j++)
      printf("%d ", matrix->matrix[i][j]);
    putchar('\n');
  }
}

/*
 * vytlacenie vektoru matic
 */
void print_vom(FInfo *vom)
{
  printf("3\n%d %d %d\n", vom->nom, vom->row, vom->col);
    for(int j = 0; j < (vom->nom *vom->row); j++)
    {
      if((j%vom->row == 0) && (j !=0))
	putchar('\n');
      for(int k = 0; k < vom->col; k++)
	printf("%d ", vom->matrix[j][k]);
      putchar('\n');
    }
    putchar('\n');
}
/*
 * otestuje a naplni pole zo suboru
 */
int fill_from_file(char *matrix, FInfo *result)
{
  result->nom = 1;
  result->row = 1;
  result->col = 1;
  result->vec = NULL;
  result->matrix = NULL;
  int type;
  int test;
  int sum = 0;
  FILE *file = fopen(matrix, "r");
  if(!fscanf(file,"%d",&type) || feof(file)/*(result->type == EOF)*/ || (type < 1) || (type > 3))
  {
    fclose(file);
    return ERFILE;
  }
  if(result->type == 0)
    result->type = type;
  else if(type != result->type)
  {
    fclose(file);
    return ERFILE;
  }
  // naplnenie vektora
  if(result->type == 1)
  {
    if(!fscanf(file,"%d",&result->col) || (result->col <= 0) || /*(result->type == EOF)*/ feof(file))
    {
      fclose(file);
      return ERFILE;
    }
    result->vec = (int*) malloc(result->col * sizeof(int));
    if(result->vec == NULL)
    {
      fclose(file);
      return EMALLOC;
    }
    for(int i =0; i < result->col; i++)
    {
      if((fscanf(file, "%d", &result->vec[i]) == 0) || (feof(file) == 1))
	break;
      sum++;
    }
    fscanf(file, "%d", &test);
    if((sum != result->col) || !feof(file))
      return ERFILE;
    fclose(file);
    return EOK;
  }

  // naplenenie matice
  if(result->type == 2)
  {
    if((fscanf(file, "%d %d", &result->row, &result->col) != 2) || feof(file) || (result->row <= 0) || (result->col <= 0))
    {
      fclose(file);
      return ERFILE;
    }
    result->matrix = (int**) malloc(result->row * sizeof(int*));
    if(result->matrix == NULL)
    {
      fclose(file);
      return EMALLOC;
    }
    for(int i = 0; i < result->row; i++)
    {
      result->matrix[i] = (int*) malloc(result->col * sizeof(int));
      if(result->matrix[i] == NULL)
      {
	for(int j = 0; j < i; j++)
	  free(result->matrix[j]);
	fclose(file);
	free(result->matrix);
	return EMALLOC;
      }
    }
    for(int i = 0; i < result->row; i++)
      for(int j = 0; j < result->col; j++)
      {
	if(!fscanf(file, "%d", &result->matrix[i][j]) || feof(file))
	  break;
	if(result->matrix[i][j] < 0)
	  result->is_less = 1;
	sum++;
      }
    fscanf(file, "%d", &test);
    if(sum != (result->row * result->col) || !feof(file))
    {
      free_matrix(result);
      fclose(file);
      return ERFILE;
    }
    fclose(file);
    return EOK;
  }

  // naplnenie a otestovanie vektora matic
  if(result->type == 3)
  {
    if((fscanf(file, "%d %d %d", &result->nom, &result->row, &result->col) != 3) || feof(file))
    {
      fclose(file);
      return ERFILE;
    }
    if((result->nom <= 0) || (result->row <= 0) || (result->col <= 0))
    {
      fclose(file);
      return ERFILE;
    }
    result->matrix = (int**) malloc(result->nom * result->row * sizeof(int*));
    if(result->matrix == NULL)
    {
      fclose(file);
      return EMALLOC;
    }
    for(int i = 0; i < (result->nom * result->row); i++)
    {
      if((result->matrix[i] = (int*) malloc(result->col * sizeof(int))) == NULL)
      {
	fclose(file);
	for(int k =0; k < i; k++)
	  free(result->matrix[k]);
	free(result->matrix);
	return EMALLOC;
      }
    }
    // naplnenie vektora matic
      for(int j = 0; j < (result->nom *result->row); j++)
	for(int k = 0; k < result->col; k++)
	{
	  if(!fscanf(file, "%d", &result->matrix[j][k]) || feof(file))
	    break;
	  sum++;
	}
    fscanf(file, "%d", &test);
    if((sum != (result->nom * result->row * result->col)) || !feof(file))
    {
      for(int i = 0; i < (result->nom * result->row); i++)
	free(result->matrix[i]);
      free(result->matrix);
      fclose(file);
      return ERFILE;
    }
    fclose(file);
    return EOK;
  }
  else
    return EUNKNOWN;
  return EUNKNOWN;
}

/*
 * scitanie dvoch vektorov
 */
void add_vector(FInfo *vec_a, FInfo *vec_b, FInfo *vec_c )
{
  for(int i = 0; i < vec_a->col; i++)
    vec_c->vec[i] = vec_a->vec[i] + vec_b->vec[i];
}


/*
 * skalarny sucin vektorov
 */
int mul_vector(FInfo *vec_a, FInfo *vec_b)
{
  int sum = 0;
  for(int i = 0; i < vec_a->col; i++)
  {
    sum += (vec_a->vec[i] * vec_b->vec[i]);
  }
  return sum;
}


/*
 * nasobenie matic
 */
void mmul_num(FInfo *matrix_a, FInfo *matrix_b, FInfo *matrix_c)
{
  for(int i = 0; i < matrix_a->row; i++)
    for(int j = 0; j < matrix_b->col; j++)
    {
      matrix_c->matrix[i][j] = 0;
      for(int k = 0; k < matrix_a->col; k++)
	matrix_c->matrix[i][j] += (matrix_a->matrix[i][k] * matrix_b->matrix[k][j]);
    }

  //return EOK;
}


/*
 * osemsmerovka
 */
int eight_num(FInfo *matrix, FInfo *vector)
{
  int max;
  max = ( matrix->row > matrix->col ? matrix->row : matrix->col);
  if(vector->col > max)
    return 0;
  for(int x = 0; x < matrix->row; x++)
    for(int y = 0; y < matrix->col; y++)
    {
      if(vector->vec[0] == matrix->matrix[x][y])
      {
	int i = x;
	int j = y;
	int k = 0;
	// smer doprava
	while(((matrix->col - j) > 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	    return 1;
	  j++;
	  k++;
	}
	// smer dolava
	j = y;
	k = 0;
	while((j >= 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	    return 1;
	  j--;
	  k++;
	}
	// smer nadol
	j = y;
	k = 0;
	while(((matrix->row - i) > 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	    return 1;
	  i++;
	  k++;
	}
	// smer nahor
	i = x;
	k = 0;
	while((i >= 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	    return 1;
	  i--;
	  k++;
	}
	// smer vpravo hore
	i = x;
	k = 0;
	while(((matrix->col - j) > 0) && (i >= 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	    return 1;
	  j++;
	  k++;
	  i--;
	}
	//smer vpravo dole
	i = x;
	j = y;
	k = 0;
	while(((matrix->col - j) > 0) && ((matrix->row - i) > 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col -1))
	    return 1;
	  j++;
	  i++;
	  k++;
	}
	//smer vlavo dole
	i = x;
	j = y;
	k = 0;
	while((j >= 0) && ((matrix->row - i) > 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	    return 1;
	  i++;
	  k++;
	  j--;
	}
	// smer vlavo hore
	i = x;
	j = y;
	k = 0;
	while((i >= 0) && (j >= 0) && (matrix->matrix[i][j] == vector->vec[k]))
	{
	  if(k == (vector->col - 1))
	      return 1;
	  k++;
	  i--;
	  j--;
	}
      }
    }
    return 0;
}

/*
 * scitanie vektorov
 */
int add_vectors(char *file_a, char *file_b)
{
  FInfo vec_a;
  vec_a.type = 1;
  FInfo vec_b;
  vec_b.type = 1;
  FInfo vec_c =
  {
    .type = 1,
    .nom = 1,
    .row = 1,
    .col = 1,
    .vec = NULL,
    .matrix = NULL,
  };
  int value;
  if((value = fill_from_file(file_a, &vec_a)) != EOK)
    return value;
  if((value = fill_from_file(file_b, &vec_b)) != EOK)
  {
    free(vec_a.vec);
    return value;
  }
  if(vec_a.col != vec_b.col)
  {
    free(vec_a.vec);
    free(vec_b.vec);
    return ECOMPUTE;
  }
  vec_c.vec = (int*) malloc(vec_a.col * sizeof(int));
  if(vec_c.vec == NULL)
  {
    free(vec_a.vec);
    free(vec_b.vec);
    return EMALLOC;
  }
  vec_c.col = vec_a.col;
  add_vector(&vec_a, &vec_b, &vec_c);
  print_vector(&vec_c);
  free(vec_a.vec);
  free(vec_b.vec);
  free(vec_c.vec);
  return EOK;
}

/*
 * funkcia vytlaci subor
 */
int print_file(char *file)
{
  FInfo ffile;
  ffile.type = 0;
  int value;
  value = fill_from_file(file, &ffile);
  if(value != EOK)
  {
    return value;
  }
  switch (ffile.type)
  {
    case 1: print_vector(&ffile);
	    free(ffile.vec);
	    break;
    case 2: print_matrix(&ffile);
	    free_matrix(&ffile);
	    break;
    case 3: print_vom(&ffile);
	    for(int i =0; i < (ffile.nom *ffile.row); i++)
	      free(ffile.matrix[i]);
	    free(ffile.matrix);
	    break;
    default:
      break;
  }
  return EOK;
}

/*
 * nasobenie vektorov
 */
int multiple_vec(char *file_a, char *file_b)
{
  int result;
  FInfo vec_a;
  vec_a.type = 1;
  FInfo vec_b;
  vec_b.type = 1;
  int value;
  if((value = fill_from_file(file_a, &vec_a)) != EOK)
    return value;
  if((value = fill_from_file(file_b, &vec_b)) != EOK)
  {
    free(vec_a.vec);
    return value;
  }
  if(vec_a.col != vec_b.col)
  {
    free(vec_a.vec);
    free(vec_b.vec);
    return ECOMPUTE;
  }
  result = mul_vector(&vec_a, &vec_b);
  free(vec_a.vec);
  free(vec_b.vec);
  printf("%d\n", result);
  return EOK;
}

/*
 * nasobenie matic
 */
int mmul(char *file_a, char *file_b)
{
  FInfo mat_a;
  mat_a.type = 2;
  FInfo mat_b;
  mat_b.type = 2;
  FInfo mat_c = {
    .type = 1,
    .nom = 1,
    .row = 1,
    .col = 1,
    .vec = NULL,
    .matrix = NULL,
  };
  int value;
  if((value = fill_from_file(file_a, &mat_a)) != EOK)
    return value;
  if((value = fill_from_file(file_b, &mat_b)) != EOK)
  {
    free_matrix(&mat_a);
    return value;
  }
  if(mat_a.col != mat_b.row)
    return ECOMPUTE;
  mat_c.matrix = (int**) malloc(mat_a.row * sizeof(int*));
  if(mat_c.matrix == NULL)
  {
    free_matrix(&mat_a);
    free_matrix(&mat_b);
    return EMALLOC;
  }
  for(int i = 0; i < mat_a.row; i++)
  {
    if((mat_c.matrix[i] = (int*) malloc(mat_b.col * sizeof(int))) == NULL)
    {
      for(int k = 0; k < i; k++)
	free(mat_c.matrix[k]);
      free(mat_c.matrix);
      return EMALLOC;
    }
  }
  mat_c.row = mat_a.row;
  mat_c.col = mat_b.col;
  mmul_num(&mat_a, &mat_b, &mat_c);
  print_matrix(&mat_c);
  free_matrix(&mat_a);
  free_matrix(&mat_b);
  free_matrix(&mat_c);
  return EOK;
}

/*
 * nasobenie A*B*A
 */
int mexpr(char *file_a, char *file_b)
{
  FInfo mat_a;
  mat_a.type = 2;
  FInfo mat_b;
  mat_b.type = 2;
  int value;
  if((value = fill_from_file(file_a, &mat_a)) != EOK)
    return value;
  if((value = fill_from_file(file_b, &mat_b)) != EOK)
  {
    free_matrix(&mat_a);
    return value;
  }
  if((mat_a.col != mat_b.row) || (mat_b.col != mat_a.row))
  {
    free_matrix(&mat_a);
    free_matrix(&mat_b);
    return ECOMPUTE;
  }

  FInfo mat_c =
  {
    .type = 2,
    .nom = 1,
    .row = mat_a.row,
    .col = mat_b.col,
    .vec = NULL,
    .matrix = NULL,
    .is_less = 0
  };
  FInfo mat_d =
  {
    .type = 2,
    .nom = 1,
    .row = mat_a.row,
    .col = mat_a.col,
    .vec = NULL,
    .matrix = NULL,
    .is_less = 0
  };
  mat_c.matrix = (int**) malloc(mat_a.row * sizeof(int*));
  if(mat_c.matrix == NULL)
  {
    free_matrix(&mat_a);
    free_matrix(&mat_b);
    return EMALLOC;
  }
  for(int i = 0; i < mat_a.row; i++)
  {
    if((mat_c.matrix[i] = (int*) malloc(mat_b.col * sizeof(int))) == NULL)
    {
      for(int k = 0; k < i; k++)
	free(mat_c.matrix[k]);
      free(mat_c.matrix);
      free_matrix(&mat_a);
      free_matrix(&mat_b);
      return EMALLOC;
    }
  }
  mat_d.matrix = (int**) malloc(mat_a.row * sizeof(int*));

  if(mat_d.matrix == NULL)
  {
    free_matrix(&mat_a);
    free_matrix(&mat_b);
    free_matrix(&mat_c);
    return EMALLOC;
  }
  for(int i = 0; i < mat_a.row; i++)
  {
    if((mat_d.matrix[i] = (int*) malloc(mat_a.col * sizeof(int))) == NULL)
    {
      for(int k = 0; k < i; k++)
	free(mat_d.matrix[k]);
      free(mat_d.matrix);
      free_matrix(&mat_a);
      free_matrix(&mat_b);
      free_matrix(&mat_c);
      return EMALLOC;
    }
  }

  mmul_num(&mat_a, &mat_b, &mat_c);
  mmul_num(&mat_c, &mat_a, &mat_d);
  print_matrix(&mat_d);
  free_matrix(&mat_a);
  free_matrix(&mat_b);
  free_matrix(&mat_c);
  free_matrix(&mat_d);
  return EOK;
}

/*
 * osemsmerovka
 */
int eight(char *file_b, char *file_a)
{
  FInfo mat;
  mat.type = 2;
  FInfo vec;
  vec.type = 1;
  int value;
  int ret_value;
  if((value = fill_from_file(file_a, &mat)) != EOK)
    return value;
  if((value = fill_from_file(file_b, &vec)) != EOK)
  {
    free_matrix(&mat);
    return value;
  }
  ret_value = eight_num(&mat, &vec);
  if(ret_value == 0)
    printf("false\n");
  else
    printf("true\n");
  free_matrix(&mat);
  free(vec.vec);
  return EOK;
}

/*
 * funkcie na zistenie poctu bublin v matici
 */
void go_right(TPos ball)
{
  while((ball.x <= ball.cols) && (ball.matrix[ball.y][ball.x] == 0))
  {
    if((ball.y > 0) && (ball.matrix[ball.y-1][ball.x] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.y--;
      go_up(ball);
      ball.y++;
    }
    if((ball.y < ball.rows) && (ball.matrix[ball.y+1][ball.x] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.y++;
      go_down(ball);
      ball.y--;
    }

    ball.matrix[ball.y][ball.x] = 1;
    ball.x++;
  }
}

void go_left(TPos ball)
{
  while((ball.x >= 0) && (ball.matrix[ball.y][ball.x] == 0))
  {
    if((ball.y > 0) && (ball.matrix[ball.y-1][ball.x] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.y--;
      go_up(ball);
      ball.y++;
    }
    if((ball.y < ball.rows) && (ball.matrix[ball.y+1][ball.x] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.y++;
      go_down(ball);
      ball.y--;
    }

    ball.matrix[ball.y][ball.x] = 1;
    ball.x--;
  }
}

void go_up(TPos ball)
{
  while((ball.y >= 0) && (ball.matrix[ball.y][ball.x] ==0))
  {
    if((ball.x > 0) && (ball.matrix[ball.y][ball.x-1] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.x--;
      go_left(ball);
      ball.x++;
    }
    if((ball.x < ball.cols) && (ball.matrix[ball.y][ball.x+1] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.x++;
      go_right(ball);
      ball.x--;
    }
    ball.matrix[ball.y][ball.x] = 1;
    ball.y--;
  }
}

void go_down(TPos ball)
{
  while((ball.y <= ball.rows) && (ball.matrix[ball.y][ball.x] == 0))
  {
    if((ball.x > 0) && (ball.matrix[ball.y][ball.x-1] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.x--;
      go_left(ball);
      ball.x++;
    }
    if((ball.x < ball.cols) && (ball.matrix[ball.y][ball.x+1] == 0))
    {
      ball.matrix[ball.y][ball.x] = 1;
      ball.x++;
      go_right(ball);
      ball.x--;
    }
    ball.matrix[ball.y][ball.x] = 1;
    ball.y++;
  }
}

int bubbles_num(FInfo *mat)
{

  TPos bubl =
  {
    .matrix = mat->matrix,
    .x = 0,
    .y = 0,
    .rows = mat->row-1,
    .cols = mat->col-1,
  };
  //int z;
  int num = 0;
  for(bubl.y =  0; bubl.y < mat->row; bubl.y++)
    for(bubl.x = 0; bubl.x < mat->col; bubl.x++)
    {
      //z = bubl.x;
      //bubl.x = bubl.y;
      //bubl.y = z;
      if(mat->matrix[bubl.y][bubl.x] == 0)
      {
	go_right(bubl);
	num++;
      }
    }
  return num;
}

int bubbles(char *file)
{
  int value;
  FInfo mat;
  mat.type = 2;
  mat.is_less = 0;
  value = fill_from_file(file, &mat);
  if(value != EOK)
    return value;
  if(mat.is_less == 1)
  {
    free_matrix(&mat);
    return ECOMPUTE;
  }
  value = bubbles_num(&mat);
  printf("%d\n", value);
  free_matrix(&mat);
  return EOK;
}



/*
 * hlavna funkcia
 */
int main(int argc, char *argv[])
{
  int return_code;
  TParams result = getParams(argc, argv);
  if(result.ecode != EOK)
  {
    printECode(result.ecode);
    return EXIT_FAILURE;
  }
  else if(result.state == HELP)
  {
    printf("%s", HELPMSG);
    return EXIT_SUCCESS;
  }
  else if(result.state == TEST)
  {
    return_code = print_file(argv[2]);
    if(return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else if(result.state == VADD)
  {
    return_code = add_vectors(argv[2], argv[3]);
    if (return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else if(result.state == VSCAL)
  {
    return_code = multiple_vec(argv[2], argv[3]);
    if (return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else if(result.state == MMULT)
  {
    return_code = mmul(argv[2], argv[3]);
    if (return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else if(result.state == MEXPR)
  {
    return_code = mexpr(argv[2], argv[3]);
    if (return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else if(result.state == EIGHT)
  {
    return_code = eight(argv[2], argv[3]);
    if(return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else if(result.state == BUBBL)
  {
    return_code = bubbles(argv[2]);
    if(return_code != EOK)
    {
      printECode(return_code);
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  else 
  {
    printECode(EUNKNOWN);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
