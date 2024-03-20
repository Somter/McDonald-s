#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream> 
using namespace std;

#define MAX_CLIENTS 30	
#define DEFAULT_BUFLEN 4096

#pragma comment(lib, "ws2_32.lib") // Winsock library
#pragma warning(disable:4996) // ��������� �������������� _WINSOCK_DEPRECATED_NO_WARNINGS

SOCKET server_socket;

vector<string> history;
vector <string> menu;	
int main() {
	system("title Server");

	menu.push_back("hamburger");	
	menu.push_back("nuggets");	
	menu.push_back("potato");	
	menu.push_back("cola");	

	puts("Start server... DONE.");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	// create a socket
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 2;
	}
	// puts("Create socket... DONE.");

	// prepare the sockaddr_in structure
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// bind socket
	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 3;
	}

	// ������� �������� ����������
	listen(server_socket, MAX_CLIENTS);

	// ������� � �������� ����������
	puts("Server is waiting for incoming connections...\nPlease, start one or more client-side app.");

	// ������ ������ ��������� ������, ��� ����� ������
	// ����� ������������ �������
	// fd means "file descriptors"
	fd_set readfds; // https://docs.microsoft.com/en-us/windows/win32/api/winsock/ns-winsock-fd_set
	SOCKET client_socket[MAX_CLIENTS] = {};

	while (true) {
		// �������� ����� fdset
		FD_ZERO(&readfds);

		// �������� ������� ����� � fdset
		FD_SET(server_socket, &readfds);

		// �������� �������� ������ � fdset
		for (int i = 0; i < MAX_CLIENTS; i++) 
		{
			SOCKET s = client_socket[i];
			if (s > 0) {
				FD_SET(s, &readfds);
			}
		}

		// ��������� ���������� �� ����� �� �������, ����-��� ����� NULL, ������� ����� ����������
		if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("select function call failed with error code : %d", WSAGetLastError());
			return 4;
		}

		// ���� ���-�� ��������� �� ������-������, �� ��� �������� ����������
		SOCKET new_socket; // ����� ���������� �����
		sockaddr_in address;
		int addrlen = sizeof(sockaddr_in);

		if (FD_ISSET(server_socket, &readfds)) {
			if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
				perror("accept function error");
				return 5;
			}

			for (int i = 0; i < history.size(); i++)
			{
				cout << history[i] << "\n";
				send(new_socket, history[i].c_str(), history[i].size(), 0);
			}
			
			// ������������� ��������� ������� � ������ ������ - ������������ � �������� �������� � ���������
			printf("New connection, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

			// �������� ����� ����� � ������ �������
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					printf("Adding to list of sockets at index %d\n", i);
					break;
				}
			}
		}

		// ���� �����-�� �� ���������� ������� ���������� ���-��
		int time = 0;
		int price = 0;	
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			SOCKET s = client_socket[i];
			// ���� ������ ������������ � ������� ������
			if (FD_ISSET(s, &readfds))
			{
				// �������� ��������� �������
				getpeername(s, (sockaddr*)&address, (int*)&addrlen);

				// ���������, ���� �� ��� �� ��������, � ����� ���������� �������� ���������
				// recv �� �������� ������� ���������� � ����� ������ (� �� ����� ��� printf %s ������������, ��� �� ����)

				char client_message[DEFAULT_BUFLEN];	

				int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
				client_message[client_message_length] = '\0';
				string check_exit = client_message;	
						
				for (size_t j = 0; j < menu.size(); ++j) {		
					if (check_exit.find(menu[j]) != -1) {		
						if (menu[j] == "hamburger") {
							time += 7;
							price += 5;
						}
						if (menu[j] == "nuggets") {
							time += 5;
							price += 3;
						}
						if (menu[j] == "potato") {
							time += 6;
							price += 3;	
						}
						if (menu[j] == "cola") {
							time += 3;	
							price += 1;	
						}
					}
				}

				size_t last_space_index = check_exit.find_last_of(' ');	
				string last_word = check_exit.substr(last_space_index + 1);		 
				int client_payment = stoi(last_word);	
				
				if(price > client_payment){
					int difference = price - client_payment;	
					string not_enough_to_pay = to_string(difference);	
					string erroror_messege = "You are missing $" + not_enough_to_pay + " to pay for your order";		
					send(client_socket[i], erroror_messege.c_str(), client_message_length, 0);	
				}

				if (check_exit == "off")
				{
					cout << "Client #" << i << " is off\n";
					client_socket[i] = 0;
				}	
				
				string temp = client_message;
				// temp += "\n";
				history.push_back(temp);

				
				/*for (int i = 0; i < MAX_CLIENTS; i++) {
					if (client_socket[i] != 0) {
						send(client_socket[i], client_message, client_message_length, 0);
					}
				}*/

			}
		}
	}

	WSACleanup();
}