/*
 * =====================================================================================
 *
 *       Filename:  readerWriter.c
 *
 *    Description:  projekt c 2 do kurzu IOS
 *
 *        Version:  1.0
 *        Created:  05.04.2012 15:56:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *        Company:  
 *
 * =====================================================================================
 */

#include "readerWriter.h"

/*
 * pomocna funkce na prevod retezce na cislo
 * @param prevadzane cislo
 */
int to_digit (char *str) {
    char *E;
    int ret_value = strtol(str, &E, 0);
    if (str[0] == '\0' || *E != '\0')
	return -1;
    else if (ret_value < 0)
	return -1;
    return ret_value;
}

/*
 * funkcia sprauvajuca argumenty prikazoveho riadku
 * @param pocet parametrov
 * @param parametre
 * @param struktura kam sa ulozia vysledky
 */
int handle_args(int argc, char **argv, params *param) {
    if (argc != 7)
	return 1;
    param->W = to_digit( argv[1] );
    param->R = to_digit( argv[2] );
    param->C = to_digit( argv[3] );
    param->SW = to_digit( argv[4] );
    param->SR = to_digit( argv[5] );
    if (param->W==-1 || param->W == 0 || param->R==-1 || param->C==-1 || param->SW==-1 || param->SR==-1) {
	return 1;
    }
    return 0;
}

/*
 * hlavna funkcia
 */
