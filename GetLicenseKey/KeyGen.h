#ifndef _ARM_KEYGEN_H_
#define _ARM_KEYGEN_H_

/*
������� ��������� ������������� �����
�������� ����� � ���� ������ ������������ � result.
� ������ ���� ���� ������.
������� ���������������.
���������:
result [out] ��������� �� ������ ������� 512 ���� 
template_ [in] ������, ���������� encription template �����������
hardwareID [in] �����, �������������� ����� ������������ �������, ��� ������� ��������� ����������
daysLifeTime [in] ����� ����� ����� � ���� ����� ������� �������
otherinfo1-otherinfo4 [in] �������������� ����, ������� ����� ���� ��������� � ������
*/
#define ARM_MAX_KEY_BUFFER_LEN 513
void MakeLicenseKey(char* result,const char *template_, unsigned long hardwareID=0, unsigned short daysLifeTime=0, unsigned short otherinfo1=0, unsigned short otherinfo2=0, unsigned short otherinfo3=0, unsigned short otherinfo4=0);

#endif