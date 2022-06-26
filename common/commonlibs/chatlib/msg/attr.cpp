#include "attr.h"
namespace chat
{
namespace attr
{
const char MESSAGE_TYPE_paramName[] = "MessageType";
const char CHAT_ID_paramName[] = "ChatId";
const char TIMESTAMP_paramName[] = "timestamp";
const char MESSAGE_ID_paramName[] = "MsgId";
const char FROM_paramName[] = "from";
const char FROM_INSTANCE_paramName[] = "fromInstance";
const char SENDER_TYPE_paramName[] = "SenderType";
const char RESPONSE_CODE_paramName[] = "RespCode";
const char MESSAGE_CONTENT_paramName[] = "content";
const char GLOBAL_CONTEXT_paramName[] = "global_context";
const char PERSONAL_CONTEXTS_paramName[] = "personal_contexts";
const char BLOB_paramName[] = "blob";
const char PREVIOUS_MESSAGE_ID_paramName[] = "previousId";
const char ORIGINAL_MESSAGE_ID_paramName[] = "originalId";

const char TAIL_HASH_paramName[] = "TailHash";
const char TAIL_LENGTH_paramName[] = "TailLength";
const char MASTER_WEIGHT_paramName[] = "master_weight";

const char BUCKET_paramName[] = "bucket";
const char BUCKETSYNC_METHOD_paramName[] = "sync_meth";
const char SYNC_ID_paramName[] = "sync_id";
const char BUCKET_HASH_paramName[] = "bucke_hash";

const char SYNC_BUCKET_START[] = "sync_start";
const char SYNC_BUCKET_LIST_REQ[] = "bucket_list_req";
const char SYNC_BUCKET_LIST_RESP[] = "bucket_list_resp";
const char GET_DIFF_MSG_REQ[] = "diff_msg";
const char GET_DIFF_MSG_RESP[] = "diff_resp";

const char SYNC_CHECK_REQ[] = "check_sync_req";
const char SYNC_CHECK_RESP[] = "check_sync_resp";

const char RESET_SYNC_REQ[] = "reset_sync_req";
const char INSERT_MSG[] = "insert_msg";
const char INSERTED_MSG_ID_paramName[] = "ins_msg_id";

const char DST_ENDPOINT_paramName[] = "dst_endpoint";
const char DST_CALLID_paramName[] = "dst_callid";
const char SRC_ENDPOINT_paramName[] = "src_ep";

const char REQ_ID_paramName[] = "reqid";
const char ERROR_CODE_paramName[] = "error_code";
const char RETRANSMITS_paramName[] = "retransmits";
}
}