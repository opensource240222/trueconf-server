#ifdef _WIN32	// not ported
#include "VS_TRStorageAdo.h"

#include "VS_DBStorage.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/clib/strcasecmp.h"
#include "../../../common/std/debuglog/VS_Debug.h"
#include "../../../common/std/VS_ProfileTools.h"

namespace VS_TRStorageAdoConstants {
	const char DB_INFO_HASH_PARAM[] = "c_info_hash";
	const char DB_CALLID_PARAM[]	= "c_call_id";
	const char DB_MAGNET_PARAM[]	= "c_magnet_link";
	const char DB_SIZE_PARAM[]		= "i_transfer_size";
	const char DB_LINK_PARAM[]		= "c_link";

	const char DB_HTTP_LINK_PARAM[] = "c_http_link";
	const char DB_FTO_ID_PARAM[]	= "fto_id";

	const char DB_FILENAME_PARAM[]	= "c_file_name";
	const char DB_FILESIZE_PARAM[]	= "i_file_size";

	const char DB_DONE_PARAM[]		= "i_done_cnt";
	const char DB_SPEED_IN_PARAM[]	= "i_speed_in";
	const char DB_SPEED_OUT_PARAM[] = "i_speed_out";
	const char DB_PEERS_PARAM[]		= "i_peers_cnt";

	const char DB_FORCE_PARAM[]		= "b_force";
	const char DB_DELETE_PARAM[]	= "b_delete_files";

	const char DB_TIME_PARAM[]		= "completion_time";
	const char DB_DAYS_PARAM[]		= "_days";
}

#define RETRIEVE_DBO(name, ret_val_err) \
	const VS_Pool::Item *item = nullptr;\
	VS_DBObjects *name = GetDBO(item);	\
	if (!name) return ret_val_err;		\
	AUTO_PROF \
	VS_SCOPE_EXIT{ m_dbo_pool->Release(item); };


#define DEBUG_CURRENT_MODULE VS_DM_DBSTOR

VS_TRStorageAdo::VS_TRStorageAdo(const std::shared_ptr<VS_Pool> &dbo_pool) : m_dbo_pool(dbo_pool) {
	const VS_Pool::Item *item = nullptr;
	VS_DBObjects *dbo = GetDBO(item);
	if (!dbo) {
		throw std::runtime_error("VS_DBObjects error");
	}
	m_dbo_pool->Release(item);
}

VS_DBObjects *VS_TRStorageAdo::GetDBO(const VS_Pool::Item* &item) {
	int n = 0;
	while ((item = m_dbo_pool->Get()) == NULL) {
		dprint1("POOL: no free db objects, sleeping\n");
		Sleep(500);
		n++;
		if (n >= 3 * 6 * 2) {
			return nullptr;
		}
	};

	return (VS_DBObjects*)item->m_data;
}

bool VS_TRStorageAdo::CreateUploadRecord(const std::string &initiator, const std::string &info_hash, const std::string &magnet, uint64_t size) {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false)

	try {
		ADODB::ParametersPtr p = dbo->create_file_transfer->GetParameters();
		p->GetItem(DB_INFO_HASH_PARAM)->Value	= info_hash.c_str();
		p->GetItem(DB_CALLID_PARAM)->Value		= initiator.c_str();
		p->GetItem(DB_MAGNET_PARAM)->Value		= magnet.c_str();
		p->GetItem(DB_SIZE_PARAM)->Value		= (LONGLONG)size;

		ADODB::_RecordsetPtr rs = dbo->create_file_transfer->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
		return rs != 0 && rs->GetState() != ADODB::adStateClosed && !rs->GetADOEOF();
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
}

bool VS_TRStorageAdo::CreateFileRecord(const std::string &info_hash, const std::string &file_name, uint64_t file_size) {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false)

	VS_WideStr file_name_wstr;
	file_name_wstr.AssignUTF8(file_name.c_str());

	try {
		ADODB::ParametersPtr p = dbo->add_file_to_transfer->GetParameters();
		p->GetItem(DB_INFO_HASH_PARAM)->Value	= info_hash.c_str();
		p->GetItem(DB_FILENAME_PARAM)->Value	= file_name_wstr.m_str;
		p->GetItem(DB_FILESIZE_PARAM)->Value	= (LONGLONG)file_size;

		return 0 != dbo->add_file_to_transfer->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
}

