#pragma once

#include <queue>
#include <utility> // pair
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <openssl/ossl_typ.h>
#include <openssl/bio.h>

#include "VS_ConnectionTCP.h"
#include "VS_IOHandler.h"
#include "VS_ConnectionOv.h"

#include "std/cpplib/function.h"
#include "std/cpplib/VS_MessageHandler.h"
#include "std/cpplib/VS_TimeoutHandler.h"

#include "SecureLib/OpenSSLCompat/tc_ssl.h"
#include "net/tls/TlsCtx.h"



class VS_ConnectionTLS : public VS_ConnectionTCP, public VS_IOHandler
{
public:
	// verify modes
	enum {
		VERIFY_NONE,
		VERIFY_PEER
	};

	typedef boost::function<void(bool,VS_ConnectionTLS*,const void*,const unsigned long)> callback_t;
private:
	/*
	Anyone who will send Client Hello can send any garbage into socket after and we will try
	to process this data as handshake forever. This is a potential vulnerability.
	We need to limit number of messages we will process as handshake with some sane number.
	*/
	static const int MAX_TRIES = 300; // to prevent server hangup
	static const size_t TLS_IOBUF_LEN = 1024;
	static const size_t BUFSZ = 1024;
	// Handshaker
	class Handshaker
	{
	public:
		static const int MESSAGE_TYPE = 42; // the answer to life the universe and everything
		Handshaker(VS_ConnectionTLS *master, bool is_server = true);


		bool Do(const void *data, size_t size, unsigned long& mills);
	private:
		enum HandshakeStatus
		{
			hs_success,
			hs_ongoing,
			hs_failure,
			hs_disconnect
		};

		bool DoHandshake(const void *data, size_t size, unsigned long& mills);
		HandshakeStatus TryHandshake();
		int SendAll(BIO *b, unsigned long& mills);
		int ReceiveSome(BIO *b, unsigned long& mills);
	private:
		SSL *m_ssl;
		SSL_CTX *m_ctx;
		bool is_cb_called;
		bool m_is_srv;
		bool m_was_handshake;
		BIO *m_in, *m_out;
		unsigned char m_buf[BUFSZ];
		VS_ConnectionTLS *m_master;
	};

	class AsyncHandshaker : public VS_IOHandler, public VS_MessageHandler, public VS_TimeoutHandler, public boost::enable_shared_from_this<AsyncHandshaker>
	{
	public:
		// use only this method to create new handshaker object
		static boost::shared_ptr<AsyncHandshaker> Make(VS_ConnectionTLS *master, bool is_server = true)
		{
			boost::shared_ptr<AsyncHandshaker> ahs(new AsyncHandshaker(master, is_server));
			return ahs;
		}
	protected:
		AsyncHandshaker(VS_ConnectionTLS *master, bool is_server = true);
	public:
		virtual ~AsyncHandshaker();
		bool Do(const void *data, unsigned long len, unsigned long& mills);
		virtual void Handle(const unsigned long transferred, const VS_Overlapped *ov) override;
		virtual void HandleMessage(const boost::shared_ptr<VS_MessageData> &message) override;
		virtual void HandleError(const unsigned long err, const VS_Overlapped *ov) override;

		virtual void Timeout() override;
	private:
		enum {
			OV_READ_COMPLETE = 1,
			OV_WRITE_COMPLETE
		};

		enum {
			MSG_START = 1,
			MSG_DONE
		};
	private:
		enum HandshakeResult
		{
			hr_success,
			hr_ongoing,
			hr_error
		};

		void invokecb(bool status);
		HandshakeResult TryHandshake();
		void Done(void *data, unsigned long len);

		bool EnqueueWrite(void *data, unsigned long len);
		bool EnqueueRead(void *data, unsigned long len);
	private:
		boost::shared_ptr<VS_WorkThread> m_thread;
		SSL *m_ssl;
		SSL_CTX *m_ctx;
		bool is_cb_called;
		bool m_is_srv;
		bool m_was_handshake;
		BIO *m_in, *m_out;
		unsigned char m_wbuf[BUFSZ];
		unsigned char m_rbuf[BUFSZ];
		VS_ConnectionTLS *m_master;
		boost::shared_ptr<AsyncHandshaker> self;
		int m_try_number;
		std::chrono::steady_clock::time_point m_handshake_timeout_time;
	};

	struct WriteRequest {
		// initialize fields properly
		WriteRequest()
			: written(0), real_size(0), encoded_size(0)
		{}

		std::vector<uint8_t> data; // enciphered data buffer
		size_t written;      // how much enciphered data we have written
		size_t real_size;    // size of plain, raw data
		size_t encoded_size; // size of enciphered data
	};

	// ReadRequest class - occasionally the hard one...
	class ReadRequest {
	public:
		// for Read()
		static boost::shared_ptr<ReadRequest> Make(void *readbuf, size_t readbuf_size)
		{
			boost::shared_ptr<ReadRequest> ptr(new ReadRequest(readbuf, readbuf_size));

			return ptr;
		}

