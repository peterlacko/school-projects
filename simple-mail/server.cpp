/*
 * =====================================================================================
 *
 *       Filename:  server.cpp
 *
 *    Description:  program "server" pro projekt #2 do kurzu IPK
 *
 *        Version:  1.0
 *        Created:  03.03.2012 14:10:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *        Company:  
 *
 * =====================================================================================
 */

#include "server.h"

inline void handleError (int RValue, const char *msg) {
	std::cerr << "error: " << msg << std::endl;
	exit(RValue);
}

// zachytenie signalov SIGINT a SIGTERM a nasledny
// zapis dat do suboru
static void *sigThreadCatch (void *arg) {
	sigset_t *set = static_cast<sigset_t*>(arg);
	int s, sig;
	while (1) {
		s = sigwait(set, &sig);
		if (s != 0) {
			errno = s;
			perror("sigwait");
			exit(EXIT_FAILURE);
		}
		pthread_mutex_lock (&dataMutex);
		std::vector<std::string>::iterator fCiter;

		// a otvoreni souboru pouze pro zapis
		file.open(filename);
		for (fCiter = fileCopy.begin(); fCiter != fileCopy.end(); fCiter++) 
			file << *fCiter << "\n";
		file.flush();
		file.close();
		pthread_mutex_destroy (&dataMutex);
		kill(getpid(), SIGINT);
		exit (EXIT_SUCCESS);
	}
}

// spracovanie argumentov prikazoveho riadku
int handleArgs (int argc, char **argv) {
	if (argc != 3)
		return -1;
	if ((strlen(argv[1]) != 2) || strcmp(argv[1], "-p"))
		return -1;
	char *ch = new char;
	unsigned port = strtol(argv[2], &ch, 0);
	if ((*ch == '\0') && (port > 0)) {
		return port;
	}
	return -1;
}

// obsluha jedneho spojenia s klientom realizovana multithreadingom
void *handleConnection( void *data){
	size_t dataSocket = *(static_cast<int*>(data));
	std::string request;
	std::string reply;
	char requestBuff[BuffSize];
	ssize_t requestSize = 0;
	while ((requestSize = read(dataSocket, requestBuff, sizeof(requestBuff))) > 0) {
		request.append(requestBuff, requestSize);
		if (requestBuff[requestSize-1] == '\n')
			break;
	}
	//odstraneni koncoveho konca noveho radku
	request.erase(request.length()-1, 1);
	// ziskanie typu requestu
	size_t echar = request.find_first_of(" ", 0);
	std::string reqType = request.substr(0, echar);
	request.erase(0, echar+1);

	// ziskanie uzivatelskeho mena
	echar = request.find_first_of(' ', 0);
	std::string username = request.substr(0, echar);
	request.erase(0, echar+1);

	// v requeste ostal zbytok, tj cislo mazanej spravy, alebo sprava

	// uzamknutie mutexu
	pthread_mutex_lock(&dataMutex);

	// praca s vektorom, eventuelne ulozenie do suboru
	bool found = false;	// pomocny flag
	std::vector<std::string>::iterator fCiter; // iterator nad vektorom
	if (reqType == "READ") {
		for (fCiter = fileCopy.begin(); fCiter != fileCopy.end(); fCiter++) {
			// hlada uzivatela
			if ((*fCiter)[0] == 'U' && (*fCiter).substr(2, (*fCiter).length()) == username) {
				fCiter++;
				// uzivatel nalezen, uklada spravy do odpovede
				while (fCiter != fileCopy.end() && (*fCiter)[0] == 'M') {
					reply = reply + (*fCiter).substr(2, (*fCiter).length()) + "\n";
					fCiter++;
				}
				found = true;
				break;
			} // end if
		} // end for
		if (!found)
			reply = "\n\n";
	}
	else if (reqType == "WRITE") {
		for (fCiter = fileCopy.begin(); fCiter != fileCopy.end(); fCiter++) {
			// ak je uzivatelske meno na radku
			if ((*fCiter)[0] == 'U' && (*fCiter).substr(2, (*fCiter).length()) == username) {
				found = true;
				fCiter++;
				// preiteruje sa na koniec sprav
				while (fCiter != fileCopy.end() && (*fCiter)[0] == 'M') 
					fCiter++;
				// vlozi spravu na koniec
				fileCopy.insert(fCiter, "M " + request);
				reply += "Ok.\n";
				break;
			}
		}
		if (!found) {
			// static_cast<threadData*>(data)->fileCopy.push_back("\n");
			fileCopy.push_back("U " + username);
			fileCopy.push_back("M " + request);
			reply += "Ok.\n";
		}
	}

	else if (reqType == "DELETE") {
		unsigned idx = atoi(request.c_str());
		for (fCiter = fileCopy.begin(); fCiter != fileCopy.end(); fCiter++) {
			// vyhladanie uzivatelskeho mena
			if ((*fCiter)[0] == 'U' && (*fCiter).substr(2, (*fCiter).length()) == username) {
				fCiter++;
				unsigned j = 1;
				while (fCiter != fileCopy.end() && (*fCiter)[0] == 'M') { 
					if ( idx == j ) {
						found = true;
						fileCopy.erase(fCiter);
						break;
					}
					j++;
					fCiter++;
				} // end while
			} // end if
		} // end for

		reply = found ? "Ok.\n" : "Err.\n";
	}
	// sleep(5);

	//odomknutie mutexu
	pthread_mutex_unlock(&dataMutex);

	if ((write (dataSocket, reply.c_str(), reply.size())) == -1) {
		std::cerr << "error: write to socket failed\n";
	}
	close(dataSocket);
	pthread_exit ((void *) 0);
}

