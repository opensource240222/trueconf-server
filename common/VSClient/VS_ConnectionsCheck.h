#pragma once
/// \brief Найти валидные соединения из тех, что описаны в Registry для endpoint,
/// и упорядочить их так, чтобы валидные соединения были в начале списка.
void VS_ConnectionsCheckFast(const char *endpoint, const unsigned long mills, unsigned long *srv_response_time = 0);
