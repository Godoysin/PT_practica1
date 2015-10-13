/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16];
	char default_ip[16]="127.0.0.1";
	
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	do{ 
		sockfd=socket(AF_INET,SOCK_STREAM,0); //socket- Crea un descriptor socket

		if(sockfd==INVALID_SOCKET)
		{
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

		
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);

			if(strcmp(ipdest,"")==0) //Si no introduces una IP, coge una por defecto
				strcpy(ipdest,default_ip);


			server_in.sin_family=AF_INET;
			server_in.sin_port=htons(TCP_SERVICE_PORT);
			server_in.sin_addr.s_addr=inet_addr(ipdest);
			
			estado=S_HELO;
		
			//Establece la conexion de transporte.
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0) //Connect- Inicia conexión con conector remoto
			//Si se conecta, te dice que se ha establecido conexión, si no, no.
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
		
				//Inicio de la máquina de estados.
				do{
					switch(estado)
					{
					case S_HELO:
						//Se recibe el mensaje de bienvenida.
						break;
					case S_USER:
						//Establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets(input);
						if(strlen(input)==0) //Si la longitud de la cadena de caracteres es cero:
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //Finaliza la conexion de manera de que
							//el servidor espere una nueva conexión.
							estado=S_QUIT;
						}
						else

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF); //Como si se ha introducido un usuario
						//supone que es correcto y pasa a introducir la clave. (Envía el usuario con input).
						break;
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets(input);
						if(strlen(input)==0) //Si la longitud de la cadena de caracteres es cero:
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //Finaliza la conexion de manera de que
							//el servidor espere una nueva conexión.
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF); //Coge la clave, si el usuario y la
							//clave son correctas, pasa al siguiente estado, si alguno de los dos u ambos no están autorizados,
							//dice que la autorización es erronea y hay que salir. (Envía la clave con input).
						break;
					case S_DATA:
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets(input);
						if(strlen(input)==0) //Si la longitud de la cadena de caracteres es cero:
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //Finaliza la conexion de manera de que
							//el servidor espere una nueva conexión.
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF); //Envía los datos con input.
						break;
				 
				
					}
					//Envio
					if(estado!=S_HELO)
					{
						//Ejercicio: Comprobar el estado de envio
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0); //Send- envía un mensaje
						if(enviados<=0)
						{
							if(enviados<0)
							{
								printf("CLIENTE> Error en la recepcion de datos\r\n");
								estado=S_QUIT;
							}
							else
							{
							printf("CLIENTE> Conexion con el servidor cerrada\r\n");
							estado=S_QUIT;
							}
					}
					}
					
						

					//Recibo
					recibidos=recv(sockfd,buffer_in,512,0); //Recv- Recibir un mensaje
					if(recibidos<=0)
					{
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepcion de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexion con el servidor cerrada\r\n");
							estado=S_QUIT;
						
					
						}
					}
					else
					{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
							estado++;  
					}

				}while(estado!=S_QUIT);
				
	
		
			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			//Fin de la conexion de transporte
			closesocket(sockfd);  //Close- cierra socket
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	
	
	return(0);

}
