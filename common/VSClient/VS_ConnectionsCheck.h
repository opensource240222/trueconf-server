#pragma once
/// \brief ����� �������� ���������� �� ���, ��� ������� � Registry ��� endpoint,
/// � ����������� �� ���, ����� �������� ���������� ���� � ������ ������.
void VS_ConnectionsCheckFast(const char *endpoint, const unsigned long mills, unsigned long *srv_response_time = 0);