// hlavna funkcia
int main (int argc, char **argv) {
	unsigned port;
	if ((port = handleArgs (argc, argv)) == -1) {
		handleError (WRONG_PARAMS, "wrong parameters on commandline");
	}

	int listenSocket;
	if((listenSocket = socket( PF_INET, SOCK_STREAM, 0)) < 0) {
		handleError(INTERNAL_ERROR, "cannot create socket");
	}

	sockaddr_in s1, s2;
	socklen_t s2Len = sizeof(s2);
	memset (&s1, 0, sizeof(s1));
	s1.sin_family = AF_INET;
	s1.sin_port = htons(port);
	s1.sin_addr.s_addr = INADDR_ANY;

	int tr = 1;
	if (setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) {
		close(listenSocket);
		handleError(INTERNAL_ERROR, "setsockopt failed");
	}

	if (bind( listenSocket, reinterpret_cast<sockaddr*>(&s1), sizeof(s1)) != 0) {
		close(listenSocket);
		handleError(INTERNAL_ERROR, "bind() failed");
	}

	if (listen(listenSocket, 0) != 0) {
		close(listenSocket);
		handleError(CONNECTION_FAILED, "listen() failed");
	}

	// otvorenie suboru s datami
	std::ifstream infile;
	infile.open(filename);
	infile.is_open(); 
	// nacitani suboru do vektoru
	std::string line;
	if (infile.is_open()) {
	while (getline(infile, line))
		fileCopy.push_back(line);
	}
	infile.close();

	// vytvoreni threadu pro signaly a mutexov
	pthread_t thread;
	pthread_attr_t attrt;
	pthread_attr_init (&attrt);
	pthread_attr_setdetachstate (&attrt, PTHREAD_CREATE_DETACHED);

	if (pthread_mutex_init (&dataMutex, NULL) != 0) {
		close (listenSocket);
		file.close();
		handleError(INTERNAL_ERROR, "mutex init failed");
	}
	size_t dataSocket;

	pthread_t sigThread;
	sigset_t set;
	int s;
	sigemptyset(&set);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	s = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if (s != 0)
		handleError (INTERNAL_ERROR, "pthread_create");
	s = pthread_create(&sigThread, NULL, sigThreadCatch, &set);
	if (s != 0)
		handleError(INTERNAL_ERROR, "pthread_create");

	while (1) {
		dataSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&s2), &s2Len);
		if (dataSocket <= 0)
			continue;
		pthread_create (&thread, &attrt, handleConnection, &dataSocket);
	}
}
