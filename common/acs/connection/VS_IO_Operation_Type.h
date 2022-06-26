#pragma once

enum VS_IO_Operation_Type
{
	e_operation_none,
	e_operation_acceptex,
	e_operation_read,
	e_operation_write,
	e_operation_udp_read,
	e_operation_udp_write,
	e_operation_connect_ok,
	e_operation_connect_failed,
	e_operation_close_one,	///
	e_operation_close_both,	///

	e_operation_close_read,
	e_operation_close_write,

	e_operation_1 //reserved
};