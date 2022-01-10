#define  WIN32_LEAN_AND_MEAN
#include"ip_checksum.h"

extern USHORT ip_checksum(USHORT* buffer, int size)
{
	//USHORT - unsigned short (2 Bytes)
	unsigned int cksum = 0;
	
	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size)cksum += *(UCHAR*)buffer;//���� ������ ����� ��������, ���������� ���������� ���� � ����������� �����.

	cksum = (cksum >> 16) + (cksum & 0xFFFF);
	cksum += (cksum >> 16);

	return (USHORT)(~cksum); //���������� �������� ����������� �����
}