		// for RRead() - it will allocate read buffer
		static boost::shared_ptr<ReadRequest> Make(size_t readbuf_size)
		{
			boost::shared_ptr<ReadRequest> ptr(new ReadRequest(NULL, readbuf_size));

			return ptr;
		}

		~ReadRequest();
	protected:
		ReadRequest(void *rb, size_t rb_sz);
	public:
		bool IsAllocated(void);
		// append decoded data into internal buffer
		void AppendData(const void *data, const size_t size);
		// get decoded data from internal buffer
		void *GetData(void);
		// get total size of decoded data in user supplied buffer
		size_t GetTotalDataSize(void);
		// get size of data in internal buffer for decoded data
		size_t GetDataSize(void);
		// get size of decoded data in internal buffer
		// get size of user supplied read buffer
		size_t GetReadBufSize(void);
		// size of use supplied read buffer
		size_t GetReadBufSizeLeft(void);
		// accquire read buffer from previous read request (for aprtial read support)
		void AccquireReadBuffer(ReadRequest &req);
		// set internal buffers to NULL to prevent in from free()ing
		void ResetReadBuffer(void);
		// get pointer to read buffer from last read request
		void *GetReadBuf(void);
		// get pointer to reda buffer from the very first read request
		void *GetReadBufInitial(void);
		// get pointer to the free part of the red buffer
		void *GetReadBufLeft(void);

		// get pointer to the temporary buffer for raw encoded data
		void *GetTemporaryBuffer(void);
		//  get pointer to the temporary buffer for raw encoded data using given offset
		void *GetTemporaryBuffer(size_t offset);
		size_t GetTemporaryBufferOffset(void);
		size_t GetTemporaryBufferSize(void);
		void SetTemporaryBufferOffset(unsigned int offset);
		void IncreaseTemporaryBufferOffset(const int count);

		inline bool WasReadCalled() {return m_read_called;}
		inline void SetReadCalled(bool value) {m_read_called = value;}
	private:
		void SetAlreadyRead(size_t read);
		size_t GetAlreadyRead(void);
	private:
		void                 *m_freebuf; // malloc()ed buffer
		std::vector<uint8_t>  m_tmpbuf; // temporary I/O buffer
		size_t                m_tmpbuf_offset;
		std::vector<uint8_t>  m_ready_data; // deciphered data
		void                 *m_readbuf; // where to put decoded data
		size_t                m_readbuf_size; // size of buffer for decoded data
		size_t                m_already_read; // how much data we have read already
		bool                  m_read_called;
	};

	friend class AsyncHandshaker;
	friend class Handshaker;

	boost::function<void(bool,VS_ConnectionTLS*,const void*,const unsigned long)> m_cb;
	boost::shared_ptr<VS_WorkThread>		m_thread;
	boost::shared_ptr<VS_WorkThread>		m_io_thread; // we will keep here shared pointer before we set io thread in a Right Way
	boost::shared_ptr<VS_WorkThread>		m_delegate_thread;
	VS_IOHandler                           *m_delegate_write_handler,
		*m_delegate_read_handler;
	VS_Overlapped							m_delegate_write_ov,
		                                    m_delegate_read_ov;
	boost::shared_ptr<Handshaker>           m_hs; // handshaker
	BIO                                    *m_bio_in;
	BIO                                    *m_bio_out;
	SSL                                    *m_ssl;
	SSL_CTX                                *m_ctx;

	bool									m_valid;
	bool                                    m_was_handshake;
	bool								    m_handshake_status;
	boost::shared_ptr<WriteRequest>         m_write_req; // write requests queue
	boost::shared_ptr<ReadRequest>          m_read_req; // read request
	std::vector<unsigned char>              m_extra_readbuffer; // Kostyl for extra read data

	VS_TlsContext                           m_tlsContext; // TLS connection context
public:
	bool AddCaCert();
	bool AddCert(const void *cert, const unsigned int size);
	bool UseEndCert(const void *cert, const unsigned int size);
	bool UsePrivateKey(const void *key, const unsigned int size, const char* pass = "");
	virtual bool IsValid() const override;
	// set keys verify mode
	void SetVerifyMode(int mode);
	// Debugging Utilities
	const char *GetStateString() const;
	const char *GetStateStringLong() const;

	bool DeriveKey(uint8_t *out, size_t out_len, const char *label, size_t label_len, const uint8_t *context, size_t context_len);

	// constructor/destructor
	VS_ConnectionTLS(const boost::shared_ptr<VS_WorkThread> &io_thread = boost::shared_ptr<VS_WorkThread>(), const SSL_METHOD* method = TLS_method(), bool use_IPv6 = false);
	// this is special constructor if we want to reuse existing connection.
	// BE CAREFUL!!!
	// You have to delete VS_Connection (conn) after you have used this constructor.
	// it will replace internal imp member and 'conn' will be invalid.
	VS_ConnectionTLS(VS_ConnectionTCP* conn, const boost::shared_ptr<VS_WorkThread> &io_thread = boost::shared_ptr<VS_WorkThread>(), const SSL_METHOD* method = TLS_method(), bool use_IPv6 = false);
	virtual ~VS_ConnectionTLS();

