/*
 * =====================================================================================
 *
 *       Filename:  webinfo.cpp
 *
 *    Description:  projekt 1 do kurzu IPK
 *
 *        Version:  1.0
 *        Created:  26.02.2012 23:34:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Peter Lacko (xlacko06), xlacko06@stud.fit.vutbr.cz
 *
 * =====================================================================================
 */

#include "webinfo.h"

// pripojeni k servru a komunikace se servrem
int connectAndWrite (std::string & reply, const uriInfo uri, int & sockNumber) {
	if((sockNumber = socket( PF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "socket() failed\n";
		return SOCKET_ERROR;
	}

	// ziskanie adresy hosta
	hostent *host = new hostent;
	host = gethostbyname(uri.domain.c_str());
	if (host == NULL) {
		std::cerr << "gethostbyname() failed\n";
		return CONNECTION_ERROR;
	}

	int port = atol(uri.port.c_str());
	sockaddr_in servSock;	// popis socketu
	servSock.sin_family = AF_INET;
	servSock.sin_port = htons(port);
	memcpy(&(servSock.sin_addr), host->h_addr_list[0], host->h_length);

	// pripojenie k serveru
	if ( connect(sockNumber, reinterpret_cast<sockaddr*>(&servSock), sizeof(servSock)) != 0) {
		std::cerr << "connect() failed\n";
		return CONNECTION_ERROR;
	}

	// http request
	std::string request = "HEAD " + uri.path + " HTTP/1.1\r\nHost: " + uri.domain
		+ "\r\nConnection: close\r\n\r\n";
	if (write( sockNumber, request.c_str(), request.length()) == -1) {
		std::cerr << "error: write to socket failed\n";
		close(sockNumber);
		return CONNECTION_ERROR;
	}

	char replyBuff[BUFFER_SIZE];
	unsigned replySize = 0;
	while ((replySize = read(sockNumber, replyBuff, sizeof(replyBuff))) > 0) {
		reply.append(replyBuff, replySize);
	}
	close (sockNumber);
	return 0;
}

// spracovani odpovede
int handleReply (std::string & reply, std::string & codeOrAddress, int & crlf) {
	// najpr z odpovede odstranime "Connection: close"
	size_t sp, ep;
	sp = reply.find("Connection: close");
	ep = reply.find_first_of('\n', sp);
	reply.erase(sp, ep-sp+1);

	// ziskame navratovu hodnotu
	regmatch_t errMatch[2];
	regex_t errPreg;
	std::string retValue;
	std::string errPattern = "^HTTP/1.[01] ([0-9]{3}[^\r\n]*)";
	if (regcomp(&errPreg, errPattern.c_str(), REG_EXTENDED) != 0) {
		return INTERNAL_ERROR;
	}
	int regexecStatus = regexec(&errPreg, reply.c_str(), 2, errMatch, 0);
	if (regexecStatus != 0) {
		return INTERNAL_ERROR;
	}
	else {
		retValue = reply.substr(errMatch[1].rm_so, errMatch[1].rm_eo-errMatch[1].rm_so);
		if (retValue[0] != '2' && retValue[0] != '1') {
			if (retValue [0] == '3' && (retValue[2] == '1' || retValue[2] == '2')) {
				
				sp = reply.find("Location: ") + 10; // 10 = dlzka retezce "Location: "
				ep = reply.find_first_of(crlf, sp);
				codeOrAddress = reply.substr(sp, ep-sp);
				return HTTP_REDIRECT;
			}
			else {
				codeOrAddress = retValue;
				return HTTP_ERROR;
			}
		}
	}
	return HTTP_OK;
}

// naparsovani adresy
int parseUri (const char * uriStr_c, uriInfo & uri) {
	std::string uriStr = uriStr_c;
	regmatch_t strMatch[5];
	regex_t uriRe;
	int status;

	std::string uriPattern = "^http://([^/?#:]*)(:([0-9]+)?)?(.*)$";//(\?([^#]*))?(#(.*))?";
	if (regcomp(&uriRe, uriPattern.c_str(), REG_EXTENDED) != 0) {
		return INTERNAL_ERROR;
	}
	status = regexec(&uriRe, uriStr.c_str(), 5, strMatch, 0);
	if (status == 0) {
		uri.domain = uriStr.substr (strMatch[1].rm_so, strMatch[1].rm_eo-strMatch[1].rm_so);
		uri.port = (strMatch[3].rm_so == -1) ? "80" : uriStr.substr (strMatch[3].rm_so, strMatch[3].rm_eo-strMatch[3].rm_so);
		uri.path = ((strMatch[4].rm_so == -1) || (strMatch[4].rm_so == strMatch[4].rm_eo)) 
			? "/" : uriStr.substr (strMatch[4].rm_so, strMatch[4].rm_eo-strMatch[4].rm_so);
	}
	else {
		return 1;
	}
	regfree(&uriRe);
	return 0;
}


// spracovani argumentu prikazove radky
int GetArg (int argc, char *argv[], paramsInfo & params) {
	if (argc > 6 || argc == 1)
		return 1;
	int ch;
	int argOrder = 1;
	while ((ch = getopt(argc, argv, "lsmt")) != -1) {
		switch (ch) {
			case 'l':	if (params.lSize){ 
							return 1;
						}
						else
							params.lSize = argOrder;
						argOrder++;
						break;

			case 's':	if (params.sId)
							return 1;
						else
							params.sId = argOrder;
						argOrder++;
						break;

			case 'm':	if (params.mModified)
							return 1;
						else
							params.mModified = argOrder;
						argOrder++;
						break;

			case 't':	if (params.tType)
							return 1;
						else
							params.tType = argOrder;
						argOrder++;
						break;
			default:
						return 1;
		}
	}
	if ((optind != argc-1) || strncmp(argv[optind], "http:", 5)) 
		return 1;
	else return 0;
}

// hlavni funkce
int main (int argc, char *argv[]) {
	paramsInfo params = {};
	uriInfo uri = {};

	// kontrola argumentov
	if (GetArg(argc, argv, params) != 0) {
		std::cout << "pouziti: [-l],[-s],[-m],[-t] URL\n"
			<< "-l velikost objektu\n"
			<< "-s identifikace servru\n"
			<< "-m informace o posledni modifikaci objektu\n"
			<< "-t typ obsahu objektu\n";
		return WRONG_PARAMS;
	}

	// naparsovani adresy
	if (parseUri(argv[argc-1], uri) != 0) {
		std::cerr << "error: wrong address\n";
		return WRONG_ADDRESS;
	}

	// prve, a castokrat posledni pripojeni k serveru
	int sockNumber;
	int retInfo;
	std::string reply;
	if ((retInfo = connectAndWrite(reply, uri, sockNumber)) != 0) {
		return retInfo;;
	}

	int crlf; // znak konce radku, kvoli spracovaniu nestandardnej odpovede
	if (reply[reply.length()-2] == '\r')
		crlf = '\r';
	else 
		crlf = '\n';

	// osetreni pripadnych chyb
	int errCode;
	std::string errStr;
	for (int i = 0; i < MaxRedirections; i++) {
		errCode = handleReply(reply, errStr, crlf);
		if (i == 4 && errCode == HTTP_REDIRECT) {
			std::cerr << "error: too many redirections\n";
			return CONNECTION_ERROR;
		}
		if (errCode == HTTP_OK)
			break;
		else if (errCode == HTTP_ERROR) {
			std::cerr << "chyba: " << errStr << std::endl;
			return CONNECTION_ERROR;
		}
		else if (errCode == HTTP_REDIRECT) {
			reply.erase();
			if (parseUri(errStr.c_str(), uri) != 0) {
				return CONNECTION_ERROR;
			}
			if ((retInfo = connectAndWrite(reply, uri, sockNumber)) != 0) {
				return retInfo;
			}
		}
		else
			return INTERNAL_ERROR;
	}

	paramsInfo paramsUgly = {};
	if (memcmp(&params, &paramsUgly, sizeof(paramsInfo)) == 0) {
		std::cout << reply; // << std::endl;
	}
	else {
		std::string parsedReply[4];  // pole stringov urcene na vystup
		int sp, ep;
		if (params.lSize != 0) {
			if ((sp = reply.find("Content-Length")) != -1) {
				ep = reply.find_first_of (crlf, sp);
				parsedReply[params.lSize-1] = reply.substr(sp, ep-sp) + "\r\n";
			}
			else
				parsedReply[params.lSize-1] = "Content-Length: N/A\r\n";
		}
		if (params.sId != 0) {
			if ((sp = reply.find("Server")) != -1) {
				ep = reply.find_first_of (crlf, sp);
				parsedReply[params.sId-1] = reply.substr(sp, ep-sp) + "\r\n";
			}
			else
				parsedReply[params.sId-1] = "Server: N/A\r\n";
		}
		if (params.mModified != 0) {
			if ((sp = reply.find("Last-Modified")) != -1) {
				ep = reply.find_first_of (crlf, sp);
				parsedReply[params.mModified-1] = reply.substr(sp, ep-sp) + "\r\n";
			}
			else
				parsedReply[params.mModified-1] = "Last-Modified: N/A\r\n";
		}
		if (params.tType != 0) {
			if ((sp = reply.find("Content-Type")) != -1) {
				ep = reply.find_first_of (crlf, sp);
				parsedReply[params.tType-1] = reply.substr(sp, ep-sp) + "\r\n";
			}
			else
				parsedReply[params.tType-1] = "Content-Type: N/A\r\n";
		}
		for (int i = 0; i < 4; i++)
			std::cout << parsedReply[i];
	}
	close(sockNumber);
	exit (EXIT_SUCCESS);
}
