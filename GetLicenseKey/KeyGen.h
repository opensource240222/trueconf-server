#ifndef _ARM_KEYGEN_H_
#define _ARM_KEYGEN_H_

/*
Функция генерации лицензионного ключа
Значение ключа в виде строки возвращается в result.
В случае сбоя ключ пустой.
Функция тридобезопасная.
Параметры:
result [out] указатель на массив длинной 512 байт 
template_ [in] строка, содержащая encription template сертификата
hardwareID [in] число, соответсвующее ключу оборудования системы, для которой генерится сертификат
daysLifeTime [in] время жизни ключа в днях после первого запуска
otherinfo1-otherinfo4 [in] дополнительные поля, которые могут быть добавлены к клюучу
*/
#define ARM_MAX_KEY_BUFFER_LEN 513
void MakeLicenseKey(char* result,const char *template_, unsigned long hardwareID=0, unsigned short daysLifeTime=0, unsigned short otherinfo1=0, unsigned short otherinfo2=0, unsigned short otherinfo3=0, unsigned short otherinfo4=0);

#endif