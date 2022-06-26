#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SIPURI.h"

#include <boost/regex.hpp>
#include <boost/optional.hpp>
#include <chrono>

class VS_SIPURI;

class VS_SIPField_Contact: public VS_BaseField
{
public:
	const static boost::regex e;
	const static boost::regex e1;
	const static boost::regex e2;
	const static boost::regex base_uri_e;
	const static boost::regex name_addr_param_e;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	TSIPErrorCodes Init(const VS_SIPGetInfoInterface& call) override;
	VS_SIPField_Contact & operator=(const VS_SIPField_Contact &src);
	bool operator==(const VS_SIPField_Contact &src)const;

	void Clean() noexcept override;

	VS_SIPURI* GetLastURI();
	void SetLastURI(VS_SIPURI* uri);

	boost::optional<std::chrono::seconds> GetExpires() const;

	VS_SIPField_Contact();
	VS_SIPField_Contact(const VS_SIPField_Contact&) = delete;
	~VS_SIPField_Contact() {}

	int order() const override
	{
		return 80;
	}

	struct ContactParam final
	{
		ContactParam()
			: uri(nullptr)
			, expires(boost::none)
		{}

		ContactParam(std::shared_ptr<VS_SIPURI> uri_, boost::optional<std::chrono::seconds> expires_, std::string sip_instance_, std::string gruu_)
			: uri(std::move(uri_))
			, sip_instance(std::move(sip_instance_))
			, gruu(std::move(gruu_))
			, expires(std::move(expires_))
		{}

		ContactParam(VS_SIPURI *uri_, boost::optional<std::chrono::seconds> expires_, std::string sip_instance_, std::string gruu_)
			: uri(uri_)
			, sip_instance(std::move(sip_instance_))
			, gruu(gruu_)
			, expires(std::move(expires_))
		{}

		std::shared_ptr<VS_SIPURI> uri;
		std::string sip_instance;
		std::string gruu;
		boost::optional<std::chrono::seconds> expires;

		bool operator==(const ContactParam& other)const//for std::vector::operator== function
		{
			if (this->uri || other.uri)
			{
				if (this->uri && other.uri)
				{
					if ((*this->uri) != (*other.uri))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}

			return (this->expires == other.expires) && (this->sip_instance == other.sip_instance) && (this->gruu == other.gruu);

		}
	};

	std::string LastGruu() const;

	void GetURIs(std::vector<std::string> &OUT_contacts) const;
	void AddContacts(const VS_SIPField_Contact* pContact);
private:
	std::vector<ContactParam> iContacts;
	bool compact;
	bool operator!=(const VS_SIPField_Contact &src)const;
};

std::unique_ptr<VS_BaseField> VS_SIPField_Contact_Instance();