int main(int argc, char **argv) {
    params param;
    if (handle_args (argc, argv, &param) != 0) {
	printf("readerWriter W R C SW SR OUT\n\
		W   :  number of readers\n\
		R   :  number of writers\n\
		C   :  number of cycles\n\
		SW  :  range for reader simulation [ms]\n\
		SR  :  range for writer simulation [ms]\n\
		OUT :  name of filename for output, or \"-\" for stdout\n");
	exit(EXIT_FAILURE);
    }
    bool failed = false; // indikace chyby

    // otvorenie vystupneho suboru
    FILE *file;
    if (!strcmp (argv[6], "-")) {
	file = stdout;
    }
    else {
	if ((file = fopen(argv[6], "w")) == NULL)
	    exit (EXIT_FAILURE);
	setvbuf(file, NULL, _IOLBF, 0);
    }

    int shmid;
    key_t key;
    char *shm;
    int *w_id, *counter, *w_ended; // promenne ukazujuce do zdielanej pamate
    int *r_counter, *w_counter;

    // generovanie kluca
    key = ftok(argv[0], FTOK_NUMBER);

    // vytvorenie zdielaneho segmentu
    if ((shmid = shmget(key, 5*sizeof(int), IPC_CREAT|IPC_EXCL|0600)) < 0) {
	fclose (file);
	perror("shmget");
	exit(EXIT_FAILURE);
    }

    // naviazanie segmentu k pamati 
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
	fclose (file);
	shmctl (shmid, IPC_RMID, NULL);
	perror ("shmat");
	exit (EXIT_FAILURE);
    }

    // nastavenie "sdileneho prostoru" a "citaca akci"
    w_id = (int*) shm;
    counter = (int*) (shm + sizeof(int));

    // inicializace citacov ctenaru a pisaru
    r_counter =  (int*) (shm + 2 * sizeof(int));
    w_counter = (int*) (shm + 3 * sizeof(int));
    w_ended = (int*) (shm + 4 * sizeof(int));

    *w_id = -1;
    *counter = 1;
    *r_counter = 0;
    *w_counter = 0;
    *w_ended = 0;

    // deklaracia pouzitych semaforov
    sem_t *sem_1; 
    sem_t *sem_2; 
    sem_t *sem_3; 
    sem_t *sem_r;   // povolenie ku cteni
    sem_t *sem_w;   // povolenie na zapis
    sem_t *sem_o;   // zapis do suboru
    sem_t *sem_we;  // pocitadlo ukoncenych writerov

    // inicializace semaforov
    if (SEM_FAILED == (sem_1 = sem_open("/sem_1", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;
    if (SEM_FAILED == (sem_2 = sem_open("/sem_2", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;
    if (SEM_FAILED == (sem_3 = sem_open("/sem_3", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;
    if (SEM_FAILED == (sem_r = sem_open("/sem_r", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;
    if (SEM_FAILED == (sem_w = sem_open("/sem_w", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;
    if (SEM_FAILED == (sem_o = sem_open("/sem_o", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;
    if (SEM_FAILED == (sem_we = sem_open("/sem_we", O_CREAT, S_IRUSR | S_IWUSR, 1)))
	failed = true;

    // pocet vykonani cyklu: max z {readers, writers} alebo 0, ak sa niekde stala chyba
    int loops;
    if (failed)
	loops = 0;
    else
        loops = (param.W > param.R ? param.W : param.R);

    pid_t PID, PID2; // pid readera a writera

    for (int rw_id = 1; rw_id <= loops; rw_id++) {
	srand(time(NULL));
	if (rw_id <= param.R)
	    PID = fork(); // fork READER
	else
	    PID = 1; // lubovolne cislo > 0

	if (PID > 0) {

	    if (rw_id <= param.W)
		PID2 = fork(); // fork WRITER
	    else 
		PID2 = 1;
	    if (PID2 == 0) {
		/** -------------------------------- **/
		/*  begin WRITER                      */
		/** -------------------------------- **/
		int randomizer = 0; // gen. nahodnych hodnot v kazdom procese i cykle
				    // v skutocnosti puhe pocitadlo cyklov :-)
		for (int i = 0; i < param.C; i++) {
		    LOCK(sem_o);
		    fprintf(file, "%d: writer: %d: new value\n", (*counter), rw_id);
		    ++(*counter);
		    UNLOCK(sem_o);

		    // uspani procesu
		    srand(time(NULL)^(getpid()+randomizer));
		    usleep((rand()%(param.SW+1))*1000);
		    randomizer++;

		    LOCK(sem_o);
		    fprintf(file, "%d: writer: %d: ready\n", (*counter), rw_id);
		    ++(*counter);
		    UNLOCK(sem_o);

		    LOCK(sem_2);
		    ++(*w_counter);		// inkrementace pocitadla pisaru
		    if (*w_counter == 1)	// a blokovani ctenaru
			LOCK(sem_r);
		    UNLOCK(sem_2);

		    LOCK(sem_w);
		    LOCK(sem_o);
		    fprintf(file, "%d: writer: %d: writes a value\n", (*counter), rw_id);
		    ++(*counter);
		    UNLOCK(sem_o);

		    *w_id = rw_id; // vlozeni hodnoty do zdielanej pamate

		    LOCK(sem_o);
		    fprintf(file, "%d: writer: %d: written\n", (*counter), rw_id);
		    ++(*counter);
		    UNLOCK(sem_o);
		    UNLOCK(sem_w);

		    LOCK(sem_2);
		    --(*w_counter);		// dekrementace pisaru
		    if (*w_counter == 0)	// pripadne odblokovani ctenaru
			UNLOCK(sem_r);		
		    UNLOCK(sem_2);

		} // rof
		LOCK(sem_we);		// pocitadlo ukoncenych pisaru
		++(*w_ended);
		if (*w_ended == param.W) {
		    LOCK(sem_r);
		    LOCK(sem_w);
		    *w_id = 0;		// a nastaveni zdielanej pamate na 0
		    UNLOCK(sem_w);
		    UNLOCK(sem_r);
		}
		UNLOCK(sem_we);
		exit(EXIT_SUCCESS);
	    } 
	    /** -------------------------------- **/
	    /*  end WRITER                        */
	    /** -------------------------------- **/
	    else if (PID2 < 0) {
		perror("fork()");
		failed = true;
		rw_id = loops+1;
	    }
	} // fi PID > 0 

	/** -------------------------------- **/
	/*  begin READER                      */
	/** -------------------------------- **/
	else if (PID == 0) { 
	    int read_value;
	    int randomizer = 0; // koli generovaniu skutocne nahodnych hodnot
	    do {
		LOCK(sem_o);
		fprintf(file, "%d: reader: %d: ready\n", (*counter), rw_id);
		++(*counter);
		UNLOCK(sem_o);

		LOCK(sem_3);
		LOCK(sem_r);   
		LOCK(sem_1);
		++(*r_counter);		// inkrementace pocitadla ctenaru
		if (*r_counter == 1)    // a zablokovani pisaru
		    LOCK(sem_w);
		UNLOCK(sem_1);
		UNLOCK(sem_r);
		UNLOCK(sem_3);

		LOCK(sem_o);
		fprintf(file, "%d: reader: %d: reads a value\n", (*counter), rw_id);
		++(*counter);
		UNLOCK(sem_o);

		read_value = *w_id;	// cteni hodnoty zo zdielaneho prostoru

		LOCK(sem_o);
		fprintf(file, "%d: reader: %d: read: %d\n", (*counter), rw_id, read_value);
		++(*counter);
		UNLOCK(sem_o);

		LOCK(sem_1);
		--(*r_counter);		// dekrementace pocitadla, uvolneni pisaru
		if (*r_counter == 0)
		    UNLOCK(sem_w);
		UNLOCK(sem_1);

		// uspanie procesu
		srand(time(NULL)^(getpid()+randomizer));
		usleep((rand()%(param.SR+1))*1000);

		++randomizer;
	    } while (read_value != 0);
	    exit(EXIT_SUCCESS);
	} 
	/** -------------------------------- **/
	/*  end READER                        */
	/** -------------------------------- **/
	else if (PID2 < 0) {
	    perror("fork()");
	    failed = true;
	    rw_id = loops+1;
	}
    } // rof ...


    // cakanie za smrtou vsetkych potomkov
    for (int i = 0; i < param.W + param.R; i++) {
	waitpid(-1, NULL, 0);
    }

    // uvoleneni zdielanej pamate, semaforov
    shmdt (shm);
    shmctl (shmid, IPC_RMID, NULL);

    sem_unlink("/sem_1");
    sem_unlink("/sem_2");
    sem_unlink("/sem_3");
    sem_unlink("/sem_r");
    sem_unlink("/sem_w");
    sem_unlink("/sem_o");
    sem_unlink("/sem_we");

    failed ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
