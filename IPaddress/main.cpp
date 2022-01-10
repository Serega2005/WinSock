﻿#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef UNICODE
#undef UNICODE
#endif

#include<iostream>
#include<WinSock2.h>
#include<iphlpapi.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

void main()
{
	setlocale(LC_ALL, "Russian");
	//1) Инициализация WinSock:
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//MAKEWORD(2,2) задаёт желаемую версию WinSock - 2.2
	if (iResult != 0)
	{
		cout << "Инициализация не удалась" << endl;
		return;
	}

	///////////////////////////////////////////////////////////
	
	//Определим IP-адреса интерфейсов:
	ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
	PIP_ADAPTER_ADDRESSES pAddress = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, outBufLen);//Выделяет память, как оператор view
	//IP_ADAPTER_ADDRESS - это список интерфейсов в системесо  всеми изх свойствами.
	
	//получаем список интерфейсов:
	//Первый вызов GetAdapterAddress нужен для определения размера буфера outBufLen
	if (GetAdaptersAddresses(AF_INET, NULL, NULL, pAddress, &outBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		HeapFree(GetProcessHeap(), 0, pAddress);//И для последующего выделения памяти
		pAddress = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, outBufLen);
	}
	if (pAddress == NULL)
	{
		cout << "память не выделена" << endl;
		return;
	}

	//Вторй вызов GetAdapterAddress() уже заполняет структуру pAddress всеми данными об интерфейсах.
	DWORD dwRetVal = GetAdaptersAddresses(AF_INET, NULL, NULL, pAddress, &outBufLen);
	
	if (dwRetVal == NO_ERROR)
	{
		for (PIP_ADAPTER_ADDRESSES pCurAddress = pAddress; pCurAddress; pCurAddress = pCurAddress->Next)
		{
			cout << "Interface Index:\t" << pCurAddress->IfIndex << endl;
			cout << "Interface name:\t" << pCurAddress->AdapterName << endl;
			wcout << "Description:\t" << pCurAddress->Description << endl;

			DWORD dwAddressStringLength = 256; //Размер строки, содержашей IP-адрес
			
			//Проход по списку Unicast-адресов текущего адаптера (pCurAddress):
			for (PIP_ADAPTER_UNICAST_ADDRESS_LH pUnicast = pCurAddress->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next)
			{
				//Выделяем память для строки, которой будет хранится полученный IP-адрес.
				LPSTR lpszAddressString = (LPSTR)HeapAlloc(GetProcessHeap(), 0, dwAddressStringLength);
				//Представляем адрес в точечно-десятичной нотации:
				WSAAddressToString(pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, NULL, lpszAddressString, &dwAddressStringLength);
				//буква W в имени функции, или типа данных означает WIDE_CHAR(w_char_t) - символы в кодировке Unicode
				
				/*cout << "\tIP address:\t" << lpszAddressString << endl;
				wcout << "/" << pUnicast->OnLinkPrefixLength << endl;*/
				printf("\tIP address:\t%s /%d\n", lpszAddressString, pUnicast->OnLinkPrefixLength);
				//%s - string
				//TODO
				ULONG lMask = 0;
				union
				{
					ULONG ul;
					BYTE b[4];
				}mask;
				if (ConvertLengthToIpv4Mask(pUnicast->OnLinkPrefixLength, &mask.ul) == NO_ERROR)
				{
					//cout << "\tSubnet mask: " << mask.b[0] << mask.b<< endl;
					printf("\tSubnet mask:\t %u.%u.%u.%u\n", mask.b[0], mask.b[1], mask.b[2], mask.b[3]);
				}

				HeapFree(GetProcessHeap(), 0, lpszAddressString);
			}

			if (pCurAddress->PhysicalAddressLength)
			{
				cout << "Phisical address (MAC): ";
				for (int i = 0; i < pCurAddress->PhysicalAddressLength; i++)
				{
					//cout.width(2); //Задаёт ширину поля для вывода
					//cout.fill('0');//Заполняет пустое место в выделенном поля заддным символом
					//cout << (hex) << (int)pCurAddress->PhysicalAddress[i];
					//if(i!=pCurAddress->PhysicalAddressLength-1)cout << "-";

					printf("%2.X", pCurAddress->PhysicalAddress[i]);
					if (i != pCurAddress->PhysicalAddressLength - 1)printf(":");
				}
			}

			cout << "\n------------------------------------------------------------\n";
		}
	}

	HeapFree(GetProcessHeap(), 0, pAddress);

	//Удаляем сокет:
	WSACleanup();
}