	//Handshake MUST be called after loading PrivateKey and certificates
	// If IOthread is set, asyncronous handshake will begin
	bool Handshake(const void *in_buf, const unsigned long in_len,boost::function<void(bool,VS_ConnectionTLS*,const void*,const unsigned long)> cb, unsigned long &mills, bool isServer = true);

	virtual bool RWrite(const VS_Buffer *buffers, const unsigned long n_buffers) override;
	virtual bool Write(const void *buffer, const unsigned long n_bytes) override;
	virtual int	GetWriteResult( unsigned long &milliseconds ) override;
	virtual int	SetWriteResult( const unsigned long b_trans, const struct VS_Overlapped *ov ) override;

	inline bool IsRead() const override {return !!m_read_req;};
	virtual bool RRead( const unsigned long n_bytes ) override;
	virtual bool Read( void *buffer, const unsigned long n_bytes ) override;
	virtual int	GetReadResult( unsigned long &milliseconds, void **buffer = 0, const bool portion = false ) override;
	virtual int	SetReadResult(const unsigned long b_trans, const struct VS_Overlapped *ov, void **buffer = 0, const bool portion = false) override;

	virtual int	Send(const void *buffer, const unsigned long n_bytes, unsigned long &mills, const bool keep_blocked = true) override;
	virtual int Send(const void *buffer, const unsigned long n_bytes) override;
	virtual int	Receive(void *buffer, const unsigned long n_bytes, unsigned long &mills, bool portion = false) override;
	virtual int	Receive(void* buffer, const unsigned long n_bytes) override;

	bool SetIOThread(const boost::shared_ptr<VS_WorkThread> &io_thread) override;

	virtual void Handle(const unsigned long sz, const struct VS_Overlapped *ov) override;
	virtual void HandleError(const unsigned long err, const struct VS_Overlapped *ov) override;

	virtual bool Connect( const char *host, const unsigned short port,
		unsigned long &milliseconds ,
		bool isFastSocket = false, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL) override;
	virtual bool Connect( const unsigned long ip, const unsigned short port,
		unsigned long &milliseconds  ,
		bool isFastSocket = false, const bool isQoSSocket = false , _QualityOfService * QoSdata = NULL) override;
	virtual bool	IsAccept(void) const override;
	virtual bool	Accept( const char *host, const unsigned short port,
		unsigned long &milliseconds,
		const bool exclusiveUseAddr,
		bool isFastSocket = false,
		bool qos = false,
		_QualityOfService * qos_params = NULL ) override;
	virtual int		Accept( VS_ConnectionTCP *listener, unsigned long &milliseconds ,
		bool isFastSocket = false,
		bool qos = false,
		_QualityOfService * qos_params = NULL ) override;
	virtual bool	Accept( VS_ConnectionTCP *listener,
		bool isFastSocket = false,
		bool qos = false,
		_QualityOfService * qos_params = NULL) override;
	virtual void Close(void) override;

	template<class F>
	void SetCallback(F&& cb)
	{
		m_cb = std::forward<F>(cb);
	}

	void RemoveCallback(void);
	virtual const VS_TlsContext* GetTlsContext() const override {return &m_tlsContext;}
	private:
		bool CreateSSL();
		// handle read and write requests properly
		int DecryptInput(boost::shared_ptr<ReadRequest>& req);
		int HandleReadRequest(void **buffer, bool portion);
		int HandleWriteRequest(const unsigned long b_trans);
		// write all data in SSL output BIO
		bool WriteOutgoing(int written);
		// internal most general send and receive - other overloaded Send() and Receive() will call them
		int Send(const void *buffer, const unsigned long n_bytes, unsigned long *mills, const bool *keep_blocked);
		int	Receive(void *buffer, const unsigned long n_bytes, unsigned long *mills);

		// Raw procedures - transmit data via TCP. For Handshake mostly

		// "Real" send and receive - they will map to the one of the calls in VS_ConeeectionTCP according to its parameters
		int SendData(const void *buffer, const unsigned long n_bytes, unsigned long *mills = nullptr, const bool *keep_blocked = nullptr);
		int	ReceiveData(void *buffer, const unsigned long n_bytes, unsigned long *mills = nullptr, bool portion = false);

		int RawSend(const void *buffer, const unsigned long n_bytes, unsigned long& mills);
		int	RawReceive(void *buffer, const unsigned long n_bytes, unsigned long& mills, bool portion = false);
		bool RawRead(void *buffer, const unsigned long n_bytes);
		bool RawWrite(const void *buffer, const unsigned long n_bytes);
		int RawSetReadResult(const unsigned long b_trans, const struct VS_Overlapped *ov, void **buffer, const bool portion);
		int RawSetWriteResult(const unsigned long b_trans, const struct VS_Overlapped *ov);
		bool TempBufferSatisfiesReqQueue();
		bool TempBufferSatisfiesReq(const boost::shared_ptr<ReadRequest>& req);
		int CertificateVerifyCallback(int ok, X509_STORE_CTX* ctx);
};
