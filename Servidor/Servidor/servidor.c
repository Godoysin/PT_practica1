/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: servidor.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Servidor de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <Winsock2.h>

#include "protocol.h"




main()
{

	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET sockfd,nuevosockfd;
	struct sockaddr_in  local_addr,remote_addr;
	char buffer_out[1024],buffer_in[1024], cmd[10], usr[10], pas[10];
	int err,tamanio;
	int fin=0, fin_conexion=0;
	int recibidos=0,enviados=0;
	int estado=0;

	/** INICIALIZACION DE BIBLIOTECA WINSOCK2 **
	 ** OJO!: SOLO WINDOWS                    **/
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0){
		return(-1);
	}
	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup() ;
		return(-2);
	}
	/** FIN INICIALIZACION DE BIBLIOTECA WINSOCK2 **/


	sockfd=socket(AF_INET,SOCK_STREAM,0); //Creación del socket- Crea un descriptor socket

	if(sockfd==INVALID_SOCKET)	{
		return(-3);
	}
	else {
		local_addr.sin_family		=AF_INET;			// Familia de protocolos de Internet
		local_addr.sin_port			=htons(TCP_SERVICE_PORT);	// Puerto del servidor
		local_addr.sin_addr.s_addr	=htonl(INADDR_ANY);	// Direccion IP del servidor Any cualquier disponible
													// Cambiar para que conincida con la del host
	}
	
	//Enlace el socket a la direccion local (IP y puerto)
	if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr))<0) //Bind- Asocia una dirección local con un socket
		return(-4);
	
	//Se prepara el socket para recibir conexiones y se establece el tamaño de cola de espera
	if(listen(sockfd,5)!=0) //Listen- crea una cola de espera para almacenar solicitudes de conexión
		return (-6);
	
	tamanio=sizeof(remote_addr);

	do
	{
		printf ("SERVIDOR> ESPERANDO NUEVA CONEXION DE TRANSPORTE\r\n"); //Llegamos aquí si se ha creado bien la conexión 
		//con el cliente.
		
		nuevosockfd=accept(sockfd,(struct sockaddr*)&remote_addr,&tamanio); //Accept- Espera una solicitud de conexión

		if(nuevosockfd==INVALID_SOCKET) {
			
			return(-5);
		}

		printf ("SERVIDOR> CLIENTE CONECTADO\r\nSERVIDOR [IP CLIENTE]> %s\r\nSERVIDOR [CLIENTE PUERTO TCP]>%d\r\n",
					inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

		//Mensaje de Bienvenida
		sprintf_s (buffer_out, sizeof(buffer_out), "%s Bienvenindo al servidor de ECO%s",OK,CRLF);
		
		enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0); //Send- envía un mensaje
		//DONE Comprobar error de envío
		if(enviados<=0)
		{
			if(enviados<0) //Caso que la operación haya fallado =-1
			{
				fin_conexion = 1;
				printf ("SERVIDOR> Se ha producido un error en el envío de datos\r\n");
			}
			else //Caso que la operación se haya cerrado =0
			{
				fin_conexion = 1;
				printf ("SERVIDOR> Se ha cerrado la conexion de aplicacion\r\n");
			}
		}
		else //Si no hay errores en el envío de datos, paso al estado correspondiente.
		{
			//Se reestablece el estado inicial
			estado = S_USER;
			fin_conexion = 0;
		}
		

		printf ("SERVIDOR> Esperando conexion de aplicacion\r\n");
		do
		{
			//Se espera un comando del cliente
			recibidos = recv(nuevosockfd,buffer_in,1023,0); //Recv- Recibir un mensaje
			//DONE Comprobar posible error de recepción
			if(recibidos<=0)
			{
				if(recibidos<0) //Caso de error
				{
					printf ("SERVIDOR> Se ha producido un error en la recepicon de datos\r\n");
					fin_conexion = 1;
					estado = S_QUIT;
				}
				else //Caso de cerrar conexion
				{
					printf ("SERVIDOR> Se ha cerrado la conexion de aplicacion\r\n");
					fin_conexion = 1;
					estado = S_QUIT;
				}
			}
			
			//Sin esta sentencia, en el caso de que recibidos sea negativo, se produce un error y se cuelga el servidor.
			if(recibidos>=0)
			{
				buffer_in[recibidos] = 0x00;
				printf ("SERVIDOR [bytes recibidos]> %d\r\nSERVIDOR [datos recibidos]>%s", recibidos, buffer_in);
			}
			
			switch (estado)
			{
				case S_USER:    /*****************************************/
					strncpy_s ( cmd, sizeof(cmd),buffer_in, 4);
					cmd[4]=0x00; //En C los arrays finalizan con el byte 0000 0000

					if ( strcmp(cmd,SC)==0 ) //Si el comando recibido es solicitud de conexion de aplicación.
					{
						sscanf_s (buffer_in,"USER %s\r\n",usr,sizeof(usr));
						
						// envia OK acepta todos los usuarios hasta que tenga la clave
						sprintf_s (buffer_out, sizeof(buffer_out), "%s%s", OK,CRLF);
						
						estado = S_PASS;
						printf ("SERVIDOR> Esperando clave\r\n");
					} else
					if ( strcmp(cmd,SD)==0 ) //Finalizacion de la conexion de aplicación.
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1;
					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
				break;

				case S_PASS: /******************************************************/

					
					strncpy_s ( cmd, sizeof(cmd), buffer_in, 4);
					cmd[4]=0x00; //En C los arrays finalizan con el byte 0000 0000

					if ( strcmp(cmd,PW)==0 ) //Si el comando recibido es password:
					{
						sscanf_s (buffer_in,"PASS %s\r\n",pas,sizeof(usr));

						if ( (strcmp(usr,USER)==0) && (strcmp(pas,PASSWORD)==0) ) //Si el usuario y la clave recibidos son correctos
						{
							//envía aceptación de la conexión de aplicación, nombre de usuario y
							//la direccion IP desde donde se ha conectado.
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s IP(%s)%s", OK, usr, inet_ntoa(remote_addr.sin_addr),CRLF);
							estado = S_DATA;
							printf ("SERVIDOR> Esperando comando\r\n");
						}
						else
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Autenticación errónea%s",ER,CRLF);
						}
					} else
					if ( strcmp(cmd,SD)==0 ) //Finalización de la conexión de aplicación.
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1;
					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
				break;

				case S_DATA: /***********************************************************/
					
					buffer_in[recibidos] = 0x00;
					
					strncpy_s(cmd,sizeof(cmd), buffer_in, 4);

					printf ("SERVIDOR [Comando]>%s\r\n",cmd);

					//Operacion de suma
					if(strcmp(cmd,SUM)==0)
					{
						printf("se lo ha tragado");
					}
					
					if ( strcmp(cmd,SD)==0 ) //Finalizacion de la conexion de aplicacion
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
						fin_conexion=1;
					}
					else if (strcmp(cmd,SD2)==0) //Finalizacion de la conexion de aplicacion y finaliza servidor
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Finalizando servidor%s", OK,CRLF);
						fin_conexion=1;
						fin=1;
					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
					break;

				default:
					break;
					
			} // switch

			enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0); //Send- Enviar un mensaje
			//DONE
			if(enviados<=0)
			{
				if(enviados<0) //Caso de error
				{
					printf("SERVIDOR> Hay un error en el envío de datos");
					fin_conexion=1;
				}
				else //Caso de fin de conexion
				{
					printf("SERVIDOR> Se ha cerrado la conexion");
					fin_conexion=1;
				}
			}


		} while (!fin_conexion);
		printf ("SERVIDOR> CERRANDO CONEXION DE TRANSPORTE\r\n");
		shutdown(nuevosockfd,SD_SEND); //Shutdown- Deshabilita la recepción y/o el envío de datos por el socket
		closesocket(nuevosockfd); //Close- cierra socket

	}while(!fin);

	printf ("SERVIDOR> CERRANDO SERVIDOR\r\n");

	return(0);
} 
