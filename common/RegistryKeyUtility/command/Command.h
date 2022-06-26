#pragma once

class Command
{
public:
	Command();
	virtual ~Command() {}
	Command(const Command& other) = delete;
	Command& operator=(const Command &other) = delete;
	Command(Command&& other) noexcept;
	Command& operator=(Command&& other) noexcept;
	bool IsValid() const;
	/**
	 * @throw CommandException.
	 */
	void Execute();
protected:
	void SetValid(const bool isValid);
	virtual void ExecuteImpl() = 0;
private:
	bool isValid_;
};
