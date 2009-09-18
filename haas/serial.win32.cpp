bool Serial::connect()
{
  if (mConnected)
  {
    printf("You are already connected, please disconnect first\n");
    return false;
  }
  
  COMMTIMEOUTS cto;
  DCB dcb;
  char parity, stop_bits;
  int speed;
	 	 
  if (mFd != INVALID_HANDLE_VALUE && !CloseHandle(mFd)){
    printf("Can't close comm port");
    return false;
  }
  
  mFd = CreateFileA(mDevice,
		    GENERIC_READ|GENERIC_WRITE,
		    0,
		    NULL,
		    OPEN_EXISTING,
		    0,
		    NULL);
	 
  if(mFd == INVALID_HANDLE_VALUE) {
    char port[64];
    strcpy(port, "\\\\.\\");
    strcat(port, mDevice);
    mFd = CreateFileA(port,
		      GENERIC_READ|GENERIC_WRITE,
		      0,
		      NULL,
		      OPEN_EXISTING,
		      0,
		      NULL);
		 
    if(mFd == INVALID_HANDLE_VALUE) {
      printf("CreateFile Failed \n");
      return false;
    } 
  }
	 
  if(!SetupComm(
       mFd,	// handle of communications device
       1024,	// size of input buffer
       1024	// size of output buffer
       )
    ){
    printf("SetupComm failed \n");
    CloseHandle(mFd);
    return false;
  }
	 
  // verify baudrate and pairty
	 
  // set DCB
  switch (mBaud) {
  case 110:
    speed = CBR_110;
    break;
  case 300:
    speed = CBR_300;
    break;
  case 600:
    speed = CBR_600;
    break;
  case 1200:
    speed = CBR_1200;
    break;
  case 2400:
    speed = CBR_2400;
    break;
  case 4800:
    speed = CBR_4800;
    break;
  case 9600: 
    speed = CBR_9600;
    break;
  case 19200:
    speed = CBR_19200;
    break;
  case 38400:
    speed = CBR_38400;
    break;
  case 57600:
    speed = CBR_57600;
    break;
  case 115200:
    speed = CBR_115200;
    break;
  default:
    speed = CBR_9600;
    printf("WARNING Unknown baud rate %d for %s (B9600 used)\n",
	   mBaud, mDevice);
  }

  if (strncmp(mParity, "none", 4) == 0) {
    parity = NOPARITY;
  } else if (strncmp(mParity, "even", 4) == 0) {
    parity = EVENPARITY;
  } else {
    /* odd */
    parity = ODDPARITY;
  }

  /* Stop bit (1 or 2) */
  if (mStopBit == 1)
    stop_bits = ONESTOPBIT;
  else /* 2 */
    stop_bits = TWOSTOPBITS;

  memset(&dcb,0,sizeof(dcb));
  dcb.DCBlength = sizeof(dcb);
  dcb.BaudRate = speed;
  dcb.fBinary = TRUE;
  dcb.StopBits = stop_bits;
  dcb.Parity = parity;
  dcb.ByteSize = mDataBit;
	 

  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fOutxCtsFlow = FALSE ;	 
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
	 
  // setup software flow control
  dcb.fInX = dcb.fOutX = 0;
		 
  memset(&cto, 0, sizeof(cto));
  cto.ReadIntervalTimeout = 100;
  cto.ReadTotalTimeoutMultiplier = 1;
  cto.ReadTotalTimeoutConstant = 1000;
  cto.WriteTotalTimeoutMultiplier = cto.ReadIntervalTimeout;
  cto.WriteTotalTimeoutConstant = 1000;	 

  if(!SetCommTimeouts(mFd, &cto)){
    printf("SetTimeouts failed ");
	CloseHandle(mFd);
    return false;
  }	 
	 
  if(!SetCommState(mFd, &dcb)){
    printf("SetCommState failed \n");
	CloseHandle(mFd);
    return false;
  }
	 
  mConnected = true;
  
  return true;
}

bool Serial::disconnect()
{
  if (mConnected)
  {
    CloseHandle(mFd);
    mConnected = false;
	mFd = INVALID_HANDLE_VALUE;
    return true;
  }
  else
  {
    return false;
  }
}

int Serial::read(char *aBuffer, int aLen)
{
  int ret;
  DWORD bytes_read;
  if(ReadFile(mFd, aBuffer, aLen, &bytes_read, NULL) ) 
  {
    ret = bytes_read;
  }
  else
  {
    DWORD errors;
    COMSTAT status;
    
    ClearCommError(mFd, &errors, &status);
    ret = -1;
  }

  return ret;
}

int Serial::write(const char *aBuffer, int aLen)
{
  DWORD bytes_written;
  int ret;
  if(WriteFile(mFd, aBuffer, aLen, &bytes_written, NULL) ) 
  {
    ret = bytes_written;
  }
  else
  {
    DWORD errors;
    COMSTAT status;
    ClearCommError(mFd,&errors,&status);
    ret = -1;
  }
  return ret;
}

