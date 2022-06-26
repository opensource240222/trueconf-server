#pragma once
class VS_ConfControlInterface;

/**
	TODO:
	impliment ConnectToConfControl and DisconnectFromConfControl here;
*/
class VS_ConfConditionConnector
{
public:
	virtual ~VS_ConfConditionConnector(){}

	virtual void ConnectToConfControl(const char * conf_name, const std::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb) = 0;
	virtual void DisconnectFromConfControl(const char *conf_name, const std::shared_ptr<VS_ConfControlInterface>	&conf_ctrl_cb) = 0;
	//virtual void void ConnectToAllConfControl(const char *conf_name, const boost::shared_ptr<VS_ConfControlInterface> &conf_ctrl_cb);
};