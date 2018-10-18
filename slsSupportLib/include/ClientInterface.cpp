#include "ClientInterface.h"


ClientInterface::ClientInterface(MySocketTCP *socket, int n, std::string t):
mySocket(socket),
index(n),
type(t){}

ClientInterface::~ClientInterface(){}


void ClientInterface::SetSocket(MySocketTCP *socket) {
	mySocket = socket;
}


int ClientInterface::Client_GetMesage(char* mess) {
	bool created = false;
	int ret = OK;
	if (!mess){
		created = true;
		mess = new char[MAX_STR_LENGTH];
		memset(mess, 0, MAX_STR_LENGTH);
	}
	mySocket->ReceiveDataOnly(mess,MAX_STR_LENGTH);
	cprintf(RED, "%s %d returned error: %s", type.c_str(), index, mess);
	if(strstr(mess,"Unrecognized Function")!=NULL)
		ret = FAIL;

	if (created)
		delete [] mess;
	return ret;
}


int ClientInterface::Client_Send(int fnum,
		void* args, int sizeOfArgs,
		void* retval, int sizeOfRetval,
		char* mess) {

	mySocket->SendDataOnly(&fnum,sizeof(fnum));
	mySocket->SendDataOnly(args, sizeOfArgs);

	int ret = FAIL;
	mySocket->ReceiveDataOnly(&ret,sizeof(ret));
	if (ret == FAIL) {
		if (Client_GetMesage(mess) == FAIL)
			return FAIL;
	}
	mySocket->ReceiveDataOnly(retval, sizeOfRetval);

	return ret;
}


void ClientInterface::Server_SendResult(bool update, int ret,
		void* retval, int retvalSize, char* mess) {

	// update if different clients
	if (update && ret == OK && mySocket->differentClients)
		ret = FORCE_UPDATE;

	// send success of operation
	mySocket->SendDataOnly(&ret,sizeof(ret));
	if(ret == FAIL) {
		// send error message
		if (mess)
			mySocket->SendDataOnly(mess, MAX_STR_LENGTH);
		// debugging feature. should not happen.
		else
			FILE_LOG(logERROR) << "No error message provided for this failure. Will mess up TCP\n";
	}
	// send return value
	mySocket->SendDataOnly(retval, retvalSize);
}


int ClientInterface::Server_ReceiveArg(int& ret, char* mess, void* arg, int sizeofArg, bool checkbase, void* base) {
	// client socket crash, cannot receive arguments
	if (sizeofArg && mySocket->ReceiveDataOnly(arg, sizeofArg) < 0)
		return Server_SocketCrash();

	ret = OK;
	// check if server object created
	if (checkbase && base == NULL)
		Server_NullObjectError(ret, mess);

	// no crash
	return OK;
}


int ClientInterface::Server_VerifyLock(int& ret, char* mess, int lockstatus) {
	// server locked
	if (mySocket->differentClients && lockstatus)
		return Server_LockedError(ret, mess);
	return ret;
}


int ClientInterface::Server_VerifyLockAndIdle(int& ret, char* mess, int lockstatus, slsDetectorDefs::runStatus status, int fnum) {
	// server locked
	if (mySocket->differentClients && lockstatus)
		return Server_LockedError(ret, mess);

	// server not idle for this command
	if (status != slsDetectorDefs::IDLE)
		return Server_NotIdleError(ret, mess, fnum);
	return ret;
}


void ClientInterface::Server_NullObjectError(int& ret, char* mess) {
	ret=FAIL;
	strcpy(mess,"Receiver not set up. Please use rx_hostname first.\n");
	FILE_LOG(logERROR) << mess;
}


int ClientInterface::Server_SocketCrash() {
	FILE_LOG(logERROR) << "Reading from socket failed. Possible socket crash";
	return FAIL;
}


int ClientInterface::Server_LockedError(int& ret, char* mess) {
	ret = FAIL;
	sprintf(mess,"Receiver locked by %s\n", mySocket->lastClientIP);
	FILE_LOG(logERROR) << mess;
	return ret;
}


int ClientInterface::Server_NotIdleError(int& ret, char* mess, int fnum) {
	ret = FAIL;
	sprintf(mess,"Can not execute %s when receiver is not idle\n",
			getFunctionNameFromEnum((enum detFuncs)fnum));
	FILE_LOG(logERROR) << mess;
	return ret;
}




