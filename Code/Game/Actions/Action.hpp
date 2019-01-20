#pragma once
class Action
{
public:
	Action();
	~Action();

	void Update(float deltaSeconds);

	bool IsComplete(){return m_isComplete;}

private:
	bool m_isComplete = false;
};