bool VS_TRStorageAdo::UpdateUploadRecord(const std::string &info_hash, uint64_t downloaded,
										 unsigned dw_speed, unsigned up_speed, unsigned num_peers) {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false)

	try {
		ADODB::ParametersPtr p = dbo->add_file_transfer_stat->GetParameters();
		p->GetItem(DB_INFO_HASH_PARAM)->Value	= info_hash.c_str();
		p->GetItem(DB_DONE_PARAM)->Value		= (LONGLONG)downloaded;
		p->GetItem(DB_SPEED_IN_PARAM)->Value	= (LONG)dw_speed;
		p->GetItem(DB_SPEED_OUT_PARAM)->Value	= (LONG)up_speed;
		p->GetItem(DB_PEERS_PARAM)->Value		= (LONG)num_peers;

		return 0 != dbo->add_file_transfer_stat->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
}

bool VS_TRStorageAdo::UpdateFileRecord(const std::string &info_hash, const std::string &file_name,
									   uint64_t downloaded, unsigned dw_speed, unsigned up_speed, unsigned num_peers) {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false)

	VS_WideStr file_name_wstr;
	file_name_wstr.AssignUTF8(file_name.c_str());

	try {
		ADODB::ParametersPtr p = dbo->add_file_transfer_files_stat->GetParameters();
		p->GetItem(DB_INFO_HASH_PARAM)->Value	= info_hash.c_str();
		p->GetItem(DB_FILENAME_PARAM)->Value	= file_name_wstr.m_str;
		p->GetItem(DB_DONE_PARAM)->Value		= (LONGLONG)downloaded;
		p->GetItem(DB_SPEED_IN_PARAM)->Value	= (LONG)dw_speed;
		p->GetItem(DB_SPEED_OUT_PARAM)->Value	= (LONG)up_speed;
		p->GetItem(DB_PEERS_PARAM)->Value		= (LONG)num_peers;

		return 0 != dbo->add_file_transfer_files_stat->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
}

bool VS_TRStorageAdo::DeleteUploadRecord(const std::string &info_hash) {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false)

	try {
		ADODB::ParametersPtr p = dbo->delete_file_transfer->GetParameters();
		p->GetItem(DB_INFO_HASH_PARAM)->Value	= info_hash.c_str();
		p->GetItem(DB_FORCE_PARAM)->Value		= (BOOL)false;

		return 0 != dbo->delete_file_transfer->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
}

bool VS_TRStorageAdo::MarkDeleteUpload(const std::string &info_hash) {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false)

	try {
		ADODB::ParametersPtr p = dbo->set_file_transfer_delete_files->GetParameters();
		p->GetItem(DB_INFO_HASH_PARAM)->Value	= info_hash.c_str();
		p->GetItem(DB_DELETE_PARAM)->Value		= (BOOL)true;

		return 0 != dbo->set_file_transfer_delete_files->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
}

bool VS_TRStorageAdo::MarkDeleteOldUploads(unsigned lifetime_days)
{
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, false);

	try {
		ADODB::ParametersPtr p = dbo->set_file_transfer_delete_files_by_days->GetParameters();
		p->GetItem(DB_DAYS_PARAM)->Value = (LONG)lifetime_days;

		return 0 != dbo->set_file_transfer_delete_files_by_days->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
	}
	catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return false;
	}
	return false;
}

std::vector<VS_TRStorageInterface::UploadRecord> VS_TRStorageAdo::QueryUploadsActive() {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, {})

	try {
		ADODB::ParametersPtr p = dbo->get_file_transfer->GetParameters();
		p->GetItem(DB_DELETE_PARAM)->Value = (BOOL)false;

		ADODB::_RecordsetPtr rs = 0;
		try {
			rs = dbo->get_file_transfer->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
		} catch (_com_error err) {
			ADODB::_ConnectionPtr cur_db = dbo->get_file_transfer->ActiveConnection;
			ADODB::ErrorPtr ae = cur_db->Errors->GetItem(0);
			auto sql_state = (LPCSTR)ae->SQLState;
			if (sql_state && strcasecmp("S1000", sql_state) != 0) {
				throw err;
			}
		}

		if (rs == 0 || rs->GetState() == ADODB::adStateClosed || rs->GetADOEOF()) {
			return {};
		}

		std::vector<UploadRecord> ret;
		for (; !rs->GetADOEOF(); rs->MoveNext()) {
			ADODB::FieldsPtr f = rs->Fields;

			const auto &vt_magnet_link = f->GetItem("magnet_link")->Value;
			std::string magnet_link = vt_magnet_link.vt == VT_NULL ? "" : (const char*)(_bstr_t)vt_magnet_link;

			const auto &vt_transfer_size = f->GetItem("transfer_size")->Value;
			uint64_t transfer_size = vt_transfer_size.vt == VT_NULL ? 0 : (uint64_t)vt_transfer_size;

			const auto &vt_completion_time = f->GetItem("completion_time")->Value;
			int64_t completion_time = vt_completion_time.vt == VT_NULL ? 0 : (int64_t)vt_completion_time;

			ret.emplace_back((_bstr_t)f->GetItem("info_hash")->Value, (_bstr_t)f->GetItem("call_id")->Value,
							 magnet_link.c_str(), transfer_size, completion_time);
		}
		return ret;
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return {};
	}
}

std::vector<VS_TRStorageInterface::UploadRecord> VS_TRStorageAdo::QueryUploadsToDelete() {
	using namespace VS_TRStorageAdoConstants;
	RETRIEVE_DBO(dbo, {})

	try {
		ADODB::ParametersPtr p = dbo->get_file_transfer->GetParameters();
		p->GetItem(DB_DELETE_PARAM)->Value = (BOOL)true;

		ADODB::_RecordsetPtr rs = 0;
		try {
			rs = dbo->get_file_transfer->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
		} catch (_com_error err) {
			ADODB::_ConnectionPtr cur_db = dbo->get_file_transfer->ActiveConnection;
			ADODB::ErrorPtr ae = cur_db->Errors->GetItem(0);
			auto sql_state = (LPCSTR)ae->SQLState;
			if (sql_state && strcasecmp("S1000", sql_state) != 0) {
				throw err;
			}
		}

		if (rs == 0 || rs->State == ADODB::adStateClosed || rs->GetADOEOF()) {
			return {};
		}

		std::vector<UploadRecord> ret;
		for (; !rs->GetADOEOF(); rs->MoveNext()) {
			ADODB::FieldsPtr f = rs->Fields;

			const auto &vt_magnet_link = f->GetItem("magnet_link")->Value;
			std::string magnet_link = vt_magnet_link.vt == VT_NULL ? "" : (const char*)(_bstr_t)vt_magnet_link;

			const auto &vt_transfer_size = f->GetItem("transfer_size")->Value;
			uint64_t transfer_size = vt_transfer_size.vt == VT_NULL ? 0 : (uint64_t)vt_transfer_size;

			const auto &vt_completion_time = f->GetItem("completion_time")->Value;
			int64_t completion_time = vt_completion_time.vt == VT_NULL ? 0 : (int64_t)vt_completion_time;

			ret.emplace_back((_bstr_t)f->GetItem("info_hash")->Value, (_bstr_t)f->GetItem("call_id")->Value,
							 magnet_link.c_str(), transfer_size, completion_time);
		}
		return ret;
	} catch (_com_error err) {
		dbo->ProcessCOMError(err, dbo->db, nullptr, nullptr);
		return {};
	}
}

#undef DEBUG_CURRENT_MODULE
#undef RETRIEVE_DBO
#endif